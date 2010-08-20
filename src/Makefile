CC=gcc
CFLAGS=-g -Wall -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

C2SIM_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            postfilter.o interp.o c2sim.o 

all: c2sim

c2sim: $(C2SIM_OBJ)
	$(CC) $(CFLAGS) $(C2SIM_OBJ) -o c2sim -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~
