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
#include <time.h>

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
void printProcesses();
void printQueues(deque<ProcessQueue>);
void printIntVector(vector<int>);
void agingProcesses(int);
void putNextOnQueueByPrio(int);
int updateCurrentQueue();


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
int numInQueues = 0;
time_t start,end;

// custom comparator for sorting processes by arrival time
bool sortProcesses (const Process &a, const Process &b) { 
   return a.arrival < b.arrival;
}

int main () {
   bool done = false;

   // Read in processes from file
   if (!readProcesses("MFQS starving")) {
      perror("Error reading file");
      exit(1);
   }

   // sort processes by arrival time
   std::sort(processes.begin(), processes.end(), sortProcesses);


   #ifdef DEBUG
      printProcesses();
   #endif
   cout << "Number of processes read in: " << processes.size() << "\n";

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
            time (&start); // start timer

            runMFQS();

            time (&end);
            done = true;

            // ====== Results ======

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
            cout << "Processes scheduled: " << finished.size() << "\n";

            // Time elapsed
            double duration = difftime (end,start);
            std::cout << "\nTime elapsed: " << duration;

            break;
         }{ 
         case 2:
            runRTS();
            done = true;

            // ====== Results ======

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
            cout << "Processes scheduled: " << attempted.size() << "\n";
            cout << "Processes finished: " << finished.size() << "\n";

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
            cout << "Processes scheduled: " << processes.size() << "\n";
            break;
         }{
         default:
            break;
         }
      }
   }

   exit(0);
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
    int currentqueue = 0;
    
    //make appropriate number of RR queues
    int g = 0;
    for(g = 0; g < numQueues - 1; g++){
        ProcessQueue pq;
        pq.priority = g;
        queues.push_back(pq);   
    }
    //make fcfs queue
    ProcessQueue fcfs;
    fcfs.priority = g;
    queues.push_back(fcfs);
    
    while ((!processes.empty()) || (numInQueues > 0)) {
        
        //put next ones on the queue
        putNextOnQueueByPrio(clk);
        
        //do age-up first in order to follow assignment guidelines
//        agingProcesses();
        
        //then do moving down queues, but run current process first.
        //run current process
        // for(int i = 0; i < queues.size(); i++){
         //if the current queue we're looking at isn't empty (which it shouldn't be, but just in case), run the first process in that queue
         if (!queues[currentqueue].processes.empty()) {
            #ifdef DEBUG
               cout << clk << " - current queue " << currentqueue << ":  ";
               printQueues(queues);
            #endif

            queues[currentqueue].processes[0].execute();
            totalWaitTime += (numInQueues - 1); // add wating time for all other processes
            queues[currentqueue].processes[0].lastrun = clk;
            queues[currentqueue].processes[0].timeInThisQuantum++;


            // if it has now finished, remove it and update the current queue
            if(queues[currentqueue].processes[0].timeLeft == 0){
               numInQueues--;
               finished.push_back(queues[currentqueue].processes[0]);
               totalTurnaroundTime += (clk - queues[currentqueue].processes[0].arrival);
               //pop off the first process because it's done
               queues[currentqueue].processes.pop_front();
               // update the current queue - a new process could have come into a higher queue while this process was running, or we might go down a level
               currentqueue = updateCurrentQueue();
            }

            // Deal with time quantum stuff
             
            // if the current queue is fcfs
            if (currentqueue == queues.size() - 1) {
               // don't do anything, just go on to the next clock tick. This process will finish before we move on to the next (FCFS)
                 
            //if the current queue is not fcfs, cut off at the tq for that queue
            } else {
               //tq * 2^i will make the appropriate tq for that level.
               int timeToRun = tq * pow(2, currentqueue);
               if (queues[currentqueue].processes[0].timeInThisQuantum == timeToRun) {
                  queues[currentqueue].processes[0].timeInThisQuantum = 0;
                  queues[currentqueue+1].processes.push_back(queues[currentqueue].processes[0]);
                  queues[currentqueue].processes.pop_front();
                  currentqueue = updateCurrentQueue();
               }
            }

            // age any processes that need it
            agingProcesses(clk);
         }
        // }
        clk++;
    }
   printf("Done with MFQS!\n");
   return 0;
}

void agingProcesses(int clk){
   //starts at 2 to ignore moving from 2nd queue up to 1st queue
   for(int i = 2; i < queues.size(); i++){
      for(int j = 0; j < queues[i].processes.size(); j++){
         if((clk - queues[i].processes[j].lastrun) >= agingtime){
            #ifdef DEBUG
               cout << "starving process (" << queues[i].processes[j].pid << ") found - aging up\n";
            #endif
            queues[(i-1)].processes.push_back(queues[i].processes[j]);
            queues[i].processes.erase(queues[i].processes.begin() + (j));
         }
      }
   }
}

int updateCurrentQueue() {
   // return first queue that has processes in it
   for (int i = 0; i < queues.size(); i++) {
      if (queues[i].processes.size() > 0) {
         return i;
      }
   }
   // if there are no processes, just return 0 so we're at the first queue when the next process comes in
   return 0;
}

