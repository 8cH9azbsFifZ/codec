CC=gcc
CFLAGS=-g -Wall -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

SINENC_OBJ  = sine.o four1.o sinenc.o dump.o
SINEDEC_OBJ = sine.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
              postfilter.o interp.o sinedec.o 

all: sinenc sinedec

sinenc: $(SINENC_OBJ)
	$(CC) $(CFLAGS) $(SINENC_OBJ) -o sinenc -lm

sinedec: $(SINEDEC_OBJ)
	$(CC) $(CFLAGS) $(SINEDEC_OBJ) -o sinedec -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~
