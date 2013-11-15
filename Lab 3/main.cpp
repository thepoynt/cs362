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
#include "processQueue.h"
#include <algorithm>
#include <cmath>

using std::cout;
using std::cin;
using std::vector;
using std::deque;
using std::string;
using std::make_heap;

// #define DEBUG 0;


int runMFQS();
int runRTS();
int runHS();
int readProcesses(string);
void stringToProcess(string);
void putNextOnQueueByDeadline(int);
bool thereAreProcessesLeftToBeScheduled();
void putNextOnQueueByPriorityHS(int);
void scheduleMeHS(int, Process);    
void putNextOnProcessQueues(int);
void addToProcessQueues(Process, int);
void printProcesses(deque<Process>);
void printQueues(deque<ProcessQueue>);
void printIntVector(vector<int>);
void agingProcesses();
void putNextOnQueueByPrio(int);


int scheduler;
int numQueues = 3;
int tq;
int agingtime;
deque<Process> processes;     // for all processes read in from file
deque<Process> queue;         // process queue for RTS
deque<Process> iOqueue;       // queue for processes doing IO for HWS
deque<Process> finished;      // for all processes that sucessfully finished
deque<Process> attempted;     // for all processes, after they've been processed (whether they finished or not) so we have data about their execution
deque<ProcessQueue> queues;   // A queue of queues for HWS
int totalWaitTime = 0;
int totalTurnaroundTime = 0;
int totalProcessesScheduled = 0;
std::ostringstream gannt;



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
      printf("Please enter the number of the type of Scheduler you want to run:\n   1: MFQS\n   2: RTS\n   3: HWS\n");
      cin >> scheduler;

      // Determine which scheduler to use
      switch (scheduler) {
         {case 1:
            bool validNum = false; 
            while (!validNum) {
               printf("How many queues should be used (between 2 and 5)? ");
               cin >> numQueues;
               if(numQueues < 6 && numQueues > 1) {
                  validNum = true;
               }
            }
            printf("Please enter the Time Quantum: ");
            cin >> tq;
            printf("Please enter the Aging Time: ");
            cin >> agingtime;
            runMFQS();
            done = true;
            break;
         }{ 
         case 2:
            runRTS();
            done = true;

            // ====== Results ======

            // #ifdef DEBUG
            //    // List the processes' data about start and end times
            //    for (int i=0; i < attempted.size(); i++) {
            //       cout << attempted[i].pid << ":\n";
            //       cout << "   Start times: "; printIntVector(attempted[i].startTimes); cout << "\n";
            //       cout << "   End times: "; printIntVector(attempted[i].endTimes); cout << "\n";
            //    }
            
            // #endif

            // "Gannt chart"
            cout << gannt.str();

            // Average Waiting Time
            double avgWaitTime = totalWaitTime;
            avgWaitTime = avgWaitTime/totalProcessesScheduled;
            cout <<  "AWT: " << avgWaitTime << "\n";

            // Average Turnaround Time
            double avgTurn = totalTurnaroundTime;
            avgTurn = avgTurn/totalProcessesScheduled;
            cout << "ATT: " << avgTurn << "\n";

            // Total number of processes scheduled
            cout << "Process scheduled: " << processes.size() << "\n";
            cout << "Process finished: " << finished.size() << "\n";

            break;
         }{
         case 3:
            printf("Please enter the Time Quantum (2 or greater): ");
            cin >> tq;
            runHS();
            done = true;

            // ====== Results ======

            // "Gannt chart"
            cout << gannt.str();

            // Average Waiting Time
            double avgWaitTime = totalWaitTime;
            avgWaitTime = avgWaitTime/finished.size();
            cout <<  "AWT: " << avgWaitTime << "\n";

            // Average Turnaround Time
            double avgTurn;
            for (int i=0; i < finished.size(); i++) {
               avgTurn += finished[i].turnaround;
            }
            avgTurn = avgTurn/finished.size();
            cout << "ATT: " << avgTurn << "\n";

            // Total number of processes scheduled
            cout << "Process scheduled: " << processes.size() << "\n";
            break;
         }{
         default:
            break;
         }
      }
   }

   // // Results

   // // Average Waiting Time
   // double avgWaitTime = totalWaitTime;
   // avgWaitTime = avgWaitTime/totalProcessesScheduled;
   // cout <<  "AWT: " << avgWaitTime << "\n";

   // // Average Turnaround Time
   // double avgTurn = totalTurnaroundTime;
   // avgTurn = avgTurn/totalProcessesScheduled;
   // cout << "ATT: " << avgTurn << "\n";


   // // Total number of processes scheduled
   // cout << "NP: " << finished.size() << "\n";

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
    int clk = 0;
    int tqcount = 0;
    int currentqueue = 0;
    
    //make appropriate number of RR queues
    for(int i = 0; i < numQueues - 1; ++i){
        ProcessQueue pq;
        queues.push_back(pq);   
    }
    //make fcfs queue
    ProcessQueue fcfs;
    queues.push_back(fcfs);
    
    bool keeprunning = true;
    while(thereAreProcessesLeftToBeScheduled() || keeprunning){
        
        //put next ones on the queue
        putNextOnQueueByPrio(clk);
        
        //do age-up first in order to follow assignment guidelines
//        agingProcesses();
        
        //then do moving down queues, but run current process first.
        //run current process
        for(int i = 0; i < queues.size(); i++){
            //if the current queue we're looking at isn't empty, run the first process in that queue
            if(!queues[i].empty){
                
                //if the current queue is fcfs, run the whole process
                if(i == queues.size()){
                    int burst = queues[i].processes[0].timeLeft;
                    for(int j = 0; j < burst; j++){
                        queues[i].processes[0].execute;
                        queues[i].processes[0].lastrun = 0;
                        agingProcesses();
                        if(queues[i].processes[0].timeLeft == 0){
                            queues[i].pop_front();
                        }
                    }
                    
                //if the current queue is not fcfs, run for the designated tq for that queue
                }else{
                    //tq * 2^i will make the appropriate tq for that level.
                    int timeToRun = tq * pow(2, i);
                    for(int j = 0; j < timeToRun; j++){
                        queues[i].processes[0].execute;
                        queues[i].processes[0].lastrun = 0;
                        agingProcesses();
                        tqcount++;
                        if(queues[i].processes[0].timeLeft == 0){
                            //pop off the first process because it's done
                            queues[i].pop_front();
                            break;
                        }else if(tqcount == timeToRun){
                            //move down a queue if needed because it ran for the necessary time
                            tqcount = 0;
                            Process temp = queues[i].processes[0];
                            queues[i].pop_front();
                            queues[i+1].push_back(temp);
                        }
                    }
                }
                break;
            }
        }
        clk++;
    }
    
   return 0;
}

