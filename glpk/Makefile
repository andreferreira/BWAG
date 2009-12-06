all: translator.o ../mmio.o
	g++ -O3 translator.o ../mmio.o

translator.o: translator.cpp
	g++ -O3 -c translator.cpp

../mmio.o: ../mmio.c ../mmio.h
	gcc -o ../mmio.o -c ../mmio.c

clean:
	rm -f translator.o
