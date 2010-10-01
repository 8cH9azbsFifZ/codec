CFLAGS = -I. -I../src -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

all: genres genlsp extract vqtrain tnlp tinterp tquant tcodec2

genres: genres.o ../src/lpc.o
	gcc $(CFLAGS) -o genres genres.o ../src/lpc.o -lm

LSP_TEST_OBJ = lsptest.o ../src/lpc.o ../src/lsp.o sd.o ../src/four1.o

TNLP_OBJ     = tnlp.o ../src/sine.o ../src/nlp.o ../src/four1.o ../src/dump.o

TCONTPHASE_OBJ = tcontphase.o ../src/globals.o ../src/dump.o ../src/synth.o \
                 ../src/four1.c ../src/initdec.o ../src/phase.o

TINTERP_OBJ    = tinterp.o ../src/sine.o ../src/four1.o ../src/interp.o

TQUANT_OBJ     = tquant.o ../src/quantise.o ../src/codebook.o \
		 ../src/lpc.o ../src/lsp.o \
                 ../src/dump.o ../src/four1.o

TCODEC2_OBJ    = tcodec2.o ../src/quantise.o  ../src/codebook.o \
		../src/lpc.o ../src/lsp.o ../src/pack.o \
                 ../src/dump.o ../src/four1.o ../src/codec2.o ../src/sine.o \
		../src/nlp.o ../src/postfilter.o ../src/phase.o ../src/interp.o

lsptest: $(LSP_TEST_OBJ)
	gcc $(CFLAGS) -o lsptest $(LSP_TEST_OBJ) -lm

genlsp: genlsp.o ../src/lpc.o ../src/lsp.o
	gcc $(CFLAGS) -o genlsp genlsp.o ../src/lpc.o ../src/lsp.o -lm

extract: extract.o
	gcc $(CFLAGS) -o extract extract.o -lm

vqtrain: vqtrain.o
	gcc $(CFLAGS) -o vqtrain vqtrain.o -lm

tnlp: $(TNLP_OBJ)
	gcc $(CFLAGS) -o tnlp $(TNLP_OBJ) -lm

tcontphase: $(TCONTPHASE_OBJ)
	gcc $(CFLAGS) -o tcontphase $(TCONTPHASE_OBJ) -lm

tmodel: tmodel.o
	gcc $(CFLAGS) -o tmodel tmodel.o -lm

tinterp: $(TINTERP_OBJ)
	gcc $(CFLAGS) -o tinterp $(TINTERP_OBJ) -lm

tquant: $(TQUANT_OBJ)
	gcc $(CFLAGS) -o tquant $(TQUANT_OBJ) -lm

tcodec2: $(TCODEC2_OBJ)
	gcc $(CFLAGS) -o tcodec2 $(TCODEC2_OBJ) -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f *.o *~ src/*~ \
	extract genlsp genres tcodec2 tinterp tnlp tquant vqtrain