void agingProcesses(){
    //starts at 2 to ignore moving from 2nd queue up to 1st queue
    for(int i = 2; i < queues.size(); i++){
        for(int j = 0; j < queues[i].size(); j++){
            if(queues[i].processes[j].lastrun == agingtime){
                queues[i].processes[j].lastrun = 0;
                Process relocateProcess = queues[i].processes[j];
                queues[i].erase(queues[i].begin() + (j));
                queues[i].push_back(relocateProcess);
            }
        }
    }
}

int runRTS() {
   printf("Running RTS...\n");
   int clk = 0; // Clock counter
   Process lastrun;
   bool first = true;

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
               if (i==0) { // if it's the currently running process, add end time
                  queue[i].endTimes.push_back(clk + 1);
                  // gannt << clk+1 << " | ";
               }
               attempted.push_back(queue[i]);
               queue.erase(queue.begin() + i);
               if (i==0) { // if it was the currently running process, add next processes' start time
                  queue[i].startTimes.push_back(clk + 1);
                  // gannt << clk+1 << " <- Process " << queue[0].pid << " -> ";
               }
            }
         }

         if (queue[0].pid != lastrun.pid) {
            if (!first)
               gannt << clk << " | ";
            first = false;
            gannt << clk << " <- Process " << queue[0].pid << " -> ";
         }

         // "Execute" the first process in the queue
         queue[0].execute();
         #ifdef DEBUG
            cout << clk << ": Executing " << queue[0].pid << "\n";
         #endif

         lastrun = queue[0];

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
            queue[0].endTimes.push_back(clk + 1); // this is one of the ending times
            // gannt << clk+1 << " | ";
            attempted.push_back(queue[0]); // Not only attempted, but...
            finished.push_back(queue[0]);  //   finished as well
            queue.pop_front();
            if (queue.size() > 0) {
               queue[0].startTimes.push_back(clk + 1); //record start of next process
               // gannt << clk+1 << " <- Process " << queue[0].pid << " -> ";
            }
            
         }
      }

      clk++;
   }
   gannt << clk << "\n";
   printf("Done with RTS!\n");

   return 0;
}

