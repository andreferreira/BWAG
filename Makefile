all: bwag.o mmio.o
	g++ -pg -o bwag bwag.o mmio.o

bwag.o: bwag.cpp
	g++ -pg -O3 -c bwag.cpp

mmio.o: mmio.c mmio.h
	gcc -pg -c mmio.c

clean:
	rm -f bwag.o mmio.o bwag
