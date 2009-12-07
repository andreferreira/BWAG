#include <stdio.h>
#include <stdlib.h>
#include <iostream>
extern "C" {
#include "../mmio.h"
}

using namespace std;


int N, nz; //dimension is NxN, number of non-null elements
int *I, *J; //coordinates of non-null elements

void readInput(FILE* f) {
    MM_typecode matcode;
	int i;
	int M;
	double val; //to be ignored
	
	if (mm_read_banner(f, &matcode) != 0) {
		printf("Could not process Matrix Market banner.\n");
		exit(1);
	}
	if (mm_read_mtx_crd_size(f, &M, &N, &nz) !=0) {
		printf("Problem reading matrix size");
        exit(1);
	}
	if (M != N) {
		printf("Can only handle square matrix.");
        exit(1);
	}
	I = (int *) malloc(nz * sizeof(int));
	J = (int *) malloc(nz * sizeof(int));
	bool isPattern = mm_is_pattern(matcode);
	for (i=0; i<nz; i++) {
		int readN;
		if (!isPattern)
			readN = fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val);
		else
			readN = fscanf(f, "%d %d\n", &I[i], &J[i]);
		if (!((readN == 2 && isPattern) || (readN == 3 && !isPattern))) {
			printf("Error reading matrix, read %d elements, expected %d",readN, isPattern ? 2 : 3);
			exit(1);
		}
		I[i]--;  /* adjust from 1-based to 0-based */
		J[i]--;
		if (I[i] > J[i])
			swap(I[i],J[i]);
	}
	
	printf("data;\nparam NVALUES := %d;\n",nz);
	printf("\nparam Size:= %d;\n",N);
	printf("param InitialLine:=\n");
	for (i=0; i<nz; i++)
		printf("%d %d\n",i+1,I[i]+1);
	printf(";\n\nparam InitialColumn:=\n");
	for (i=0; i<nz; i++)
		printf("%d %d\n",i+1,J[i]+1);
	printf(";\nend;\n");
}

int main(int argc, char *argv[]) {	
	readInput(stdin);
}