int runHS() {
    printf("Running HS...\n");
    int clk = 0;
    int tqcount = 0;
    make_heap(queues.begin(), queues.end()); // put those queues into heap form
    Process lastrun;
    bool first = true;
    
   while (thereAreProcessesLeftToBeScheduled() || queues.size() > 0 || iOqueue.size() > 0) {
      //put new incoming processes in their respective queues
      putNextOnProcessQueues(clk);
     

      //only do the following if the queues have something in them (could not have any scheduled or all processes could be in IO)
      if (queues.size() > 0) {

         //check for starving processes, mod priority as necessary
         if (clk % 100 == 0) {
            std::sort_heap(queues.begin(), queues.end());
            deque<ProcessQueue> queuesOld (queues); // need to make a copy to work from, otherwise we'll loop infinitely
            queues.clear();
            std::make_heap(queues.begin(), queues.end());

            deque<Process> tempQueue;

            printQueues(queues);
            for (int i = 0; i < queuesOld.size(); i++) {
               for (int j=0; j < queuesOld[i].processes.size(); j++) {
                  //only change if hasn't been run in >=100 clock ticks
                  if (clk - queuesOld[i].processes[j].lastrun >= 100) {
                     //only change if under 50
                     if (queuesOld[i].processes[j].dynamicpriority < 50) {
                        #ifdef DEBUG
                           cout << clk << ": Starving process " << queuesOld[i].processes[j].pid << " found!\n";
                        #endif
                        //if increasing by 10 puts over 50, set to 50
                        if ((queuesOld[i].processes[j].dynamicpriority + 10 > 50)) {
                           queuesOld[i].processes[j].dynamicpriority = 50;
                        } else {
                           queuesOld[i].processes[j].dynamicpriority = queuesOld[i].processes[j].dynamicpriority + 10;
                        }
                        // addToProcessQueues(queuesOld[i].processes[j], clk); // add newly updated process to real queue
                     }
                  }
                  tempQueue.push_back(queuesOld[i].processes[j]);
                  queuesOld[i].processes.erase(queuesOld[i].processes.begin() + j); // remove process from fake queue
               }
            }

            for (int i = 0; i < tempQueue.size(); i++) {
               addToProcessQueues(tempQueue[i], clk);
            }
         }


         #ifdef DEBUG
            printQueues(queues);
         #endif

         if (queues.front().processes[0].pid != lastrun.pid) {
            if (!first)
               gannt << clk << " | ";
            first = false;
            gannt << clk << " <- Process " << queues.front().processes[0].pid << " -> ";
         }

         // look at first process in first queue
         // execute
         #ifdef DEBUG
            cout << clk << ": (" << queues.front().processes[0].pid << ") running (running for " << (queues.front().processes[0].timeInThisQuantum + 1) << ")\n";
         #endif
         queues.front().processes[0].execute();
         lastrun = queues.front().processes[0];

         // lastRun gets updated
         queues.front().processes[0].lastrun = clk;

         // increment that process' timeInThisQuantum
         queues.front().processes[0].timeInThisQuantum++;

         // add to wait time for all other processes
         for (int i = 0; i < queues.size(); i++) {
            for (int j=0; j < queues[i].processes.size(); j++) {
               totalWaitTime++;
            }
         }
         totalWaitTime--; // subtract one to account for the current process not waiting (it's running)

         //if process completed, "kick" it from the queue
         if(queues.front().processes[0].timeLeft == 0){
            #ifdef DEBUG
               cout << clk << ": (" << queues.front().processes[0].pid << ") Ended\n";
            #endif
            queues.front().processes[0].endTime = clk;
            // gannt << clk+1 << " | ";
            queues.front().processes[0].turnaround = clk - queues.front().processes[0].arrival;
            finished.push_back(queues.front().processes[0]);
            // remove current process from queue
            queues.front().processes.pop_front();

            // if that queue is now empty, remove it from queues
            if (queues.front().processes.empty()) {
               std::pop_heap(queues.begin(), queues.end()); 
               queues.pop_back();
            }

            // starting next process next tick
            // gannt << clk+1 << " <- Process " << queues.front().processes[0].pid << " -> ";
           
         }

         // if the process is still running at tq - 1, time to interrupt for IO
         if (queues.front().processes[0].timeInThisQuantum == tq - 1) {
            #ifdef DEBUG
               cout << clk << ": (" << queues.front().processes[0].pid << ") Hit time quantum IO\n";
            #endif
            
            //change priority of process based on clock ticks ran
            //if the change would put the dynamic priority lower than the original priority, set it to the original priority
            if((queues.front().processes[0].dynamicpriority - (tq-1)) < queues.front().processes[0].priority){
               queues.front().processes[0].dynamicpriority = queues.front().processes[0].priority;   
            }else{ //otherwise, just subtract the amount it's run this time around (tq - 1)
               queues.front().processes[0].dynamicpriority = (queues.front().processes[0].dynamicpriority - (tq-1));
            }

            // reset timeInThisQuantum for next time
            queues.front().processes[0].timeInThisQuantum = 0;

            //if this has I/O, run it
            if(queues.front().processes[0].io > 0) {
               #ifdef DEBUG
                  cout << clk << ": (" << queues.front().processes[0].pid << ") Going to IO queue\n";
               #endif
               iOqueue.push_back(queues.front().processes[0]);
               // gannt << clk+1 << " | ";
               queues.front().processes.pop_front(); // remove current process from queue
               // if that queue is now empty, remove it from queues
               if (queues.front().processes.empty()) {
                  std::pop_heap(queues.begin(), queues.end()); 
                  queues.pop_back();
               }
               // starting next process next tick
               // gannt << clk+1 << " <- Process " << queues.front().processes[0].pid << " -> ";
            }
         }
      }

      // loop through iOqueue, searching for processes that are done with I/O
      for(int i = 0; i < iOqueue.size(); i++){
         //if IO is now 0, put onto running queues
         if(iOqueue[i].ioLeft == 0){
            #ifdef DEBUG
               cout << clk << ": (" << iOqueue[i].pid << ") done with IO\n";
            #endif
            iOqueue[i].ioLeft = iOqueue[i].io; // reset ioLeft for next time
            Process p = iOqueue[i];
            iOqueue.erase(iOqueue.begin() + i);
            addToProcessQueues(p, clk);
         }
         //decrease IO time left
         iOqueue[i].ioLeft--;
         //increase priority according to IO time
         iOqueue[i].dynamicpriority++;
      }
        
      
        
      clk++;
   }
   gannt << clk << "\n";
   printf("Done with HWS!\n");
   return 0;
}

