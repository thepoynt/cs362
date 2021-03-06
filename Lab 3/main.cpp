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
#include <typeinfo>

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
void putNextOnQueueByPriorityHS(int);
void scheduleMeHS(int, Process);    
void putNextOnProcessQueues(int);
void addToProcessQueues(Process, int);
void printProcesses();
void printQueues(deque<ProcessQueue>);
void printProcessDeque(deque<Process>);
void agingProcesses(int);
void putNextOnQueueByPrio(int);
int updateCurrentQueue();


int scheduler;
string file;
int numQueues = 3;
int tq;
int agingtime;
deque<Process> processes;     // for all processes read in from file
deque<Process> queue;         // process queue for RTS
deque<Process> iOqueue;       // queue for processes doing IO for HWS
deque<Process> finished;      // for all processes that sucessfully finished
deque<Process> attempted;     // for all processes, after they've been processed (whether they finished or not) so we have data about their execution
deque<ProcessQueue> queues;   // A queue of queues for HWS
double totalWaitTime = 0;
double totalTurnaroundTime = 0;
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

   printf("Please enter the name of the input file:\n");
   cin >> file;

   // Read in processes from file
   if (!readProcesses(file)) {
      perror("Error reading file");
      exit(1);
   }

   // sort processes by arrival time for easy access later
   std::sort(processes.begin(), processes.end(), sortProcesses);


   #ifdef DEBUG
      // printProcesses();
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

            time (&end); // end timer
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

            cout << totalWaitTime << "   " << totalTurnaroundTime << "\n";

            // Total number of processes scheduled
            cout << "Processes scheduled: " << totalProcessesScheduled << "\n";

            // Time elapsed
            double duration = difftime (end,start);
            cout << "Time elapsed: " << duration << "\n";

            break;
         }{ 
         case 2:
            time (&start); // start timer

            runRTS();

            time (&end); // end timer
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

            // Time elapsed
            double duration = difftime (end,start);
            cout << "Time elapsed: " << duration << "\n";

            break;
         }{
         case 3:
            printf("Please enter the Time Quantum (2 or greater): ");
            cin >> tq;
            time (&start); // start timer

            runHS();

            time (&end); // end timer
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
            cout << "Processes scheduled: " << totalProcessesScheduled << " " << finished.size() << "\n";

            // Time elapsed
            double duration = difftime (end,start);
            cout << "Time elapsed: " << duration << "\n";

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

    Process lastrun;
    lastrun.setVars("0  0  0  0  0  0");
    
    while ((!processes.empty()) || (numInQueues > 0)) {
        
        //put next ones on the queue
        putNextOnQueueByPrio(clk);
        
         //if the current queue we're looking at isn't empty (which it shouldn't be, but just in case), run the first process in that queue
         if (!queues[currentqueue].processes.empty()) {
            #ifdef DEBUG
               cout << clk << " - current queue " << currentqueue << ":  ";
               printQueues(queues);
            #endif

            if (queues[currentqueue].processes[0].pid != lastrun.pid) {
               if (lastrun.endTime == 0) // process is still not finished, but need to print out end of current "burst"
                  gannt << clk << " | ";
               gannt << clk << " <- Process " << queues[currentqueue].processes[0].pid << " -> ";
            }

            queues[currentqueue].processes[0].execute();
            lastrun = queues[currentqueue].processes[0];
            totalWaitTime += (numInQueues - 1); // add wating time for all other processes
            queues[currentqueue].processes[0].lastrun = clk;
            queues[currentqueue].processes[0].timeInThisQuantum++;


            // if it has now finished, remove it and update the current queue
            if(queues[currentqueue].processes[0].timeLeft == 0){
               gannt << clk+1 << " | ";
               numInQueues--;
               finished.push_back(queues[currentqueue].processes[0]);
               totalTurnaroundTime += (clk - queues[currentqueue].processes[0].arrival + 1);
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

        clk++;
    }
    gannt << "\n";
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

   while (processes.size() > 0 || queue.size() > 0) {

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
               attempted.push_back(queue[i]);
               queue.erase(queue.begin() + i);
               numInQueues--;
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
            totalTurnaroundTime += (clk - queue[0].arrival);
            gannt << clk+1 << " | ";
            attempted.push_back(queue[0]); // Not only attempted, but...
            finished.push_back(queue[0]);  //   finished as well
            queue.pop_front();
            numInQueues--;
            
         }

         // increment the waiting time for all other processes in queue
         totalWaitTime += (numInQueues - 1);
      }

      clk++;
   }
   gannt << "\n";
   printf("Done with RTS!\n");

   return 0;
}