int runRTS() {
   printf("Running RTS...\n");
   int clk = 0; // Clock counter
   Process lastrun;

   while (thereAreProcessesLeftToBeScheduled() || queue.size() > 0) {

      // cout << "\r" << finished.size() << " processes completed." << std::flush;

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
               attempted.push_back(queue[i]);
               queue.erase(queue.begin() + i);
            }
         }

         if (queue[0].pid != lastrun.pid) {
            gannt << clk << " <- Process " << queue[0].pid << " -> ";
         }

         // "Execute" the first process in the queue
         queue[0].execute();
         #ifdef DEBUG
            cout << clk << ": Executing " << queue[0].pid << "\n";
         #endif

         lastrun = queue[0];

         // if timeLeft is 0 for current process, remove it
         if (queue[0].timeLeft == 0) {
            #ifdef DEBUG
               cout << clk << ": Popping process (" << queue[0].pid << ") from queue\n";
            #endif
            queue[0].endTime = clk;
            queue[0].turnaround = clk - queue[0].arrival;
            gannt << clk+1 << " | ";
            attempted.push_back(queue[0]); // Not only attempted, but...
            finished.push_back(queue[0]);  //   finished as well
            queue.pop_front();
            
         }

         // increment the waiting time for all other processes in queue
         for (int i = 1; i<queue.size(); i++) {
            totalWaitTime++;
            totalTurnaroundTime++; // this gets incremented for all other processes, as well as the current process (all processes on queue are having their turnaround time incremented, while only all non-running processes get wait time incremented)
         }
         totalTurnaroundTime++; // Increment for current process. See comment above ^^
      }

      clk++;
   }
   gannt << "\n";
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
       cout << "\r" << finished.size() << " processes completed.       " << std::flush;

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
            if (lastrun.endTime == 0) // process is still not finished, but need to print out end of current "burst"
               gannt << clk << " | ";
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
            gannt << clk+1 << " | ";
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
            } else { // if it doesnt do IO, just put it in new queue
               addToProcessQueues(queues.front().processes[0], clk);
               queues.front().processes.pop_front();
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
   gannt << "\n";
   printf("Done with HWS!\n");
   return 0;
}

void putNextOnQueueByPrio(int clk) {
   deque<Process> newProcesses;
    
   while (processes[0].arrival == clk) {
      numInQueues++;
      Process p = processes[0];
      totalProcessesScheduled++;
      //if the newProcesses queue is empty, just add the next process to it
      if(newProcesses.size() == 0){
         newProcesses.push_front(p);   
      }else{
         int i = 0;
         for(i = 0; i < newProcesses.size(); i++) {
            if (p < newProcesses[i]) { // put new process in newProcesses based on priority, then pid.
               break;
            }
         }
         newProcesses.insert(newProcesses.begin() + i, p);

          // bool wasScheduled = false;
          
          // //if not empty, loop through the newProcesses queue to place is appropriately.
          // for(int j = 0; j < newProcesses.size(); j++){
          //     //first, if priorities are equal, sort by pid
          //     if(p.priority == newProcesses[j].priority){
          //         if(p.pid < newProcesses[j].pid){
          //             newProcesses.insert(newProcesses.begin() + (j), p);
          //         }else{
          //             newProcesses.insert(newProcesses.begin() + (j + 1), p);   
          //         }
          //         wasScheduled = true;
          //     //next, if the next process to schedule has a priority higher than the current one in the queue
          //     //insert that at that spot
          //     }else if(p.priority > newProcesses[j].priority){
          //         newProcesses.insert(newProcesses.begin() + (j), p);
          //         wasScheduled = true;
          //     }
          // }
          // //if the current process wasn't scheduled, put it at the end of the new set of processes
          // if(!wasScheduled){
          //     newProcesses.push_back(p);
          // }
      }
      processes.pop_front();
   }
    
   //put newProcesses into queues
   for(int i = 0; i < newProcesses.size(); i++){
      newProcesses[i].scheduled = true;
      // newProcesses[i].startTimes.push_back(clk);
      queues.front().addProcess(newProcesses[i]);
      #ifdef DEBUG
         cout << clk << ": Putting process (" << newProcesses[i].pid << ") in queue\n";
      #endif
   }
}

// Bring in all incoming processes on this clock tick and put them in the queue based on their deadline (for RTS)
void putNextOnQueueByDeadline(int clk) {
   while (processes[0].arrival == clk) {
      Process p = processes[0];
      totalProcessesScheduled++;
      if (queue.size() == 0) { // if there's no processes in the queue, just add it
         #ifdef DEBUG
            cout << clk << ": Putting process (" << p.pid << ") on beginning of queue\n";
         #endif
         p.scheduled = true;
         queue.push_front(p);
      } else {
         bool inserted = false;
         for (int j=0; j<queue.size(); j++) {
            if (p.deadline < queue[j].deadline) { // put it in before the first one with a later deadline
               #ifdef DEBUG
                  cout << clk << ": Putting process (" << p.pid << ") at position " << j << " in queue\n";
               #endif
               p.scheduled = true;
               queue.insert(queue.begin() + (j), p);
               inserted = true;
               break;
            }
         }
         if (!inserted) { // process[i] has a later deadline than any other process
            #ifdef DEBUG
               cout << clk << ": Putting process (" << p.pid << ") at end of queue\n";
            #endif
            p.scheduled = true;
            queue.push_back(p);
         }
      }
      processes.pop_front();
   }
}

// Bring in all incoming processes by clock tick and put them in appropriate priority queues (for HWS)
void putNextOnProcessQueues(int clk) {
   std::sort_heap(queues.begin(), queues.end());
   while (processes[0].arrival == clk) {
      Process p = processes[0];
      int j = 0;
      for (j=0; j < queues.size(); j++) { // find the process queue it should go in
         if (p.priority == queues[j].priority) {
            p.scheduled = true;
            queues[j].addProcess(p);
            #ifdef DEBUG
               cout << clk << ": Putting process (" << p.pid << ") on queue\n";
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
            cout << clk << ": Putting process (" << p.pid << ") on beginning of new queue\n";
         #endif
      }
      processes.pop_front();
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

void printProcesses() {
   for (int i = 0; i < processes.size(); i++) {
      cout << processes[i].toString() << "\n";
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


