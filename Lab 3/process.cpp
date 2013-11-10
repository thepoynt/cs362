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
	timeLeft = b; // time needed to complete
	timeSpent = 0; // total time spent running
	waited = 0; // total time spent waiting
	scheduled = false; // has this been scheduled?
	endTime = 0; // clock tick at which this process finished
	turnaround = 0; // time between arrival and finishing
    dynamicpriority = pri; // dynamic priority (for HS)
    lastrun = 0; // last clock tick at which this process ran (for HS)
    ioLeft = io; // time in IO left to wait (for HS)
    timeInThisQuantum = 0; // running time left before hitting the time quantum
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
	endTime = 0;
	turnaround = 0;
    dynamicpriority = priority;
    lastrun = 0;
    ioLeft = io;
    timeInThisQuantum = 0;
}

void Process::execute() {
	timeLeft--;
	timeSpent++;
}

string Process::toString() {
	std::ostringstream oss;
	oss << pid << "\t" << burst << "\t" << arrival << "\t" << priority << "\t" << deadline << "\t" << io;
	// string result = std::to_string(pid) + "\t" + std::to_string(burst) + "\t" + std::to_string(arrival) + "\t" + std::to_string(priority) + "\t" + std::to_string(deadline) + "\t" + std::to_string(io);
	return oss.str();;
}