int runHS() {
    printf("Running HWS...\n");
    int clk = 0;
    int tqcount = 0;
    make_heap(queues.begin(), queues.end()); // put those queues into heap form
    Process lastRun;
    lastRun.setVars("0  0  0  0  0  0");
    bool first = true;
    
   while (!processes.empty() || !queues.empty() || !iOqueue.empty()) {

      //put new incoming processes in their respective queues
      if (processes.size() > 0) {
        putNextOnProcessQueues(clk); 
      }
      
      //only do the following if the queues have something in them (could not have any scheduled or all processes could be in IO)
      if (queues.size() > 0) {


         //check for starving processes, mod priority as necessary
         if (clk % 100 == 0) {
            std::sort_heap(queues.begin(), queues.end());
            deque<ProcessQueue> queuesOld (queues); // need to make a copy to work from, otherwise we'll loop infinitely
            queues.clear();
            std::make_heap(queues.begin(), queues.end());

            deque<Process> tempQueue;

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
            // printQueues(queues);
         #endif

         if (queues.front().processes[0].pid != lastRun.pid) {
            if (lastRun.endTime == 0) // process is still not finished, but need to print out end of current "burst"
               gannt << clk << " | ";
            gannt << clk << " <- Process " << queues.front().processes[0].pid << " -> ";
         }

         // look at first process in first queue
         // execute
         #ifdef DEBUG
            cout << clk << ": (" << queues.front().processes[0].pid << ") running (running for " << (queues.front().processes[0].timeInThisQuantum + 1) << ") in queue " << queues.front().priority << "\n";
         #endif
         
         queues.front().processes[0].execute();
         lastRun = queues.front().processes[0];

         // process' lastrun gets updated
         queues.front().processes[0].lastrun = clk;

         // increment that process' timeInThisQuantum
         queues.front().processes[0].timeInThisQuantum++;

         //if process completed, "kick" it from the queue
         if(queues.front().processes[0].timeLeft == 0){
            #ifdef DEBUG
               cout << clk << ": (" << queues.front().processes[0].pid << ") Ended\n";
            #endif

            gannt << clk+1 << " | ";
            queues.front().processes[0].endTime = clk;
            queues.front().processes[0].turnaround = clk - queues.front().processes[0].arrival;
            totalTurnaroundTime += (clk - queues.front().processes[0].arrival);
            finished.push_back(queues.front().processes[0]);
            // remove current process from queue
            queues.front().processes.pop_front();
            numInQueues--;

            // if that queue is now empty, remove it from queues
            if (queues.front().processes.empty()) {
               std::pop_heap(queues.begin(), queues.end()); 
               queues.pop_back();
            }
         }

         if (!queues.empty()) { // could have just emptied the queues

            // if the process is still running at tq - 1, time to interrupt for IO
            if (queues.front().processes[0].timeInThisQuantum == (tq - 1)) {
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
                  queues.front().processes.pop_front(); // remove current process from queue
               } else { // if it doesnt do IO, just put it in new queue
                  addToProcessQueues(queues.front().processes[0], clk);
                  queues.front().processes.pop_front();
               }

               // if that queue is now empty, remove it from queues
               if (queues.front().processes.empty()) {
                  std::pop_heap(queues.begin(), queues.end()); 
                  queues.pop_back();
               }
            }
         }
      }

      // up the waiting time 
      if (queues.size() > 0 || iOqueue.size() > 0) {
         totalWaitTime += (numInQueues - 1);
      }

      
      // cout << clk << ": " << processes.size() << " : " << queues.size() << " : " << iOqueue.size() << "\n";

      // loop through iOqueue, searching for processes that are done with I/O
      deque<Process> tempIO;
      for(int i = 0; i < iOqueue.size(); i++){
         //if IO is now 0, put onto running queues
         if(iOqueue[i].ioLeft == 0){
            #ifdef DEBUG
               cout << clk << ": (" << iOqueue[i].pid << ") done with IO\n";
            #endif
            iOqueue[i].ioLeft = iOqueue[i].io; // reset ioLeft for next time
            Process p = iOqueue[i];
            addToProcessQueues(p, clk);
         }  else {
            //decrease IO time left
            iOqueue[i].ioLeft--;
            //increase priority according to IO time
            iOqueue[i].dynamicpriority++;

            tempIO.push_back(iOqueue[i]);
         }
         
      }
      iOqueue = tempIO;
      tempIO.clear();
        
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

      }
      processes.pop_front();
   }
    
   //put newProcesses into queues
   for(int i = 0; i < newProcesses.size(); i++){
      newProcesses[i].scheduled = true;
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
      numInQueues++;
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
   while ((!processes.empty()) && (processes[0].arrival == clk)) {
      numInQueues++;
      Process p = processes[0];

      deque<ProcessQueue>::iterator j;
      for (j = queues.begin(); j != queues.end(); j++) { // find the process queue it should go in
         if (p.priority == j->priority) {
            p.scheduled = true;
            j->addProcess(p);
            #ifdef DEBUG
               cout << clk << ": Putting process (" << p.pid << ") on queue\n";
            #endif
            break;
         }
      }
      if (j == queues.end()) {
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
      totalProcessesScheduled++;
      processes.pop_front();
   }
}

// Take a process and put it in the appropriate priority queue (for HS)
void addToProcessQueues(Process p, int clk) {
   deque<ProcessQueue>::iterator j;
   for (j = queues.begin(); j != queues.end(); j++) { // find the process queue it should go in
      if (p.dynamicpriority == j->priority) {
         p.scheduled = true;
         j->addProcess(p);
         // #ifdef DEBUG
         //    cout << clk << ": Putting process (" << p.pid << ") back on queue\n";
         // #endif
         break;
      }
   }
   if (j == queues.end()) {
      // no priority queue made for this priority yet, make a new one
      ProcessQueue pq;
      p.scheduled = true;
      pq.addProcess(p);
      queues.push_back(pq);
      std::push_heap(queues.begin(), queues.end());
      //  #ifdef DEBUG
      //    cout << clk << ": Putting process (" << p.pid << ") back into new queue\n";
      // #endif
   }
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

void printProcessDeque(deque<Process> v) {
   if (v.size() > 0) {
      cout << v[0].pid;
      for (int i = 1; i < v.size(); i++) {
         cout << ", " << v[i].pid;
      }
   } else {
      cout << "empty";
   }
   cout << "\n";
}

