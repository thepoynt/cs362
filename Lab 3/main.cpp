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
#include <fstream>
<<<<<<< HEAD
using std::cout;
using std::cin;
using std::vector;
using std::string;
=======
>>>>>>> 9ac466ab96fc691a4d2b0a33f83eb919aa108cde

int runMFQS();
int runRTS();
int runHS();
int readProcesses();
int stringToProcess();
void printVector(vector<std::string>);

int scheduler;
int numQueues = 3;
vector<string> processes;


int main () {
   bool done = false;
   while (!done) {
      printf("Please enter the number of the type of Scheduler you want to run:\n   1: MFQS\n   2: RTS\n   3: HS\n");
      cin >> scheduler;

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
      if (!readProcesses()) {
         perror("Error reading file");
         exit(1);
      }
         
      
      printVector(processes);
   }


   return 0; 
}

<<<<<<< HEAD
int readProcesses() {
   string line;
   std::ifstream myfile ("testfile");
   int i = 0;
   if (myfile.is_open()) {
      while ( getline (myfile,line) ) {
         if (i > 0 && line.find("-") == string::npos) { //skip first line - header
            processes.push_back(line);
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
int stringToProcess() {
   // convert line of text to a Process object
   return 0;
=======
void parseProcessFile(String filename){
    std::ifstream infile(filename);
    
    std::string line;
    while (std::getline(infile, line)){
        std::istringstream iss(line);
        int pid, bst, arr, pri, dline, io;
        if (!(infile >> pid >> bst >> arr >> pri >> dline >> io) || (pid < 0 || bst < 0 || arr < 0 || pri < 0 || dline < 0 || io < 0)) { 
            //do nothing, as this line won't read in appropriately
        } // error
        printf("%d | %d | %d | %d | %d \n", pid, bst, arr, pri, dline, io);
        // process pair (a,b)
    }
>>>>>>> 9ac466ab96fc691a4d2b0a33f83eb919aa108cde
}

int runMFQS() {
   printf("Running MFQS...\n");
    parseProcessFile("testfile");
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

void printVector(vector<string> v) {
   for (int i = 0; i < v.size(); i++) {
      cout << v[i] << "\n";
   }
}

