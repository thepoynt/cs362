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
#include <algorithm>

using std::cout;
using std::cin;
using std::vector;
using std::deque;
using std::string;
using std::make_heap;

#define DEBUG 0;


int runMFQS();
int runRTS();
int runHS();
int readProcesses(string);
void stringToProcess(string);
void printProcesses(deque<Process>);
void putNextOnQueueByDeadline(int);
bool thereAreProcessesLeftToBeScheduled();
void putNextOnQueueByPriorityHS(int);
void scheduleMeHS(int, Process);    

int scheduler;
int numQueues = 3;
int tq;
deque<Process> processes;
deque<Process> queue;
deque<Process> IOqueue;
int totalWaitTime = 0;
int totalTurnaroundTime = 0;
int totalProcessesScheduled = 0;


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

   // Results

   // Average Waiting Time
   double avgWaitTime = totalWaitTime;
   avgWaitTime = avgWaitTime/totalProcessesScheduled;
   cout <<  "AWT: " << avgWaitTime << "\n";

   // Average Turnaround Time
   double avgTurn = totalTurnaroundTime;
   avgTurn = avgTurn/totalProcessesScheduled;
   cout << "ATT: " << avgTurn << "\n";


   // Total number of processes scheduled
   cout << "NP: " << totalProcessesScheduled << "\n";

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

      // Bring in any new processes at current clok, putting them in queue based on deadline
      putNextOnQueueByDeadline(clk);

      if (queue.size() > 0) { // No need to do anything if there's no processes on queue
         // Check through processes in queue to see if there are any that cannot finish
         for (int i=0; i<queue.size(); i++) {
            if (queue[i].deadline < (clk + queue[i].timeLeft)) {
               // this process cannot complete by its deadline, so should be removed. Its contribution to waiting time will still be factored
               #ifdef DEBUG
                  cout << clk << ": Process " << queue[i].pid << " ran out of time!\n";
               #endif
               totalProcessesScheduled--; // not counted towards total number of processes scheduled
               totalTurnaroundTime -= (clk - queue[i].arrival); // turnaround time also not counted
               queue.erase(queue.begin() + i);
            }
         }

         // "Execute" the first process in the queue
         queue[0].execute();
         #ifdef DEBUG
            cout << clk << ": Executing " << queue[0].pid << "\n";
         #endif
         // increment the waiting time for all other processes in queue
         for (int i = 1; i<queue.size(); i++) {
            totalWaitTime++;
            totalTurnaroundTime++; // this gets incremented for all other processes, as well as the current process (all processes on queue are having their turnaround time incremented, while only all non-running processes get wait time incremented)
         }
         totalTurnaroundTime++; // Increment for current process. See comment above ^^

         // if timeLeft is 0 for current process, remove it
         if (queue[0].timeLeft == 0) {
            #ifdef DEBUG
               cout << clk << ": Popping process (" << queue[0].pid << ") from queue\n";
            #endif
            queue[0].endTime = clk;
            queue[0].turnaround = clk - queue[0].arrival;
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
    int clk = 0;
    int tqcount = 0;
    
    while (thereAreProcessesLeftToBeScheduled() || queue.size() > 0) {
        //determine where to put in queue based on priority
        putNextOnQueueByPriorityHS(clk);
        tqcount++;
        //only do the following if the queue has something in it, and isn't a clock interrupt time
        if (queue.size() > 0){
            
            //only execute if less than I/O time
            if(tqcount < (tq-1)) {
               queue[0].execute();
               queue[0].lastrun = clk;
   				#ifdef DEBUG
   				cout << clk << ": Executing " << queue[0].pid << "\n";
   				#endif
                
               //if process if completed, kick it from the queue
               if(queue[0].timeLeft == 0){
                  queue[0].endTime = clk;
                  queue[0].turnaround = clk - queue[0].arrival;
                  queue.pop_front();
                }
                
            }else if(tqcount == tq-1 && queue[0].timeLeft == 0){ //if process finishes on last tq tick
                                
            }else{  //this only runs on clock interrupt cases
                
                //change priority of process based on clock ticks ran
                //if the change would put the dynamic priority lower than the original priority, set it to the original priority
                if(queue[0].dynamicpriority - tqcount < queue[0].priority){
                    queue[0].dynamicpriority = queue[0].priority;   
                }else{ //otherwise, just subtract the tqcount 
                    queue[0].dynamicpriority = queue[0].dynamicpriority - tqcount;
                }
                //if this has I/O, run it
                if(queue[0].io > 0){
                    IOqueue.push_back(queue[0]);
                    queue.pop_front();
                }
            }
        }
        
        //loop through IOqueue, searching for processes that are done with I/O
        for(int i = 0; i < IOqueue.size(); ++i){
            //decrease IO time left
            IOqueue[i].io--;
            //increase prio according to IO time
            IOqueue[i].dynamicpriority++;
            //if IO is now 0, put onto running queue
            if(IOqueue[i].io == 0){
                Process p = IOqueue[i];
                IOqueue.erase(IOqueue.begin() + i);
                scheduleMeHS(clk, p);
            }
            //if not == 0, do nothing else.
        }
        
        //check for starving processes, mod priority as necessary
        if(clk % 100 == 0){
            for(int i = 0; i < queue.size(); ++i){
                //only change if 
                if(clk - queue[i].lastrun >= 100){
                    //only change if under 50
                    if(queue[i].dynamicpriority < 50){
                        //if increasing by 10 puts over 50, set to 50
                        if((queue[i].dynamicpriority + 10 > 50)){
                            queue[i].dynamicpriority = 50;
                        }else{
                            queue[i].dynamicpriority = queue[i].dynamicpriority + 10;
                        }
                    }
                }
            }
        }
        
        clk++;
    }
    
    
    return 0;
}

void putNextOnQueueByDeadline(int clk) {
   for (int i=0; i<processes.size(); i++) {
      if (processes[i].arrival == clk && !processes[i].scheduled) { // get all processes coming in at this clock tick
         totalProcessesScheduled++;
         if (queue.size() == 0) { // if there's no processes in the queue, just add it
            #ifdef DEBUG
               cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of queue\n";
            #endif
            queue.push_front(processes[i]);
            processes[i].scheduled = true;
         } else {
            for (int j=0; j<queue.size(); j++) {
               if (processes[i].deadline < queue[j].deadline) { // put it in before the first one with a later deadline
                  #ifdef DEBUG
                     cout << clk << ": Putting process (" << processes[i].pid << ") at position " << j << " in queue\n";
                  #endif
                  queue.insert(queue.begin() + (j), processes[i]);
                  processes[i].scheduled = true;
                  j = queue.size();
               }
            }
         }
      }
   }
}

//HS nextOnQueue
void putNextOnQueueByPriorityHS(int clk) {
    for (int i=0; i<processes.size(); i++) {
        if (processes[i].arrival == clk && !processes[i].scheduled) { // get all processes coming in at this clock tick
            totalProcessesScheduled++;
            
            //check legitimacy of the current priority, change dynamic priority if needed
            if(processes[i].priority > 99){
                processes[i].dynamicpriority = 99;
            }else if(processes[i].priority < 0){
                processes[i].dynamicpriority = 0;   
            }else{
                processes[i].dynamicpriority = processes[i].priority;   
            }
            if (queue.size() == 0) { // if there's no processes in the queue, just add it
                #ifdef DEBUG
                   cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of queue\n";
                #endif
                queue.push_front(processes[i]);
                processes[i].scheduled = true;
            } else {
                //if there's processes in the queue, schedule accordingly
                scheduleMeHS(clk, processes[i]);
            }
        }
    }
}

void scheduleMeHS(int clk, Process p){
    // just as what we had previously in the putNextOnQueueByPriorityHS() function, but since it can also be used for post-IO scheduling, i separated it
    for (int j=0; j<queue.size(); j++) {
        if (p.dynamicpriority > queue[j].dynamicpriority) { // put it in before the first one with a later deadline
            #ifdef DEBUG
            cout << clk << ": Putting process (" << p.pid << ") at position " << j << " in queue\n";
            #endif
            queue.insert(queue.begin() + (j), p);
            j = queue.size();
        }
    }
}

bool thereAreProcessesLeftToBeScheduled() {
   bool result = false;
   for (int i=0; i<processes.size(); i++) {
      if (!processes[i].scheduled) {
         result = true;
         i = processes.size();
      }
   }
   return result;
}

void printProcesses(deque<Process> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i].toString() << "\n";
   }
}
