CFLAGS = -I. -I../src -I../speex -Wall -g -DFLOATING_POINT -DVAR_ARRAYS

all: genres lsptest

genres: genres.o ../src/lpc.o
	gcc $(CFLAGS) -o genres genres.o ../src/lpc.o -lm

LSP_TEST_OBJ = lsptest.o ../src/lpc.o ../speex/lsp.o sd.o ../src/four1.o \
               ../speex/quant_lsp.o ../speex/bits.o ../speex/lsp_tables_nb.o \
               ../speex/high_lsp_tables.c

lsptest: $(LSP_TEST_OBJ)
	gcc $(CFLAGS) -o lsptest $(LSP_TEST_OBJ) -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
