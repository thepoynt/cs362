#ifndef PROCESSQUEUE_H
#define PROCESSQUEUE_H

class ProcessQueue {
	#include <deque>
	#include "process.h"

	public:
		ProcessQueue();
		ProcessQueue(Process);
		std::deque<Process> processes;
		int priority;
		void addProcess(Process);
		std::string toString();
		bool operator < (const ProcessQueue& p) const {
			return (priority < p.priority);
		}

};

#endif // PROCESSQUEUE_H