void putNextOnQueueByPrio(int clk) {
    deque<Process> newProcesses;
    
    for (int i=0; i<processes.size(); i++) {
        if (processes[i].arrival == clk && !processes[i].scheduled) { // get all processes coming in at this clock tick
            totalProcessesScheduled++;
            //if the newProcesses queue is empty, just add the next process to it
            if(newProcesses.size() == 0){
                newProcesses.push_front(processes[i]);   
            }else{
                bool wasScheduled = false;
                
                //if not empty, loop through the newProcesses queue to place is appropriately.
                for(int j = 0; j < newProcesses.size(); j++){
                    //first, if priorities are equal, sort by pid
                    if(processes[i].priority == newProcesses[j].priority){
                        if(processes[i].pid < newProcesses[j].pid){
                            newProcesses.insert(newProcesses.begin() + (j), processes[i]);
                        }else{
                            newProcesses.insert(newProcesses.begin() + (j + 1), processes[i]);   
                        }
                        wasScheduled = true;
                    //next, if the next process to schedule has a priority higher than the current one in the queue
                    //insert that at that spot
                    }else if(processes[i].priority > newProcesses[j].priority){
                        newProcesses.insert(newProcesses.begin() + (j), processes[i]);
                        wasScheduled = true;
                    }
                }
                //if the current process wasn't scheduled, put it at the end of the new set of processes
                if(!wasScheduled){
                    newProcesses.push_back(processes[i]);
                }
            }
        }
    }
    
    //put newProcesses into queues
    for(int i = 0; i < newProcesses(); i++){
        newProcesses[i].scheduled = true;
        newProcesses[i].startTimes.push_back(clk);
        queues.front().push_back(processes[i]);
        #ifdef DEBUG
            cout << clk << ": Putting process (" << processes[i].pid << ") in queue\n";
        #endif
    }
}

