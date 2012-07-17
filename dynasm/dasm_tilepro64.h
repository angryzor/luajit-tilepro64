#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DASM_ARCH "tilepro64"

enum {
	DASM_LINE = 2147483635,
	DASM_IMM,
	DASM_L,
	DASM_G,
	DASM_PC,
	DASM_LABEL_L,
	DASM_LABEL_G,
	DASM_LABEL_PC,
	DASM_SPACE,
	DASM_ALIGN,
	DASM_SECTION,
	DASM_ESC,
	DASM_STOP
};

#include "dasm_tilepro64_encmodes.h"
#include "../gdb-reader/luajit-reader-format.h"

/* Maximum number of section buffer positions for a single dasm_put() call. */
#define DASM_MAXSECPOS		25

/* DynASM encoder status codes. Action list offset or number are or'ed in. */
#define DASM_S_OK		0x00000000
#define DASM_S_NOMEM		0x01000000
#define DASM_S_PHASE		0x02000000
#define DASM_S_MATCH_SEC	0x03000000
#define DASM_S_RANGE_I		0x11000000
#define DASM_S_RANGE_SEC	0x12000000
#define DASM_S_RANGE_LG		0x13000000
#define DASM_S_RANGE_PC		0x14000000
#define DASM_S_UNDEF_L		0x21000000
#define DASM_S_UNDEF_PC		0x22000000

#define DASM_LSIZE 20
/* Core structure holding the DynASM encoding state. */
struct dasm_State {
	size_t psize; /* Allocated size of this structure. */
	dasm_ActList actionlist; /* Current actionlist pointer. */
	int *glabels; /* global chain/pos ptrs. */
	size_t gsize;
	int llabels[DASM_LSIZE];
	int *pclabels; /* PC label chains/pos ptrs. */
	size_t pcsize;
	void **globals; /* Array of globals (bias -10). */
	size_t codesize; /* Total size of all code sections. */
	int status; /* Status code. */
	int *buf;
	size_t bsize;
	int buffer_pos;
	int end_of_buffer_pos;
	int code_offset;
	unsigned long total_lines;
};

/* Initialize DynASM state. */
void dasm_init( Dst_DECL, int maxsection) {
	dasm_State *D;
	size_t psz = 0;
	Dst_REF = NULL;
	DASM_M_GROW(Dst, struct dasm_State, Dst_REF, psz, sizeof(dasm_State));
	D = Dst_REF;
	D->psize = psz;
	D->glabels = NULL;
	D->gsize = 0;
	D->pclabels = NULL;
	D->pcsize = 0;
	D->globals = NULL;
	D->buf = NULL; /* Need this for pass3. */
	D->bsize = 0;
	D->end_of_buffer_pos = 0; /* Wrong, but is recalculated after resize. */
	D->total_lines = 0;
}

/* Free DynASM state. */
void dasm_free( Dst_DECL) {
	dasm_State *D = Dst_REF;
	if (D->buf)
		DASM_M_FREE(Dst, D->buf, D->bsize);
	if (D->pclabels)
		DASM_M_FREE(Dst, D->pclabels, D->pcsize);
	if (D->glabels)
		DASM_M_FREE(Dst, D->glabels, D->gsize);
	DASM_M_FREE(Dst, D, D->psize);
}

/* Setup global label array. Must be called before dasm_setup(). */
void dasm_setupglobal( Dst_DECL, void **gl, unsigned int maxgl) {
	dasm_State *D = Dst_REF;
	D->globals = gl;
	DASM_M_GROW(Dst, int, D->glabels, D->gsize, maxgl*sizeof(int));
}

/* Grow PC label array. Can be called after dasm_setup(), too. */
void dasm_growpc( Dst_DECL, unsigned int maxpc) {
	dasm_State *D = Dst_REF;
	size_t osz = D->pcsize;
	DASM_M_GROW(Dst, int, D->pclabels, D->pcsize, maxpc*sizeof(int));
	memset((void *) (((unsigned char *) D->pclabels) + osz), 0, D->pcsize - osz);
}

/* Setup encoder. */
void dasm_setup( Dst_DECL, dasm_ActList actionlist) {
	dasm_State *D = Dst_REF;
	D->actionlist = actionlist;
	D->status = DASM_S_OK;
	memset((void *) D->glabels, 0, D->gsize);
	memset((void*) D->llabels, 0, DASM_LSIZE);
	if (D->pclabels)
		memset((void *) D->pclabels, 0, D->pcsize);
	D->buffer_pos = 0;
	D->code_offset = 0;
}

