#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define DASM_ARCH "tilepro64"

enum {
	DASM_IMM = 2147483637,
	DASM_L, DASM_G, DASM_PC, DASM_LABEL_L, DASM_LABEL_G, DASM_LABEL_PC,
	DASM_ALIGN, DASM_ESC, DASM_SECTION, DASM_STOP
};

typedef enum jit_encmodes{
	IEM_X0_Imm8 = 0,
	IEM_X0_Imm16,
	IEM_X1_Br,
	IEM_X1_Shift,
	IEM_X1_J,
	IEM_X1_J_jal,
} jit_encmodes;
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

/* Macros to convert positions (8 bit section + 24 bit index). */
#define DASM_POS2IDX(pos)	((pos)&0x00ffffff)
#define DASM_POS2BIAS(pos)	((pos)&0xff000000)
#define DASM_SEC2POS(sec)	((sec)<<24)
#define DASM_POS2SEC(pos)	((pos)>>24)
#define DASM_POS2PTR(D, pos)	(D->sections[DASM_POS2SEC(pos)].rbuf + (pos))

/* Per-section structure. */
typedef struct dasm_Section {
  int *rbuf;		/* Biased buffer pointer (negative section bias). */
  int *buf;		/* True buffer pointer. */
  size_t bsize;		/* Buffer size in bytes. */
  int pos;		/* Biased buffer position. */
  int epos;		/* End of biased buffer position - max single put. */
  int ofs;		/* Word offset into section. */
} dasm_Section;

#define DASM_LSIZE 20
/* Core structure holding the DynASM encoding state. */
struct dasm_State {
  size_t psize;			/* Allocated size of this structure. */
  dasm_ActList actionlist;	/* Current actionlist pointer. */
  int *glabels;		/* global chain/pos ptrs. */
  size_t gsize;
  int llabels[DASM_LSIZE];
  int *pclabels;		/* PC label chains/pos ptrs. */
  size_t pcsize;
  void **globals;		/* Array of globals (bias -10). */
  dasm_Section *section;	/* Pointer to active section. */
  size_t codesize;		/* Total size of all code sections. */
  int maxsection;		/* 0 <= sectionidx < maxsection. */
  int status;			/* Status code. */
  dasm_Section sections[1];	/* All sections. Alloc-extended. */
};

/* The size of the core structure depends on the max. number of sections. */
#define DASM_PSZ(ms)	(sizeof(dasm_State)+(ms-1)*sizeof(dasm_Section))

/* Initialize DynASM state. */
void dasm_init(Dst_DECL, int maxsection)
{
  dasm_State *D;
  size_t psz = 0;
  int i;
  Dst_REF = NULL;
  DASM_M_GROW(Dst, struct dasm_State, Dst_REF, psz, DASM_PSZ(maxsection));
  D = Dst_REF;
  D->psize = psz;
  D->glabels = NULL;
  D->gsize = 0;
  D->pclabels = NULL;
  D->pcsize = 0;
  D->globals = NULL;
  D->maxsection = maxsection;
  for (i = 0; i < maxsection; i++) {
    D->sections[i].buf = NULL;  /* Need this for pass3. */
    D->sections[i].rbuf = D->sections[i].buf - DASM_SEC2POS(i);
    D->sections[i].bsize = 0;
    D->sections[i].epos = 0;  /* Wrong, but is recalculated after resize. */
  }
}

/* Free DynASM state. */
void dasm_free(Dst_DECL)
{
  dasm_State *D = Dst_REF;
  int i;
  for (i = 0; i < D->maxsection; i++)
    if (D->sections[i].buf)
      DASM_M_FREE(Dst, D->sections[i].buf, D->sections[i].bsize);
  if (D->pclabels) DASM_M_FREE(Dst, D->pclabels, D->pcsize);
  if (D->glabels) DASM_M_FREE(Dst, D->glabels, D->gsize);
  DASM_M_FREE(Dst, D, D->psize);
}

/* Setup global label array. Must be called before dasm_setup(). */
void dasm_setupglobal(Dst_DECL, void **gl, unsigned int maxgl)
{
  dasm_State *D = Dst_REF;
  D->globals = gl;  /* Negative bias to compensate for locals. */
  DASM_M_GROW(Dst, int, D->glabels, D->gsize, maxgl*sizeof(int));
}

/* Grow PC label array. Can be called after dasm_setup(), too. */
void dasm_growpc(Dst_DECL, unsigned int maxpc)
{
  dasm_State *D = Dst_REF;
  size_t osz = D->pcsize;
  DASM_M_GROW(Dst, int, D->pclabels, D->pcsize, maxpc*sizeof(int));
  memset((void *)(((unsigned char *)D->pclabels)+osz), 0, D->pcsize-osz);
}

