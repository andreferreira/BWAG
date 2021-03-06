#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <math.h>
#include <time.h>
extern "C" {
#include "mmio.h"
}

using namespace std;

class Individual {
	public:
	vector<int> order;
	int fitness;
};

void swap(Individual &a, Individual &b) {
	swap(a.order,b.order);
	swap(a.fitness,b.fitness);
}

int N, nz; //dimension is NxN, number of non-null elements
int *I, *J; //coordinates of non-null elements

int population_size = 100; //default size, can be set by -p flag
int seed = 0xDEADCAFE; //default seed, can be set by -s flag
int timeToRun = 1;//set by -T flag
double percentTournament = 0.1; //number of individuals per tournament, can be set by -t flag
double mutationRate = 0.1; //odds of an individual to mutate in relation to Population Size, can be set by -m flag
int numberOfIslands = 1; //can be set by -i flag
int generationsToExchange = 10; //number of generations passed between each exchange, can be set by -e flag
double percentExchange = 0.1; //% of individuals from an island passed from one to another during an exchange, can be set by -x flag

double apercent = 0.2; //% of population at a caste, can be set by -a flag
double bpercent = 0.5; //% of population at b caste, can be set by -b flag

bool useIsland = true;

bool printSolution = false;
string plotFileName = ""; //file to save plot data, set by -G flag
ofstream plotFile;
int plotStep = 1;

void (*selection)(const vector<Individual> &, vector<Individual> &);
int bandwidth(const Individual &individual) {
	int band = 0;
	for (int i = 0; i < nz; i++) {
		band = max(band, abs(individual.order[I[i]] - individual.order[J[i]]));
	}
	return band;
}

vector<int> inverseA;
vector<int> inverseB;

