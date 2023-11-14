CC=gcc
CFLAGS=-W -Wall -pedantic
LDFLAGS=-lm
EXEC=cmu

all: $(EXEC)

cmu: cmu.o
	$(CC) -o build/$@ build/$^ $(LDFLAGS)

cmu.o: cmu.c cmu.h
	$(CC) -o build/$@ -c $< $(CFLAGS)

clean:
	rm -rf build/

exec:
	./build/$(EXEC)

mrproper: clean
	rm -rf $(EXEC)