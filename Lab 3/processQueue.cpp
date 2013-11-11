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
#include <deque>
#include "process.h"
#include "processQueue.h"


using std::deque;


ProcessQueue::ProcessQueue() {
	priority = -1;
}

ProcessQueue::ProcessQueue(Process p) {
	processes.push_back(p);
	priority = p.dynamicpriority;
}

void ProcessQueue::addProcess(Process p) {
	processes.push_back(p);
	if (priority == -1) { 
		// this is the first process added - set priority
		priority = p.dynamicpriority;
	}
}

std::string ProcessQueue::toString() {
	std::ostringstream oss;
	oss << processes[0].pid;
	for (int i=1; i<processes.size(); i++) {
		oss << ", " << processes[i].pid;
	}
	return oss.str();
}
