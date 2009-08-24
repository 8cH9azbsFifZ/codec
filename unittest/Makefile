CFLAGS = -I../src -Wall -g

all: genres

genres: genres.o ../src/lpc.o
	gcc $(CFLAGS) -o genres genres.o ../src/lpc.o -lm

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
