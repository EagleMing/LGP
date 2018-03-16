#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include"main.h"

using namespace std;

Point points[PointNum];
double registers[PointNum][RegNum];

class universe
{
public:
	
	universe(size_t population);
	void measureFitness();
	void executeOnce(Instruction sequence[]);
	void orderResults();
	void printBestResult();
	void reproduce(float fitnessDeletionThreshold, float eliteThreshold);
	void mutate(float mutationRate);

private:
	vector<individual> mPopulation;  // our individuals to evolve
};

universe::universe(size_t population)
{
	for (int i = 0; i < population; ++i) {
		mPopulation.push_back(individual());
	}
	for (int i = 0; i < PointNum; ++i) { // [3,5]
		points[i].x = 3 + i * 0.04;
		points[i].y = f(points[i].x);
	}
}

void universe::orderResults()
{
	sort(mPopulation.begin(), mPopulation.end(),
		[](const individual &a, const individual &b)
	{
		return a.mFitness > b.mFitness;
	});
}


void universe::printBestResult()
{
	auto &ind = mPopulation[0];
	cout << ind.mFitness << endl;
	for (int i = 0; i < SeqLen; ++i) {
		ind.sequence[i].output();
	}

	//auto &output = myMachine.mOutputStream;
	//cout << "Best output (len=" << output.size() << ") = \"" << output << "\"" << endl;
	//cout << "Best genome (" << ind.mFitness << ") = \'" << ind.mGenome << "\'" << endl;
}

void initRegisters() {
	for (int i = 0; i < PointNum; ++i) {
		for (int j = 0; j < RegNum; ++j) {
			registers[i][j] = points[i].x;
		}
	}
}

void universe::measureFitness()
{
	for (auto &ind : mPopulation) {

		initRegisters(); // For each individual, registers need to be reinitialized
		executeOnce(ind.sequence);

		ind.mFitness = 0.0;
		for (int i = 0; i < PointNum; ++i) {
			ind.mFitness -= abs(points[i].y - registers[i][1]) * abs(points[i].y - registers[i][1]); // Square error or not?
		}

	}
	

}

void universe::executeOnce(Instruction sequence[])
{
	for (int i = 0; i < SeqLen; ++i) {
		Instruction tmp = sequence[i];

		switch (tmp.op) {
		case OP_Plus: 
			for (int i = 0; i < PointNum; ++i) {
				registers[i][tmp.dest] = registers[i][tmp.source1] + registers[i][tmp.source2];
			}
			break;
		case OP_Minus:
			for (int i = 0; i < PointNum; ++i) {
				registers[i][tmp.dest] = registers[i][tmp.source1] - registers[i][tmp.source2];
			} 
			break;
		case OP_Mul:
			for (int i = 0; i < PointNum; ++i) {
				registers[i][tmp.dest] = registers[i][tmp.source1] * registers[i][tmp.source2];
			} 
			break;
		case OP_Divide:
			for (int i = 0; i < PointNum; ++i) {
				if (registers[i][tmp.source2] != 0) {
					registers[i][tmp.dest] = registers[i][tmp.source1] / registers[i][tmp.source2];
				}else {
					// Need some special operations here
				}
			}
			break;
		case OP_Exp:
			for (int i = 0; i < PointNum; ++i) {
				registers[i][tmp.dest] = exp(registers[i][tmp.source1]);
			}
			break;
		case OP_Log:
			for (int i = 0; i < PointNum; ++i) {
				if (registers[i][tmp.source1] != 0) {
					registers[i][tmp.dest] = log(registers[i][tmp.source1]);
				}
				else {
					// Need some special operations here
				}
			}
			break;
		default:
			break;
		}
	}
}

void universe::reproduce(float fitnessDeletionThreshold, float eliteThreshold)
{
	size_t fitnessThreshIdx = mPopulation.size() * (1.0 - fitnessDeletionThreshold);
	size_t eliteThreshIdx = mPopulation.size() * (1.0 - eliteThreshold);

	for (size_t replaceIdx = fitnessThreshIdx + 1; replaceIdx < mPopulation.size(); ++replaceIdx) {
		// Select two parents, one from above the elite threshold, the other will be
		// a random selection above the fitness deletion threshold, which might or might
		// not be from the elite section.

		size_t parent0Idx = rand() % (eliteThreshIdx + 1);
		size_t parent1Idx = rand() % (fitnessThreshIdx + 1);

		// Favor higher fitness parents by finding another set of random
		// indices between 0 and the indices just calculated:

		parent0Idx = rand() % (parent0Idx + 1);
		parent1Idx = rand() % (parent1Idx + 1);

		individual &parent0 = mPopulation[parent0Idx];
		individual &parent1 = mPopulation[parent1Idx];


		// 暂时这样，能跑之后再改
		int crossPoint = rand() % (SeqLen - 1);

		for (int i = 0; i <= crossPoint; ++i) {
			mPopulation[replaceIdx].sequence[i] = parent0.sequence[i];
		}
		for (int i = crossPoint + 1; i < SeqLen; ++i) {
			mPopulation[replaceIdx].sequence[i] = parent1.sequence[i];
		}
		
		mPopulation[replaceIdx].mFitness = 0.0;
	}
}

void universe::mutate(float mutationRate)
{

	for (auto &ind : mPopulation) {
		if (rand() / (float)RAND_MAX < mutationRate) {
			int mutateInd = rand() % SeqLen;
			ind.sequence[mutateInd] = Instruction();
		}
	}
}

struct commandLineParameters
{
		commandLineParameters(void) :
		fitnessThreshold(0.5), // -F
		eliteThreshold(0.9),   // -E
		maxGenerations(20000),   // -g
		mutationRate(0.01),    // -M
		population(100)     // -p		
	{}

	float fitnessThreshold;
	float eliteThreshold;
	size_t maxGenerations;
	float mutationRate;
	size_t population;

	
private:
	template <typename T>
	void getArg(T *pRetVal, int &i, int argc, char **argv)
	{
		if (argv[i][2] != '\0') {
			istringstream(&argv[i][2]) >> *pRetVal;
			return;
		}
		else if (i < argc - 1) {
			istringstream(argv[++i]) >> *pRetVal;
			return;
		}

		cout << "synopsis: evolve [-p population] [-g maxGenerations]" << endl;
		cout << "[-k maxCyclesToRun] [-t numThreads] [-L archLength] [-W archWidth]" << endl;
		cout << "[-M mutationRate] [-F fitnessThreshold] [-E eliteThreshold] [startingProgram]" << endl;
		exit(1);  // alternatively, return a status to the caller
	}
};


int main(int argc, char **argv)
{
	// Disable the next line to debug with a predictable random sequence:
	srand(time(NULL));

	commandLineParameters args;

	// Make a universe with a bunch of individuals:
	universe myWorld(args.population);

	// Evolve the population
	const size_t everyNth = 32;
	for (size_t generationNumber = 0; generationNumber < args.maxGenerations; ++generationNumber) {
		
		myWorld.measureFitness();
		myWorld.orderResults();
		/*myWorld.printBestResult();*/
		if (generationNumber % everyNth == 0) {
			cout << "\nGeneration " << generationNumber << endl;
			myWorld.printBestResult();

		}
		myWorld.reproduce(args.fitnessThreshold, args.eliteThreshold);
		myWorld.mutate(args.mutationRate);
	}

	return 0;
}

