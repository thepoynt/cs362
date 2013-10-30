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

using std::cout;
using std::cin;
using std::vector;
using std::string;


int runMFQS();
int runRTS();
int runHS();
int readProcesses(string);
// int parseProcessFile(string);
void stringToProcess(string, vector<string>);
void printVector(vector<std::string>);

int scheduler;
int numQueues = 3;
vector<vector<int>> processes;


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
      if (!readProcesses("testfile")) {
         perror("Error reading file");
         exit(1);
      }
         
      
      // printVector(processes);
   }


   return 0; 
}

int readProcesses(string filename) {
   string line;
   std::ifstream myfile (filename);
   int i = 0;
   if (myfile.is_open()) {
      while ( getline (myfile,line) ) {
         if (i > 0 && line.find("-") == string::npos) { //skip first line - header
            stringToProcess(line);
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
void stringToProcess(string s) {
   std::istringstream iss(s);
   vector<string> tokens;
   copy(std::istream_iterator<string>(iss),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string>>(tokens));
   return 0;
}

// int parseProcessFile(string filename){
//    std::ifstream infile(filename);

//    if (infile.is_open()) {
//       std::string line;
//       while (std::getline(infile, line)){
//          std::istringstream iss(line);
//          int pid, bst, arr, pri, dline, io;
//          if (!(iss >> pid >> bst >> arr >> pri >> dline >> io) || (pid < 0 || bst < 0 || arr < 0 || pri < 0 || dline < 0 || io < 0)) { 
//             //do nothing, as this line won't read in appropriately
//          } else {
//             printf("%d | %d | %d | %d | %d | %d\n", pid, bst, arr, pri, dline, io);
//          }

//     }
//     return 1;
//    } else {
//    cout << "Unable to open file";
//    return 0;
//    }
// }

int runMFQS() {
   printf("Running MFQS...\n");
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

