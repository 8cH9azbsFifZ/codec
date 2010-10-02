AM_CFLAGS = -I../src -fPIC -Wall -O3
AUTOMAKE_OPTS = gnu
NAME = codec2
AM_CPPFLAGS = $(AM_CFLAGS)

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

noinst_PROGRAMS = generate_codebook

codebook.$(OBJEXT): codebook.c

codebook.lo: codebook.c

codebook.c: generate_codebook $(CODEBOOKS)
	./generate_codebook $(CODEBOOKS) > codebook.c

clean-local:
	-rm -f codebook.c

lib_LTLIBRARIES	= libcodec2.la
libcodec2_la_SOURCES = dump.c \
lpc.c \
nlp.c \
postfilter.c \
sine.c \
codec2.c \
four1.c \
interp.c \
lsp.c \
phase.c \
quantise.c \
pack.c \
codebook.c

libcodec2_la_CFLAGS = $(AM_CFLAGS)
libcodec2_la_LDFLAGS = $(LIBS)

library_includedir = $(prefix)
library_include_HEADERS	= codec2.h \
defines.h \
four1.h \
interp.h \
lsp.h \
phase.h \
quantise.h \
comp.h \
dump.h \
globals.h \
lpc.h \
nlp.h  \
postfilter.h \
sine.h

bin_PROGRAMS = c2dec c2enc c2sim

c2dec_SOURCES = c2dec.c
c2dec_LDADD = $(lib_LTLIBRARIES) 
c2dec_LDFLAGS = $(LIBS)

c2enc_SOURCES = c2enc.c
c2enc_LDADD = $(lib_LTLIBRARIES) 
c2enc_LDFLAGS = $(LIBS)

c2sim_SOURCES = c2sim.c
c2sim_LDADD = $(lib_LTLIBRARIES) 
c2sim_LDFLAGS = $(LIBS)
