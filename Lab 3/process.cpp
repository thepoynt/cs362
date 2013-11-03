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
}

void Process::setVars(string s) {
	// Parse string into tokens
	std::istringstream iss(s);
	// put those into the variables
	iss >> pid >> burst >> arrival >> priority >> deadline >> io;
}

