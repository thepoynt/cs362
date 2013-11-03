#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include "process.h"

using std::cout;
using std::cin;
using std::vector;
using std::string;


int runMFQS();
int runRTS();
int runHS();
int readProcesses(string);
void stringToProcess(string);
void printProcesses(vector<Process>);
void putNextOnQueueByDeadline(vector<Process>*, int);

int scheduler;
int numQueues = 3;
int tq;
vector<Process> processes;


int main () {
   bool done = false;

   // Read in processes from file
   if (!readProcesses("testfile")) {
      perror("Error reading file");
      exit(1);
   }
   printProcesses(processes);

   // Get user input
   while (!done) {
      printf("Please enter the number of the type of Scheduler you want to run:\n   1: MFQS\n   2: RTS\n   3: HS\n");
      cin >> scheduler;

      // Determine which scheduler to use
      switch (scheduler) {
         {case 1:
            bool validNum = false; 
            while (!validNum) {
               printf("How many queues should be used (between 1 and 5)? ");
               cin >> numQueues;
               if(numQueues < 5 && numQueues > 0) {
                  validNum = true;
               }
            }
            printf("Please enter the Time Quantum: ");
            cin >> tq;
            runMFQS();
            done = true;
            break;
         }{ 
         case 2:
            printf("Please enter the Time Quantum: ");
            cin >> tq;
            runRTS();
            done = true;
            break;
         }{
         case 3:
            printf("Please enter the Time Quantum: ");
            cin >> tq;
            runHS();
            done = true;
            break;
         }{
         default:
            break;
         }
      }
   }

   // I dunno, do something here to print out results and such...

   return 0; 
}

int readProcesses(string filename) {
   string line;
   std::ifstream myfile (filename);
   int i = 0;
   if (myfile.is_open()) {
      while ( getline (myfile,line) ) {
         if (i > 0 && line.find("-") == string::npos) { //skip first line - header
            // make new Process object from line
            Process p;
            p.setVars(line);
            processes.push_back(p); // add it to our vector
         }
         i++;
      }
      myfile.close();
      return 1;
   } else {
      cout << "Unable to open file";
      return 0;
   } 
}

int runMFQS() {
   printf("Running MFQS with %d queues...\n", numQueues);
   return 0;
}

int runRTS() {
   printf("Running RTS...\n");
   vector<Process> queue; // Queue for processes
   int clk = 0; // Clock counter
   bool processesLeft = true; // Are there any processes still running?

   while (processesLeft) {
      // Bring in any new processes at current clk, putting them in queue based on deadline
      putNextOnQueueByDeadline(&queue, clk);

      // "Execute" the first process in the queue by decrementing it's timeLeft variable


      // if timeLeft is 0 for current process, remove it


      clk++;
   }
   return 0;
}

int runHS() {
   printf("Running HS...\n");
   return 0;
}

void putNextOnQueueByDeadline(vector<Process>* queue, int clk) {
   for (int i=0; i<processes.size(); i++) {
      if (processes[i].arrival == clk) { // get all processes coming in at this clock tick
         Process temp = processes[i];
         if (queue.size() == 0) { // if there's no processes in the queue, just add it
            queue.push_front(temp);
            processes.erase(processes.begin() + i);
         } else {
            for (int j=0; j<*queue.size(); j++) {
               if (temp.deadline > *queue[j].deadline) { // put it in before the first one with a later deadline
                  *queue.insert(queue.begin() + (j-1), temp);
                  processes.erase(processes.begin() + i);
               }
            }
         }
      }
   }
}

void printProcesses(vector<Process> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i].pid << "\t" << v[i].burst << "\t" << v[i].arrival << "\t" << v[i].priority << "\t" << v[i].deadline << "\t" << v[i].io << "\n";
   }
}