/* Setup encoder. */
void dasm_setup(Dst_DECL, dasm_ActList actionlist)
{
  dasm_State *D = Dst_REF;
  int i;
  D->actionlist = actionlist;
  D->status = DASM_S_OK;
  D->section = &D->sections[0];
  memset((void *)D->glabels, 0, D->gsize);
  memset((void*)D->llabels, 0, DASM_LSIZE);
  if (D->pclabels) memset((void *)D->pclabels, 0, D->pcsize);
  for (i = 0; i < D->maxsection; i++) {
    D->sections[i].pos = DASM_SEC2POS(i);
    D->sections[i].ofs = 0;
  }
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

void collapse_chain(dasm_State* D, int lofs, int* lcptr)
{
	int n = *lcptr;
	while(n != 0)
	{
		int* pb = DASM_POS2PTR(D,*lcptr);
		n = *pb;
		*pb = lofs;
	}
}


/* Pass 1: Store actions and args, link branches/labels, estimate offsets. */
void dasm_put(Dst_DECL, int start, ...)
{
  va_list ap;
  dasm_State *D = Dst_REF;
  dasm_ActList p = D->actionlist + start;
  dasm_Section *sec = D->section;
  int pos = sec->pos, ofs = sec->ofs;
  int *b;

  if (pos >= sec->epos) {
    DASM_M_GROW(Dst, int, sec->buf, sec->bsize,
      sec->bsize + 2*DASM_MAXSECPOS*sizeof(int));
    sec->rbuf = sec->buf - DASM_POS2BIAS(pos);
    sec->epos = sec->bsize/sizeof(int) - DASM_MAXSECPOS + DASM_POS2BIAS(pos);
  }

  b = sec->rbuf;
  b[pos++] = start;

  va_start(ap, start);
  while (1) {
    int action = *p++;
    if (action < DASM_IMM) ofs++;
	else if (action == DASM_IMM) {
		p++;											/* Constant encoding mode */
		unsigned long n = va_arg(ap,unsigned long);		/* Semiconstant immediate value */
		b[pos++] = n;
	}
	else if (action <= DASM_PC) {
		p++;											/* Constant encoding mode */
		unsigned long n = *p++;							/* Constant label ID */

		switch(action) {
		case DASM_L:
			if(n < 10)
			{
				if(D->llabels[n] == 0)
					return; /* TODO: error */

				b[pos++] = D->llabels[n];
			}
			else
			{
				/* Hook into chain */
				b[pos] = D->llabels[n];
				D->llabels[n] = pos++;
			}
			break;
		case DASM_G:
			if(D->glabels[n] < 0)
				b[pos++] = -D->glabels[n];
			else
			{
				b[pos] = D->glabels[n];
				D->glabels[n] = pos++;
			}
			break;
		}
	}
	else if(action <= DASM_ALIGN) {
		unsigned long n = *p++;							/* Constant alignment value */

		switch(action) {
		case DASM_LABEL_L:
			collapse_chain(D, ofs, &D->llabels[10+n]);
			D->llabels[n] = ofs;
			break;
		case DASM_LABEL_G:
			collapse_chain(D, ofs, &D->glabels[n]);
			D->glabels[n] = -ofs;
			break;
		case DASM_ALIGN:
			while(ofs & (n >> 2)) ofs++;
			break;
		case DASM_SECTION:
			CK(n < D->maxsection, RANGE_SEC); 
			D->section = &D->sections[n];
			goto stop;
		}
	}
	else switch(action) {
	case DASM_ESC:
		p++;
		ofs++;
		break;
	case DASM_STOP:
		goto stop;
	}
  }
stop:
  va_end(ap);
  sec->pos = pos;
  sec->ofs = ofs;
}
#undef CK

/* Pass 2: Link sections, shrink branches/aligns, fix label offsets. */
int dasm_link(Dst_DECL, size_t *szp)
{
  dasm_State *D = Dst_REF;
  int secnum;
  int ofs = 0;

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
    for (idx = 0; idx*sizeof(unsigned long) < D->gsize; idx++) {
      int n = D->glabels[idx];
      /* Undefined label: Collapse rel chain and replace with marker (< 0). */
      while (n > 0) { int *pb = DASM_POS2PTR(D, n); n = *pb; *pb = -idx; }
    }
  }

  /* Combine all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->rbuf;
    int pos = DASM_SEC2POS(secnum);
    int lastpos = sec->pos;

    while (pos != lastpos) {
		dasm_ActList p = D->actionlist + b[pos++];
		while (1) {
			int action = *p++;
			switch (action) {
			case DASM_IMM:
				p++;
				pos++;
				break;
			case DASM_L:
			case DASM_G:
				p += 2;
				b[pos++] += ofs;
				break;
			case DASM_LABEL_L:
			case DASM_LABEL_G:
			case DASM_ALIGN:
			case DASM_ESC:
				p++;
				break;
			case DASM_SECTION:
			case DASM_STOP:
				goto stop;
			}
		}
		stop: (void)0;
	}
	ofs += sec->ofs;  /* Next section starts right after current section. */
  }

  D->codesize = ofs;  /* Total size of all code sections */
  *szp = ofs;
  return DASM_S_OK;
}