#ifdef DASM_CHECKS
#define CK(x, st) \
  do { if (!(x)) { \
    D->status = DASM_S_##st|(p-D->actionlist-1); return; } } while (0)
#define CKPL(kind, st) \
  do { if ((size_t)((char *)pl-(char *)D->kind##labels) >= D->kind##size) { \
    D->status = DASM_S_RANGE_##st|(p-D->actionlist-1); return; } } while (0)
#else
#define CK(x, st)	((void)0)
#define CKPL(kind, st)	((void)0)
#endif

void collapse_chain(dasm_State* D, int lofs, int* lcptr) {
	int n = *lcptr;
	while (n != 0) {
		int* pb = D->buf + n;
		n = *pb;
		*pb = lofs;
	}
	*lcptr = 0;
}


/* Pass 1: Store actions and args, link branches/labels, estimate offsets. */
void dasm_put( Dst_DECL, int start, ...) {
	va_list ap;
	dasm_State *D = Dst_REF;
	dasm_ActList p = D->actionlist + start;
	int pos = D->buffer_pos, ofs = D->code_offset;
	int *b;

	if (pos >= D->end_of_buffer_pos) {
		DASM_M_GROW(Dst, int, D->buf, D->bsize,	D->bsize + 2*DASM_MAXSECPOS*sizeof(int));
		D->end_of_buffer_pos = D->bsize / sizeof(int) - DASM_MAXSECPOS;
	}

	b = D->buf;
	b[pos++] = start;

	va_start(ap, start);
	while (1) {
		int action = *p++;
		if (action < DASM_LINE)
			ofs++;
		else if (action == DASM_LINE) {
			p++;										/* Constant line number */
			D->total_lines++;
		}
		else if (action == DASM_IMM) {
			p++;										/* Constant encoding mode */
			unsigned long n = va_arg(ap,unsigned long);	/* Semiconstant immediate value */
			b[pos++] = n;
		} else if (action < DASM_PC) {
			p++;										/* Constant encoding mode */
			unsigned long n = *p++;						/* Constant label ID */

			switch (action) {
			case DASM_L:
				if (n < 10) {
					if (D->llabels[n] == 0)
						return; /* TODO: error */

					b[pos++] = D->llabels[n];
				} else {
					/* Hook into chain */
					b[pos] = D->llabels[n];
					D->llabels[n] = pos++;
				}
				break;
			case DASM_G:
				if (D->glabels[n] < 0)
					b[pos++] = -D->glabels[n];
				else {
					b[pos] = D->glabels[n];
					D->glabels[n] = pos++;
				}
				break;
			}
		} else if (action == DASM_PC) {
			p++;										/* Constant encoding mode */
			unsigned long n = va_arg(ap,unsigned long);	/* Semiconstant label ID */
			if (D->pclabels[n] < 0)
				b[pos++] = -D->pclabels[n];
			else {
				b[pos] = D->pclabels[n];
				D->pclabels[n] = pos++;
			}
		} else if(action == DASM_LABEL_PC) {
			unsigned long n = va_arg(ap,unsigned long);	/* Semiconstant space amount / label ID */

//			switch (action) {
//			case DASM_LABEL_PC:
				if (D->pclabels[n] >= 0)
					collapse_chain(D, ofs, &D->pclabels[n]);
				D->pclabels[n] = -ofs;
/*				break;
			case DASM_SPACE:
				b[pos++] = n;
				ofs += n;
				break;
			}*/
		} else if (action == DASM_SPACE) {
			unsigned long n = va_arg(ap,unsigned long);

			b[pos++] = n;
			ofs += n;
		} else if (action <= DASM_SECTION) {
			unsigned long n = *p++;						/* Constant alignment value / label ID */

			switch (action) {
			case DASM_LABEL_L:
				collapse_chain(D, ofs, &D->llabels[10 + n]);
				D->llabels[n] = ofs;
				break;
			case DASM_LABEL_G:
				if (D->glabels[n] >= 0)
					collapse_chain(D, ofs, &D->glabels[n]);
				D->glabels[n] = -ofs;
				break;
			case DASM_ALIGN:
				while (ofs & (n >> 2))
					ofs++;
				break;
			}
		} else {
			switch (action) {
			case DASM_ESC:
				p++;
				ofs++;
				break;
			case DASM_STOP:
				goto stop;
			}
		}
	}
	stop: va_end(ap);
	D->buffer_pos = pos;
	D->code_offset = ofs;
}
#undef CK

/* Pass 2: externals */
int dasm_link( Dst_DECL, size_t *szp) {
	dasm_State *D = Dst_REF;

#ifdef DASM_CHECKS
	*szp = 0;
	if (D->status != DASM_S_OK) return D->status;
	{
		int pc;
		for (pc = 0; pc*sizeof(unsigned long) < D->pcsize; pc++)
		if (D->pclabels[pc] > 0) return DASM_S_UNDEF_PC|(unsigned long)pc;
	}
#endif

	{ /* Handle globals not defined in this translation unit. */
		int idx;
		for (idx = 0; idx * sizeof(unsigned long) < D->gsize; idx++) {
			if(D->glabels[idx] >= 0)
				collapse_chain(D,-idx,&D->glabels[idx]);
		}
	}

	D->codesize = D->code_offset; /* Total size of all code sections */
	*szp = D->code_offset << 2;
	return DASM_S_OK;
}

/*
 * This procedure is called when a variable or halfconstant needs to be compiled in at runtime
 */
static void encode_refactoring(unsigned long* tgt, jit_encmodes mode, unsigned long r) {
	unsigned long* tgtlo = tgt;
	unsigned long* tgthi = tgt + 1;

	switch (mode) {
	case IEM_X0_Imm8:
		*tgtlo |= ((r & 0xFF) << 12);
		break;
	case IEM_X0_Imm16:
		*tgtlo |= ((r & 0xFFFF) << 12);
		break;
	case IEM_X1_Br:
		r = (r - (unsigned long) tgt) >> 3;
		*tgthi |= ((r & 0x7FFF) << 11);
		*tgthi |= (((r >> 15) & 3) << 3);
		break;
	case IEM_X1_Shift:
		*tgthi |= (r << 11);
		break;
	case IEM_X1_J_j:
	case IEM_X1_J_jal:
		if (r - (unsigned long) tgt <= 0x80000000 ) { // TODO: Possible off-by-one error here. Should think about this.
			/*
			 * Jump forward. use j(al)f.
			 * setNextPC(getCurrentPC() + BACKWARD_OFFSET +
			 *			 (JOff << (INSTRUCTION_SIZE_LOG_2 - BYTE_SIZE_LOG_2)));
			 * ==> nextPC = curPC + (JOff << 3)
			 * <=> JOff = (nextPC - curPC) >> 3
			 */
			r = (r - (unsigned long) tgt) >> 3;
			/* Add the j(al)f instruction. */
			if(mode == IEM_X1_J_jal)
				*tgthi |= (0xC << 27);
			else
				*tgthi |= (0xA << 27);
		} else {
			/*
			 * Jump backward. use j(al)b.
			 * setNextPC(getCurrentPC() +
			 *			 (JOff << (INSTRUCTION_SIZE_LOG_2 - BYTE_SIZE_LOG_2)));
			 * ==> nextPC = curPC + 0x80000000 + (JOff << 3)
			 * <=> JOff = (nextPC - curPC - 0x80000000) >> 3
			 */
			r = (r - (unsigned long) tgt - 0x80000000) >> 3;
			/* Add the j(al)b instruction. */
			if(mode == IEM_X1_J_jal)
				*tgthi |= (0xD << 27);
			else
				*tgthi |= (0xB << 27);
		}
		/* Continue as we would for a normal X1_J encoding */
	case IEM_X1_J:
		*tgthi |= ((r & 0x7FFF) << 11);
		*tgthi |= (((r >> 15) & 3) << 3);
		*tgtlo |= ((r >> 17) << 31);
		*tgthi |= ((r >> 18) & 7);
		*tgthi |= (((r >> 21) & 0x3F) << 5);
		*tgthi |= (((r >> 27) & 1) << 26);
		break;
	default:
		/* TODO: error */
		break;
	}
}

struct ljit_reader_line_def* debug_get_lines_offset( void *debugdata ) {
	return (struct ljit_reader_line_def*)((char*)debugdata + sizeof(struct ljit_reader_func_def));
}

void debug_write_func_def( dasm_State* D, void *debugdata , GDB_CORE_ADDR begin, GDB_CORE_ADDR end, const char* filename, const char* name ) {
	struct ljit_reader_func_def* fdef = (struct ljit_reader_func_def*) debugdata;
	memset(fdef,0,sizeof(struct ljit_reader_func_def));

	fdef->begin = begin;
	fdef->end = end;
	fdef->num_lines = D->total_lines;
	strcpy(fdef->filename, filename);
	strcpy(fdef->name, name);
}

size_t get_debug_bufsize( dasm_State* D ) {
	return sizeof(struct ljit_reader_func_def) + D->total_lines * sizeof(struct ljit_reader_line_def);
}

/* Pass 3: Encode sections. */
int dasm_encode( Dst_DECL, void *buffer, void** debugdata, size_t* ddatasize, const char* filename, const char* blockname) {
	dasm_State *D = Dst_REF;
	unsigned long *base = (unsigned long *) buffer;
	unsigned long *cp = base;

	int *b = D->buf;
	int *endb = D->buf + D->buffer_pos;

	size_t ddsize = get_debug_bufsize(D);
	*ddatasize = ddsize;
	void* dbgdata = malloc(ddsize);
	struct ljit_reader_line_def *lines = debug_get_lines_offset(dbgdata);

	while (b != endb) {
		dasm_ActList p = D->actionlist + *b++;
		while (1) {
			int action = *p++;
			int param = ((action >= DASM_IMM && action <= DASM_PC) || action == DASM_SPACE) ? *b++ : 0;
			jit_encmodes mode =	(action >= DASM_IMM && action <= DASM_PC) ? *p++ : 0;
			int cparam = ((action >= DASM_L && action <= DASM_ALIGN && action != DASM_PC && action != DASM_LABEL_PC && action != DASM_SPACE) || action == DASM_LINE) ? *p++ : 0;

			switch (action) {
			case DASM_LINE:
				lines->line = cparam;
				lines->addr = cp;
				lines++;
				break;
			case DASM_IMM:
				encode_refactoring(cp - 2, mode, param);
				break;
			case DASM_L:
			case DASM_PC:
				encode_refactoring(cp - 2, mode, (unsigned long)base + (param << 2));
				break;
			case DASM_G:
				if(param >= 0)
					encode_refactoring(cp - 2, mode, (unsigned long)base + (param << 2));
				else
					encode_refactoring(cp - 2, mode, (unsigned long)D->globals[-param]);
				break;
			case DASM_LABEL_L:
			case DASM_LABEL_PC:
				break;
			case DASM_LABEL_G:
				D->globals[cparam] = cp;
				break;
			case DASM_SPACE:
				cp += param;
				break;
			case DASM_ALIGN:
				while ((int) cp & cparam)
					cp++;
				break;
			case DASM_ESC:
				action = *p++;
			default:
				*cp++ = action;
				break;
			case DASM_STOP:
				goto stop;
			}
		}
		stop: (void) 0;
	}

	debug_write_func_def(D, dbgdata, (GDB_CORE_ADDR)base, (GDB_CORE_ADDR)(cp - 2), filename, blockname);
	*debugdata = dbgdata;

	if (base + D->codesize != cp) /* Check for phase errors. */
		return DASM_S_PHASE;
	return DASM_S_OK;
}

/* Get PC label offset. */
int dasm_getpclabel(Dst_DECL, unsigned int pc)
{
  dasm_State *D = Dst_REF;
  if (pc*sizeof(int) < D->pcsize) {
    int pos = D->pclabels[pc];
    if (pos < 0) return D->buf[-pos];
    if (pos > 0) return -1;  /* Undefined. */
  }
  return -2;  /* Unused or out of range. */
}

#ifdef DASM_CHECKS
/* Optional sanity checker to call between isolated encoding steps. */
int dasm_checkstep(Dst_DECL, int secmatch)
{
	dasm_State *D = Dst_REF;
	if (D->status == DASM_S_OK) {
		int i;
		for (i = 1; i <= 9; i++) {
			if (D->lglabels[i] > 0) {D->status = DASM_S_UNDEF_L|i; break;}
			D->lglabels[i] = 0;
		}
	}
	if (D->status == DASM_S_OK && secmatch >= 0 &&
			D->section != &D->sections[secmatch])
	D->status = DASM_S_MATCH_SEC|(D->section-D->sections);
	return D->status;
}
#endif

