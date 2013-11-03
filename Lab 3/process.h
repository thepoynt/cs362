#ifndef PROCESS_H
#define PROCESS_H

class Process {
	#include <string.h>

	public:
		Process();
		Process(int, int, int, int, int, int);
		int pid, burst, arrival, priority, deadline, io, timeLeft;
		void setVars(std::string);


};

#endif // PROCESS_H