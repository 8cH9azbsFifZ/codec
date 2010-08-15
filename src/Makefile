CC=gcc
CFLAGS=-g -Wall -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

SINENC_OBJ  = sinenc.o globals.o initenc.o four1.o refine.o spec.o dump.o
SINEDEC_OBJ = sinedec.o globals.o initenc.o initdec.o four1.o synth.o \
              quantise.o lpc.o dump.o refine.o lsp.o phase.o postfilter.o \
              interp.o

all: sinenc sinedec

sinenc: $(SINENC_OBJ)
	$(CC) $(CFLAGS) $(SINENC_OBJ) -o sinenc -lm

sinedec: $(SINEDEC_OBJ)
	$(CC) $(CFLAGS) $(SINEDEC_OBJ) -o sinedec -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~
