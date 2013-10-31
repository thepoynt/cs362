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
}

void Process::setVars(string s) {
	// Parse string into tokens
	std::istringstream iss(s);
	iss >> pid >> burst >> arrival >> priority >> deadline >> io;
	// std::vector<string> tokens;
	// copy(std::istream_iterator<string>(iss),
	// 	std::istream_iterator<string>(),
	// 	std::back_inserter<std::vector<string> >(tokens));

	// // convert tokens into 
	// std::vector<int> v;
	// for (int i = 0; i < tokens.size(); i++) {
	//   	v.push_back(atoi(tokens[i].c_str()));
	// }

	// pid = v[0];
	// burst = v[1];
	// arrival = v[2];
	// priority = v[3];
	// deadline = v[4];
	// io = v[5];
}