// Bring in all incoming processes on this clock tick and put them in the queue based on their deadline (for RTS)
void putNextOnQueueByDeadline(int clk) {
   for (int i=0; i<processes.size(); i++) {
      if (processes[i].arrival == clk && !processes[i].scheduled) { // get all processes coming in at this clock tick
         totalProcessesScheduled++;
         if (queue.size() == 0) { // if there's no processes in the queue, just add it
            #ifdef DEBUG
               cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of queue\n";
            #endif
            processes[i].scheduled = true;
            processes[i].startTimes.push_back(clk);
            queue.push_front(processes[i]);
            // gannt << clk << " <- Process " << queue[0].pid << " -> ";
         } else {
            bool inserted = false;
            for (int j=0; j<queue.size(); j++) {
               if (processes[i].deadline < queue[j].deadline) { // put it in before the first one with a later deadline
                  #ifdef DEBUG
                     cout << clk << ": Putting process (" << processes[i].pid << ") at position " << j << " in queue\n";
                  #endif
                  if (j == 0) { // if at the beginning of queue, we're replacing an already running process, so take care of end and start times
                     queue[0].endTimes.push_back(clk);
                     processes[i].startTimes.push_back(clk);
                     // gannt << clk << " | ";
                     // gannt << clk << " <- Process " << processes[i].pid << " -> ";
                  }
                  processes[i].scheduled = true;
                  queue.insert(queue.begin() + (j), processes[i]);
                  inserted = true;
                  break;
               }
            }
            if (!inserted) { // process[i] has a later deadline than any other process
               #ifdef DEBUG
                  cout << clk << ": Putting process (" << processes[i].pid << ") at end of queue\n";
               #endif
               processes[i].scheduled = true;
               queue.push_back(processes[i]);
            }
         }
      }
   }
}

