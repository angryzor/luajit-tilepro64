TARGET=luajit-reader.dll

CC=gcc

CFLAGS=-fPIC

$(TARGET): *.o
	$(CC) $(CFLAGS) -shared -o $@ *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

make.depend: *.c
	$(CC) $(CFLAGS) -MM $^ > make.depend

clean:
	rm -f *.o $(TARGET) make.depend

include make.depend

.PHONY:	clean