/*
 * This procedure is called when a variable or halfconstant needs to be compiled in at runtime
 */
static void encode_refactoring(unsigned long* tgt, jit_encmodes mode, unsigned long r)
{
	unsigned long* tgtlo = tgt;
	unsigned long* tgthi = tgt + sizeof(unsigned long);
	
	switch(mode)
	{
	case IEM_X0_Imm8:
	case IEM_X0_Imm16:
		*tgtlo |= (r << 12);
		break;
	case IEM_X1_Br:
		*tgthi |= ((r & 0x7FFF) << 11);
		*tgthi |= ((r >> 15) << 3);
		break;
	case IEM_X1_Shift:
		*tgthi |= (r << 11);
		break;
	case IEM_X1_J_jal:
		if(r >= (unsigned long)tgt)
		{
			/*
			 * Jump forward. use jalf.
			 * setNextPC(getCurrentPC() + BACKWARD_OFFSET +
			 *			 (JOff << (INSTRUCTION_SIZE_LOG_2 - BYTE_SIZE_LOG_2))); 
			 * ==> nextPC = curPC + (JOff << 3)
			 * <=> JOff = (nextPC - curPC) >> 3
			 */
			r = (r - (unsigned long)tgt) >> 3;
			/* Add the jalf instruction. */
			*tgthi |= (0xC << 27);
		}
		else
		{
			/*
			 * Jump backward. use jalb.
			 * setNextPC(getCurrentPC() +
			 *			 (JOff << (INSTRUCTION_SIZE_LOG_2 - BYTE_SIZE_LOG_2))); 
			 * ==> nextPC = curPC + 0x80000000 + (JOff << 3)
			 * <=> JOff = (nextPC - curPC - 0x80000000) >> 3
			 */
			r = (r - (unsigned long)tgt - 0x80000000) >> 3;
			/* Add the jalb instruction. */
			*tgthi |= (0xD << 27);
		}
		/* Continue as we would for a normal X1_J encoding */
	case IEM_X1_J:
		*tgthi |= ((r & 0x7FFF) << 11);
		*tgthi |= (((r >> 15) & 3) << 3);
		*tgtlo |= ((r >> 17) << 31);
		*tgthi |= ((r >> 18) & 7);
		*tgthi |= (((r >> 21) & 0x3F) << 5);
		*tgthi |= ((r >> 27) << 26);
		break;
	default:
		/* TODO: error */
		break;
	}
}

/* Pass 3: Encode sections. */
int dasm_encode(Dst_DECL, void *buffer)
{
  dasm_State *D = Dst_REF;
  unsigned long *base = (unsigned long *)buffer;
  unsigned long *cp = base;
  int secnum;

  /* Encode all code sections. No support for data sections (yet). */
  for (secnum = 0; secnum < D->maxsection; secnum++) {
    dasm_Section *sec = D->sections + secnum;
    int *b = sec->buf;
    int *endb = sec->rbuf + sec->pos;

    while (b != endb) {
		dasm_ActList p = D->actionlist + *b++;
		while (1) {
			int action = *p++;
			int param = (action >= DASM_IMM && action <= DASM_PC) ? *b++ : 0;
			jit_encmodes mode = (action >= DASM_IMM && action <= DASM_PC) ? *p++ : 0;
			if(action >= DASM_L && action <= DASM_ALIGN) p++;

			switch(action) {
			case DASM_IMM:
				encode_refactoring(cp - 2, mode, param);
				break;
			case DASM_L:
			case DASM_G:
				encode_refactoring(cp - 2, mode, (param << 2));
				break;
			case DASM_ALIGN:
				{
					int n = *p++;
					while((int)cp & n) cp++;
				}
				break;
			case DASM_ESC:
				action = *p++;
			default:
				*cp++ = action;
				break;
			case DASM_SECTION:
			case DASM_STOP:
				goto stop;
			}
		}
		stop: (void)0;
	}
  }

  if (base + D->codesize != cp)  /* Check for phase errors. */
    return DASM_S_PHASE;
  return DASM_S_OK;
}

/* Get PC label offset. */
int dasm_getpclabel(Dst_DECL, unsigned int pc)
{
#if 0
  dasm_State *D = Dst_REF;
  if (pc*sizeof(int) < D->pcsize) {
    int pos = D->pclabels[pc];
    if (pos < 0) return *DASM_POS2PTR(D, -pos);
    if (pos > 0) return -1;  /* Undefined. */
  }
#endif
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
      if (D->lglabels[i] > 0) { D->status = DASM_S_UNDEF_L|i; break; }
      D->lglabels[i] = 0;
    }
  }
  if (D->status == DASM_S_OK && secmatch >= 0 &&
      D->section != &D->sections[secmatch])
    D->status = DASM_S_MATCH_SEC|(D->section-D->sections);
  return D->status;
}
#endif


