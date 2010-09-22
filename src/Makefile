CC=gcc
CFLAGS=-g -Wall -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS -O2

C2SIM_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2sim.o 

C2ENC_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2enc.o 

C2DEC_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2dec.o 

all: c2sim c2enc c2dec

c2sim: $(C2SIM_OBJ)
	$(CC) $(CFLAGS) $(C2SIM_OBJ) -o c2sim -lm

c2enc: $(C2ENC_OBJ)
	$(CC) $(CFLAGS) $(C2ENC_OBJ) -o c2enc -lm

c2dec: $(C2DEC_OBJ)
	$(CC) $(CFLAGS) $(C2DEC_OBJ) -o c2dec -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~ c2enc c2dec c2sim
