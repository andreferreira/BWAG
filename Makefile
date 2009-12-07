all: bwag.o mmio.o
	g++ -o bwag bwag.o mmio.o

bwag.o: bwag.cpp
	g++  -c bwag.cpp

mmio.o: mmio.c mmio.h
	gcc -c mmio.c

clean:
	rm -f bwag.o mmio.o bwag