void PMX(vector<int> a, vector<int> b, vector<int> &son, int location1, int location2) {
	int size = a.size();
	inverseA.resize(size);
	inverseB.resize(size);
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

void generateInitialPopulation(vector<Individual> &population) {
	for (int i = 0; i < population.size(); i++) {
		population[i].order = random_sequence(N);
	}
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
				case 'T':
					i++;
					timeToRun = atoi(argv[i]);
					break;
				case 's':
					i++;
					seed = atoi(argv[i]);
					break;
				case 't':
					i++;
					percentTournament = atoi(argv[i])/100.0;
					break;
				case 'm':
					i++;
					mutationRate = atoi(argv[i])/100.0;
					break;
				case 'i':
					i++;
					numberOfIslands = atoi(argv[i]);
					break;
				case 'e':
					i++;
					generationsToExchange = atoi(argv[i]);
					break;
				case 'P':
					printSolution = true;
					break;
				case 'x':
					i++;
					percentExchange = atoi(argv[i])/100.0;
					break;
				case 'a':
					i++;
					apercent = atoi(argv[i])/100.0;
					break;
				case 'b':
					i++;
					bpercent = atoi(argv[i])/100.0;
					break;
				case 'I':
					useIsland = true;
					break;
				case 'C':
					useIsland = false;
					break;
				case 'G':
					i++;
					plotFileName = argv[i];
					break;
				case 'g':
					i++;
					plotStep = atoi(argv[i]);
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

int fitness(const Individual &individual) {
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

Individual tournament(vector<Individual> population) {
	Individual best;
	best.fitness = 9999;
	int tournamentSize = ceil(percentTournament * population.size());
	for (int i = 0; i < tournamentSize; i++) {
		int selected = (rand() % (population.size() - i)) + i;
		swap(population[i],population[selected]);
		if (best.fitness > population[i].fitness)
			best = population[i];
	}
	return best;
}

void tournamentSelection(const vector<Individual> &population, vector<Individual> &nextPopulation) {
	for (int i = 0; i < nextPopulation.size(); i+=2) {
		Individual a = tournament(population);
		Individual b = tournament(population);
		Individual next2; //next1 is nextPopulation[i]
		two_point_crossover(a,b,nextPopulation[i],next2);
		if (i + 1 < nextPopulation.size())
			nextPopulation[i+1] = next2;
	}
}

void mutatePopulation(vector<Individual> &population) {
	for (int i = 0; i < population.size(); i++) {
		if (mutationRate > double(rand()) / RAND_MAX) {
			population[i] = mutation(population[i]);
		}
	}
}

void populationFitness(vector<Individual> &population) {
	//unsigned long long avg = 0;
	int size = population.size();
	for (int i = 0; i < size; i++) {
		population[i].fitness = fitness(population[i]);
		//avg += population[i].fitness;
	}
	//cout<<"Avg:"<<double(avg)/population.size()<<endl;
}

class Island {
	public:
	vector<Individual> population;
	vector<Individual> nextPopulation;
};

void (*exchange)(vector<Island> &);

//island 0 exchanges with island 1, 1 with 2, etc
void circularExchange(vector<Island> &archipelago) {
	int exchangedPop = percentExchange * archipelago[0].population.size();
	vector<Individual> temp(exchangedPop);
	copy(archipelago[0].population.begin(),
		     archipelago[0].population.begin() + exchangedPop,
			 temp.begin());
	for (int island = 1; island < archipelago.size(); island++) {
		copy(archipelago[island].population.begin(),
		     archipelago[island].population.begin() + exchangedPop,
			 archipelago[(island-1) % archipelago.size()].population.begin());
	}
	copy(temp.begin(),
		 temp.begin() + exchangedPop,
		 archipelago[archipelago.size()-1].population.begin());
}

unsigned long long sumFitness(vector<Individual> &population) {
	unsigned long long sum = 0;
	populationFitness(population);
	for (int i = 0; i < population.size(); i++) {
		sum += population[i].fitness;
	}
	return sum;
}

Individual GAIsland(vector<Individual> initPopulation) {
	clock_t start = clock();
	vector<Island> archipelago(numberOfIslands);
	int perIsland = initPopulation.size() / numberOfIslands;
	int left = initPopulation.size();
	for (int island = 0; island < numberOfIslands; island++) { //initialize islands
		
		int islandSize;
		if ((island+1)*perIsland >= initPopulation.size())
			islandSize = left;
		else
			islandSize = perIsland;
		left -= islandSize;
		archipelago[island].population.resize(islandSize);
		copy(initPopulation.begin() + island*perIsland,
			 (island+1)*perIsland >= initPopulation.size() ? initPopulation.end() 
														   : initPopulation.begin() + (island+1)*perIsland,
		     archipelago[island].population.begin());
		archipelago[island].nextPopulation.resize(archipelago[island].population.size());
	}
	
	Individual best;
	best.fitness = 9999;
	int clocksToRun = timeToRun * CLOCKS_PER_SEC;
	int g = 0;
	while (clock() - start < clocksToRun) {
		for (int island = 0; island < numberOfIslands; island++) {
			populationFitness(archipelago[island].population);
			for (int i = 0; i < archipelago[island].population.size(); i++)
				if (best.fitness > archipelago[island].population[i].fitness)
					best = archipelago[island].population[i];
			selection(archipelago[island].population,archipelago[island].nextPopulation);
			mutatePopulation(archipelago[island].nextPopulation);
			swap(archipelago[island].population,archipelago[island].nextPopulation);
		}
		if (plotFileName != "" && g % plotStep == 0) {
			unsigned long long totalFitness = 0;
			int totalPopulation = 0;
			for (int island = 0; island < numberOfIslands; island++) {
				totalFitness += sumFitness(archipelago[island].population);
				totalPopulation += archipelago[island].population.size();
			}
			plotFile<<g<<" "<<best.fitness<<" "<<totalFitness/totalPopulation<<endl;
		}
		if (g % generationsToExchange == 0)
				exchange(archipelago);
		g++;
	}
	cout<<"Generations:"<<g<<endl;
	return best;
}

bool compare(const Individual &a, const Individual &b) {
	if (a.fitness > b.fitness)
		return true;
	return false;
}

Individual GACaste(vector<Individual> initPopulation) {
	clock_t start = clock();
	vector<Individual> population = initPopulation;	
	vector<Individual> casteA(initPopulation.size()*apercent);
	vector<Individual> casteB(initPopulation.size()*bpercent);
	vector<Individual> casteC(initPopulation.size() - casteA.size() - casteB.size());
	
	Individual best;
	best.fitness = 9999;
	int clocksToRun = timeToRun * CLOCKS_PER_SEC;
	int g = 0;
	while (clock() - start < clocksToRun) {
		populationFitness(population);
		for (int i = 0; i < population.size(); i++)
				if (best.fitness > population[i].fitness)
					best = population[i];
		
		sort(population.begin(),population.end(),compare);
		selection(population,casteB);
		mutatePopulation(casteB);
		generateInitialPopulation(casteC);
		
		/*copy back*/
		copy(casteB.begin(),
			 casteB.end(),
			 population.begin() + casteA.size());
		copy(casteC.begin(),
			 casteC.end(),
			 population.begin() + casteA.size() + casteB.size());
		//cout<<"Generation:"<<g<<" Best:"<<best.fitness<<endl;
		if (plotFileName != "" && g % plotStep == 0) {
			unsigned long long totalFitness = sumFitness(population);
			int totalPopulation = population.size();
			plotFile<<g<<" "<<best.fitness<<" "<<totalFitness/totalPopulation<<endl;
		}
		g++;
	}
	cout<<"Generations:"<<g<<endl;
	return best;
}

int main(int argc, char *argv[]) {
	readOptions(argc,argv);
	
	srand(seed);
	selection = tournamentSelection;
	
	readInput(stdin);
	Individual (*GA)(vector<Individual>);
	if (useIsland)
		GA = GAIsland;
	else
		GA = GACaste;
	exchange = circularExchange;
	
	if (plotFileName != "") {
		ofstream commandFile(plotFileName.c_str());
		plotFile.open((plotFileName+".dat").c_str());
		commandFile<<"set terminal png\nset output '"<<plotFileName<<".png'"<<endl;
		commandFile<<"plot \""<<(plotFileName+".dat")<<"\" ";
		commandFile<<"using 1:3 title \"Average Fitness\"";
		commandFile<<", \""<<(plotFileName+".dat")<<"\" ";
		commandFile<<"using 1:2 title \"Best Fitness\""<<endl;
	}
	
	vector<Individual> initPopulation(population_size);
	generateInitialPopulation(initPopulation);
	Individual solution = GA(initPopulation);
	if (plotFileName != "") {
		plotFile<<"e"<<endl;
	}
	cout<<solution.fitness<<endl;
	for (int i = 0; i < solution.order.size(); i++) {
		cout<<solution.order[i]<<" ";
	}
	
	return 0;
}
