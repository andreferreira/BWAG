all: bwag.o mmio.o
	g++ -O3 bwag.o mmio.o

bwag.o: bwag.cpp
	g++ -O3 -c bwag.cpp

mmio.o: mmio.c mmio.h
	gcc -c mmio.c

clean:
	rm -f bwag.o mmio.o
