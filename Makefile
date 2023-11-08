CC=gcc
CFLAGS=-W -Wall -pedantic
LDFLAGS=-lm
EXEC=cmu

all: $(EXEC)

cmu: cmu.o
	$(CC) -o $@ $^ $(LDFLAGS)

cmu.o: cmu.c cmu.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)