#ifndef PROCESS_H
#define PROCESS_H

class Process {
	#include <string.h>
	#include <vector>

	public:
		Process();
		Process(int, int, int, int, int, int);
		int pid, burst, arrival, priority, deadline, io, timeLeft, timeSpent, waited, endTime, turnaround, dynamicpriority, lastrun, ioLeft, timeInThisQuantum;
		bool scheduled;
		void setVars(std::string);
		std::string toString();
		void execute();
		bool operator < (const Process& p) const {
			if (dynamicpriority < p.dynamicpriority) {
		    	return true;
			} else if (dynamicpriority > p.dynamicpriority) {
			  	return false;
			} else { // dynamicpriority is equal
			  	return (pid < p.pid);
			}
		}

};

#endif // PROCESS_H
