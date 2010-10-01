CC=gcc
CFLAGS=--pedantic -Wall -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS -O2

C2SIM_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2sim.o codebook.o

C2ENC_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2enc.o codebook.o

C2DEC_OBJ = sine.o nlp.o four1.o dump.o quantise.o lpc.o lsp.o phase.o \
            pack.o postfilter.o interp.o codec2.o c2dec.o codebook.o

D=codebook
CODEBOOKS= \
  $D/lsp1.txt \
  $D/lsp2.txt \
  $D/lsp3.txt \
  $D/lsp4.txt \
  $D/lsp5.txt \
  $D/lsp6.txt \
  $D/lsp7.txt \
  $D/lsp8.txt \
  $D/lsp9.txt \
  $D/lsp10.txt

all: c2sim c2enc c2dec

c2sim: $(C2SIM_OBJ)
	$(CC) $(CFLAGS) $(C2SIM_OBJ) -o c2sim -lm

c2enc: $(C2ENC_OBJ)
	$(CC) $(CFLAGS) $(C2ENC_OBJ) -o c2enc -lm

c2dec: $(C2DEC_OBJ)
	$(CC) $(CFLAGS) $(C2DEC_OBJ) -o c2dec -lm

codebook.c: generate_codebook $(CODEBOOKS)
	./generate_codebook $(CODEBOOKS) > codebook.c

codebook.o: codebook.c
	$(CC) -c $(CFLAGS) $< -o $@

generate_codebook: generate_codebook.o
	$(CC) $(CFLAGS) generate_codebook.o -o generate_codebook -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~ c2enc c2dec c2sim generate_codebook src/codebook.c
