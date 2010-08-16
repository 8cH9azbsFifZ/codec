CFLAGS = -I. -I../src -I../speex -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

all: genres genlsp extract vqtrain tnlp

genres: genres.o ../src/lpc.o
	gcc $(CFLAGS) -o genres genres.o ../src/lpc.o -lm

LSP_TEST_OBJ = lsptest.o ../src/lpc.o ../src/lsp.o sd.o ../src/four1.o

TNLP_OBJ     = tnlp.o ../src/nlp.o ../src/four1.o ../src/initenc.o ../src/dump.o \
               ../src/globals.o  ../src/refine.o

TCONTPHASE_OBJ = tcontphase.o ../src/globals.o ../src/dump.o ../src/synth.o \
                 ../src/four1.c ../src/initdec.o ../src/phase.o

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

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
