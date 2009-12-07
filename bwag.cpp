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
	int fitness;
};

int N, nz; //dimension is NxN, number of non-null elements
int *I, *J; //coordinates of non-null elements

int population_size = 100; //default size, can be set by -p flag
int seed = 0xDEADCAFE; //default seed, can be set by -s flag
int generations = 10;
double percenttournament = 0.1; //number of individuals per tournament
int tournamentSize; //percenttournament * population_size
double mutationRate = 0.1; //odds of an individual to mutate in relation to Population Size
int numberOfIslands = 1;

void (*selection)(const vector<Individual> &, vector<Individual> &);
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

void two_point_crossover(Individual a, Individual b, Individual &sonA, Individual &sonB) {
	int size = a.order.size();
	
	int location1 = rand() % size;
	int location2;
	do {
		location2 = rand() % size;
	} while (location1 == location2); //make sure they are different
	
	if (location1 > location2)
		swap(location1,location2); //location1 should be the smaller index
	
	sonA.order.resize(size);
	sonB.order.resize(size);

	PMX(a.order,b.order,sonA.order,location1,location2+1);
	PMX(b.order,a.order,sonB.order,location1,location2+1);
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
				case 't':
					i++;
					percenttournament = atof(argv[i]);
					break;
				case 'm':
					i++;
					mutationRate = atof(argv[i]);
					break;
				case 'i':
					i++;
					numberOfIslands = atoi(argv[i]);
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

bool compareIndividual(const Individual &a, const Individual &b) {
	if (a.fitness < b.fitness)
		return true;
	return false;
}

Individual tournament(vector<Individual> population) {
	Individual best;
	best.fitness = -9999;
	for (int i = 0; i < tournamentSize; i++) {
		int selected = (rand() % (population.size() - i)) + i;
		swap(population[i],population[selected]);
		if (compareIndividual(best,population[i]))
			best = population[i];
	}
	return best;
}

void tournamentSelection(const vector<Individual> &population, vector<Individual> &nextPopulation) {
	nextPopulation.resize(population.size());
	for (int i = 0; i < population.size(); i+=2) {
		Individual a = tournament(population);
		Individual b = tournament(population);
		Individual next2; //next1 is nextPopulation[i]
		two_point_crossover(a,b,nextPopulation[i],next2);
		if (i + 1 < population.size())
			nextPopulation[i+1] = next2;
	}
}

void mutatePopulation(vector<Individual> &population) {
	for (int i = 0; i < population.size(); i++) {
		if (rand() % population.size() <= int(mutationRate * population.size()))
			population[i] = mutation(population[i]);
	}
}

void populationFitness(vector<Individual> &population) {
	for (int i = 0; i < population.size(); i++) {
		population[i].fitness = fitness(population[i]);
	}
}

struct Island {
	vector<Individual> population;
	vector<Individual> nextPopulation;
};

Individual GAIsland(vector<Individual> initPopulation) {
	vector<Island> archipelago(numberOfIslands);
	int perIsland = initPopulation.size() / numberOfIslands;
	for (int island = 0; island < numberOfIslands; island++) { //initialize islands
		copy(initPopulation.begin() + island*perIsland,
			 (island+1)*perIsland >= initPopulation.size() ? initPopulation.end() 
														   : initPopulation.begin() + (island+1)*perIsland,
		     archipelago[island].population.begin());
		archipelago[island].nextPopulation.resize(archipelago[island].population.size());
	}
	
	for (int island = 0; island < numberOfIslands; island++) {
		selection(archipelago[island].population,archipelago[island].nextPopulation);
		mutatePopulation(archipelago[island].nextPopulation);
		swap(archipelago[island].population,archipelago[island].nextPopulation);
	}
}

int main(int argc, char *argv[]) {
	readOptions(argc,argv);
	
	srand(seed);
	tournamentSize = percenttournament * population_size;
	selection = tournamentSelection;
	
	readInput(stdin);
	
	Individual normal;
	normal.order.resize(N);
	for (int i = 0; i < N; i++) {
		normal.order[i] = i;
	}
	vector<Individual> initPopulation = generateInitialPopulation(population_size);
}
