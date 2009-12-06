#include <stdio.h>
#include <stdlib.h>
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
	
	for (i=0; i<nz; i++) {
		int ignore = fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val);
		I[i]--;  /* adjust from 1-based to 0-based */
		J[i]--;
	}
	
	printf("data;\nset VALUES :=");
	for (i=0; i<nz; i++)
		printf("%d ",i+1);
	printf(";\n\nparam Size:= %d;\n",N);
	printf("param InitialLine:=\n");
	for (i=0; i<nz; i++)
		printf("%d %d\n",i+1,I[i]+1);
	printf(";\n\nparam InitialColumn:=\n");
	for (i=0; i<nz; i++)
		printf("%d %d\n",i+1,J[i]+1);
	printf(";\n");
}

int main(int argc, char *argv[]) {	
	readInput(stdin);
}