// Bring in all incoming processes by clock tick and put them in appropriate priority queues (for HS)
void putNextOnProcessQueues(int clk) {
   std::sort_heap(queues.begin(), queues.end());
   for (int i=0; i < processes.size(); i++) { // for every process
      if (processes[i].arrival == clk && !processes[i].scheduled) {
         int j = 0;
         for (j=0; j < queues.size(); j++) { // find the process queue it should go in
            if (processes[i].priority == queues[j].priority) {
               processes[i].scheduled = true;
               queues[j].addProcess(processes[i]);
               #ifdef DEBUG
                  cout << clk << ": Putting process (" << processes[i].pid << ") on queue\n";
               #endif
               break;
            }
         }
         if (j == queues.size()) {
            // no priority queue made for this priority yet, make a new one
            ProcessQueue pq;
            processes[i].scheduled = true;
            pq.addProcess(processes[i]);
            queues.push_back(pq);
            std::push_heap(queues.begin(), queues.end());
             #ifdef DEBUG
               cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of new queue\n";
            #endif
         }
      }
   }
   std::make_heap(queues.begin(), queues.end());
}

// Take a process and put it in the appropriate priority queue (for HS)
void addToProcessQueues(Process p, int clk) {
   std::sort_heap(queues.begin(), queues.end());
   int j = 0;
   for (j=0; j < queues.size(); j++) { // find the process queue it should go in
      if (p.dynamicpriority == queues[j].priority) {
         p.scheduled = true;
         queues[j].addProcess(p);
         #ifdef DEBUG
            cout << clk << ": Putting process (" << p.pid << ") back on queue\n";
         #endif
         break;
      }
   }
   if (j == queues.size()) {
      // no priority queue made for this priority yet, make a new one
      ProcessQueue pq;
      p.scheduled = true;
      pq.addProcess(p);
      queues.push_back(pq);
      std::push_heap(queues.begin(), queues.end());
       #ifdef DEBUG
         cout << clk << ": Putting process (" << p.pid << ") back into new queue\n";
      #endif
   }
      
   std::make_heap(queues.begin(), queues.end());
}

bool thereAreProcessesLeftToBeScheduled() {
   bool result = false;
   for (int i=0; i<processes.size(); i++) {
      if (!processes[i].scheduled) {
         result = true;
         break;
      }
   }
   return result;
}

void printProcesses(deque<Process> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i].toString() << "\n";
   }
}

void printQueues(deque<ProcessQueue> v) {
   cout << "\t";
   for (int i = 0; i < v.size(); i++) {
      cout << "Queue "<< v[i].priority << ": " << v[i].toString() << "  ----  ";
   }
   cout << "\n";
}

void printIntVector(vector<int> v) {
   cout << v[0];
   for (int i = 1; i < v.size(); i++) {
      cout << ", " << v[i];
   }
}

// //HS nextOnQueue
// void putNextOnQueueByPriorityHS(int clk) {
//   for (int i=0; i<processes.size(); i++) {
//         if (processes[i].arrival == clk && !processes[i].scheduled) { // get all processes coming in at this clock tick
//          totalProcessesScheduled++;

//             //check legitimacy of the current priority, change dynamic priority if needed
//          if(processes[i].priority > 99){
//            processes[i].dynamicpriority = 99;
//         }else if(processes[i].priority < 0){
//            processes[i].dynamicpriority = 0;   
//         }else{
//            processes[i].dynamicpriority = processes[i].priority;   
//         }
//             if (queue.size() == 0) { // if there's no processes in the queue, just add it
//                #ifdef DEBUG
//                   cout << clk << ": Putting process (" << processes[i].pid << ") on beginning of queue\n";
//                #endif
//            queue.push_front(processes[i]);
//            processes[i].scheduled = true;
//         } else {
//                 //if there's processes in the queue, schedule accordingly
//            scheduleMeHS(clk, processes[i]);
//         }
//      }
//   }
// }


// void scheduleMeHS(int clk, Process p){
//     // just as what we had previously in the putNextOnQueueByPriorityHS() function, but since it can also be used for post-IO scheduling, i separated it
//     for (int j=0; j<queue.size(); j++) {
//         if (p.dynamicpriority > queue[j].dynamicpriority) { // put it in before the first one with a later deadline
//             #ifdef DEBUG
//             cout << clk << ": Putting process (" << p.pid << ") at position " << j << " in queue\n";
//             #endif
//             queue.insert(queue.begin() + (j), p);
//             j = queue.size();
//         }
//     }
// }


