#ifndef PROCESS_H
#define PROCESS_H

class Process {
	#include <string.h>

	public:
		Process();
		Process(int, int, int, int, int, int);
		int pid, burst, arrival, priority, deadline, io, timeLeft, timeSpent, waited, endTime, turnaround;
		bool scheduled;
		void setVars(std::string);
		std::string toString();
		void execute();

};

#endif // PROCESS_H