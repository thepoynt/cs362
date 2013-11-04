#include <iostream>
#include <vector>
#include <deque>
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
using std::deque;
using std::string;

#define DEBUG ;


int runMFQS();
int runRTS();
int runHS();
int readProcesses(string);
void stringToProcess(string);
void printProcesses(deque<Process>);
void putNextOnQueueByDeadline(int);
int avgWaitTime();
bool thereAreProcessesLeftToBeScheduled();

int scheduler;
int numQueues = 3;
int tq;
deque<Process> processes;
deque<Process> queue;


int main () {
   bool done = false;

   // Read in processes from file
   if (!readProcesses("testfile2")) {
      perror("Error reading file");
      exit(1);
   }
   #ifdef DEBUG
      printProcesses(processes);
   #endif

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
               if(numQueues < 6 && numQueues > 0) {
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

   // I dunno, maybe do something here to print out results and such...

   return 0; 
}

int readProcesses(string filename) {
   string line;
   std::ifstream myfile (filename.c_str());
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
   int clk = 0; // Clock counter

   while (thereAreProcessesLeftToBeScheduled() || queue.size() > 0) {
      // cout << "clock: " << clk;
      // Bring in any new processes at current clk, putting them in queue based on deadline
      putNextOnQueueByDeadline(clk);

      if (queue.size() > 0) {
         // Check through processes in queue to see if there are any that cannot finish


         // "Execute" the first process in the queue by decrementing it's timeLeft variable
         queue[0].execute();
         cout << clk << ": Executing " << queue[0].pid << "\n";
         // increment the waiting time for all other processes in queue
         for (int i = 1; i<queue.size(); i++) {
            queue.waited++;
         }

         // if timeLeft is 0 for current process, remove it
         if (queue[0].timeLeft == 0) {
            #ifdef DEBUG
               cout << clk << ": Popping process (" << queue[0].pid << ") from queue\n";
            #endif
            queue.pop_front();
         }
      }

      clk++;
   }
   printf("Done with RTS!\n");
   return 0;
}

int runHS() {
   printf("Running HS...\n");
   return 0;
}

void putNextOnQueueByDeadline(int clk) {
   for (int i=0; i<processes.size(); i++) {
      if (processes[i].arrival == clk && !processes[i].sheduled) { // get all processes coming in at this clock tick
         // Process temp = processes[i];
         if (queue.size() == 0) { // if there's no processes in the queue, just add it
            #ifdef DEBUG
               cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of queue\n";
            #endif
            queue.push_front(processes[i]);
            processes[i].sheduled = true;
         } else {
            for (int j=0; j<queue.size(); j++) {
               if (processes[i].deadline < queue[j].deadline) { // put it in before the first one with a later deadline
                  #ifdef DEBUG
                     cout << clk << ": Putting process (" << processes[i].pid << ") at position " << j << " in queue\n";
                  #endif
                  queue.insert(queue.begin() + (j), processes[i]);
                  processes[i].sheduled = true;
                  j = queue.size();
               }
            }
         }
      }
   }
}

thereAreProcessesLeftToBeScheduled()

void printProcesses(deque<Process> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i].toString() << "\n";
   }
}

int avgWaitTime() {
   int avg;

   return avg;
}





