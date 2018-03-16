#pragma once
#include<cmath>
#include<algorithm>

const int SeqLen = 15;
const int RegNum = 3;
const int FuncLen = 6;
const int PointNum = 50;

void executeOnce();

void initReg(double);

struct Point {
	double x;
	double y;
};

struct Instruction {
	int dest;
	int op;
	int source1;
	int source2;
	Instruction() {
		dest = rand() % RegNum;
		op = rand() % FuncLen;
		source1 = rand() % RegNum;
		source2 = rand() % RegNum;
	}
	Instruction(int a, int b) {
		dest = a;
		op = b;
		source1 = rand() % RegNum;
		source2 = rand() % RegNum;
	}
	void output() {
		std::cout << dest << " : "
			<< op << " : "
			<< source1 << " : "
			<< source2 << std::endl;
	}
};


struct individual
{
	Instruction sequence[SeqLen];
	float mFitness;  // 0 - errors, the bigger, the better
};
enum Functions { OP_Plus, OP_Minus, OP_Mul, OP_Divide , OP_Log, OP_Exp};

double f(double x) {
	return 4.251 * x * x + 2 * log(x) + 7.243 * exp(x);
}