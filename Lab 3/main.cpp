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

int scheduler;
int numQueues = 3;
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
               printf("How many queues should be used (between 1 and 5)?");
               cin >> numQueues;
               if(numQueues < 5 && numQueues > 0) {
                  validNum = true;
               }
            }
            runMFQS();
            done = true;
            break;
         }{ 
         case 2:
            runRTS();
            done = true;
            break;
         }{
         case 3:
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
   return 0;
}

int runHS() {
   printf("Running HS...\n");
   return 0;
}

void printProcesses(vector<Process> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i].pid << "\t" << v[i].burst << "\t" << v[i].arrival << "\t" << v[i].priority << "\t" << v[i].deadline << "\t" << v[i].io << "\n";
   }
}


