#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include "process.h"

using std::string;
using std::cout;


Process::Process() {

}

Process::Process(int pd, int b, int a, int pri, int d, int i) {
	pid = pd;
	burst = b;
	arrival = a;
	priority = pri;
	deadline = d;
	io = i;
	timeLeft = b;
	timeSpent = 0;
	waited = 0;
	scheduled = false;
	turnaround = 0;
}

void Process::setVars(string s) {
	// Parse string into tokens
	std::istringstream iss(s);
	// put those into the variables
	iss >> pid >> burst >> arrival >> priority >> deadline >> io;
	timeLeft = burst;
	timeSpent = 0;
	waited = 0;
	scheduled = false;
	turnaround = 0;
}

void Process::execute() {
	timeLeft--;
	timeSpent++;
}

string Process::toString() {
	string result = std::to_string(pid) + "\t" + std::to_string(burst) + "\t" + std::to_string(arrival) + "\t" + std::to_string(priority) + "\t" + std::to_string(deadline) + "\t" + std::to_string(io);
	return result;
}
