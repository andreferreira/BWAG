#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
extern "C" {
#include "mmio.h"
}

using namespace std;

struct Individual {
	vector<int> order;
};

int N, nz; //dimension is NxN, number of non-null elements
int *I, *J; //coordinates of non-null elements

int population_size = 100; //default size, can be set by -p flag
int seed = 0xDEADCAFE; //default seed, can be set by -s flag
int generations = 10;

int bandwidth(const Individual &individual) {
	int band = 0;
	for (int i = 0; i < nz; i++) {
		band = max(band, abs(individual.order[I[i]] - individual.order[J[i]]));
	}
	return band;
}

void PMX(vector<int> a, vector<int> b, vector<int> &son, int location1, int location2) {
	int size = a.size();
	vector<int> inverseA(size);
	vector<int> inverseB(size);
	for (int i = 0; i < size; i++) {
		inverseA[a[i]] = i;
		inverseB[b[i]] = i;
		son[i] = -1;
	}
	
	vector<bool> inChild(size,false);
	for (int i = location1; i < location2; i++) {
		son[i] = a[i];
		inChild[a[i]] = true;
	}
	 
	for (int i = location1; i < location2; i++) {
		if (inChild[b[i]] == false) {
			int from = b[i];
			int toI = i;
			while (location1 <= toI && toI < location2) {
				toI = inverseB[a[toI]];
			}
			son[toI] = from;
			inChild[from] = true;
		}
	}
	
	for (int i = 0; i < size; i++) {
		if (inChild[b[i]] == false)
			son[i] = b[i];
	}
}

void two_point_crossover_vector(vector<int> a, vector<int> b, vector<int> &sonA, vector<int> &sonB) {
	int size = a.size();
	
	int location1 = rand() % size;
	int location2;
	do {
		location2 = rand() % size;
	} while (location1 == location2); //make sure they are different
	
	if (location1 > location2)
		swap(location1,location2); //location1 should be the smaller index
	
	sonA.resize(size);
	sonB.resize(size);

	PMX(a,b,sonA,location1,location2+1);
	PMX(b,a,sonB,location1,location2+1);
}

Individual mutation(Individual individual) {
	int size = individual.order.size();
	int location1 = rand() % size;
	int location2;
	do {
		location2 = rand() % size;
	} while (location1 == location2); //make sure they are different
	
	swap(individual.order[location1],individual.order[location2]);
	return individual;
}

vector<int> random_sequence(int n)
{
	vector<int> array(n);
    vector<int> count_array(n,0);
    
    for ( int i = 0; i < n; ++i)
    {
        array[i] = rand() % n;
        count_array[array[i]]++;  
    }

    for ( int i = 1; i < n; ++i)
        count_array[i] = count_array[i-1] + count_array[i];
    
    for ( int i = n - 1; i >= 0; --i)
        array[i] = --count_array[array[i]];

    return array;
}

vector<Individual> generateInitialPopulation(int population_size) {
	vector<Individual> population(population_size);
	for (int i = 0; i < population_size; i++) {
		population[i].order = random_sequence(N);
	}
	return population;
}

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
		if (I[i] > J[i])
			swap(I[i],J[i]);
	}
	
}

void readOptions(int argc, char *argv[]) {
	for(int i=1; i < argc; i++) {
		if(argv[i][0]=='-'){
			switch(argv[i][1]){
				case 'p':
					i++;
					population_size = atoi(argv[i]);
					break;
				case 'g':
					i++;
					generations = atoi(argv[i]);
					break;
				case 's':
					i++;
					seed = atoi(argv[i]);
					break;
				default:
					printf("Parametro %d: %s invalido\n",i,argv[i]);
					exit(1);
					break;
			}
		}else{
			printf("Parametro %d: %s invalido\n",i,argv[i]);
			exit(1);
		}
	}
}

int fitness(Individual individual) {
	return bandwidth(individual);
}

void printAsMatrix(Individual individual) {
	map<int, map<int,bool> > solution;
	for (int i = 0; i < nz; i++) {
		solution[individual.order[I[i]]][individual.order[J[i]]] = true;
		solution[individual.order[J[i]]][individual.order[I[i]]] = true;
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++)
			printf("%d ",solution[i][j]);
		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	readOptions(argc,argv);
	
	srand(seed);
	
	readInput(stdin);
	
	Individual normal;
	normal.order.resize(N);
	for (int i = 0; i < N; i++) {
		normal.order[i] = i;
	}
	vector<Individual> InitPopulation = generateInitialPopulation(population_size);
}
