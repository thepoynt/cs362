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

int runMFQS();
int runRTS();
int runHS();


int scheduler;
int numQueues = 3;


int main () {
   bool done = false;
   while (!done) {
      printf("Please enter the number of the type of Scheduler you want to run:\n   1: MFQS\n   2: RTS\n   3: HS\n");
      std::cin >> scheduler;

      switch (scheduler) {
         {case 1:
            bool validNum = false;
            while (!validNum) {
               printf("How many queues should be used (between 1 and 5)?");
               std::cin >> numQueues;
               if(numQueues < 5 && numQueues > 0) {
                  validNum = true;
               }
            } 
            runMFQS();
            done = true;
            break;
         } 
         {case 2 :
            runRTS();
            done = true;
            break;
         }
         {case 3:
            runHS();
            done = true;
            break;
         } 
         {default:
            break;
         }
      }
   }


   return 0; 
}

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


