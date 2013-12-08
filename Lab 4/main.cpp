#include <iostream>
#include <vector>
#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <cmath>
#include <time.h>
#include <typeinfo>
#include <pthread.h>
#include <semaphore.h>

using std::cout;
using std::cin;
using std::vector;
using std::deque;
using std::string;
// using std::pthread_create;

// #define DEBUG 0;

void arrival(int);
void departure(int);
double rndom();
void drive(int);
int round_up(double);

int numCars = 0;
deque<deque<pthread_t> > streets; //each queue for each street
bool empty = false; // intersection is empty (Boolean)
int clk = 0; // clock counter
int carsFinished = 0;
int carsScheduled = 0;
sem_t turn;



// Each car thread runs this
void* car(void *n) {
   // pull in the queue that I'm in (0-3)
   int *queue = (int *) n;

   // wait until I'm at the front of the queue
   while (true) {
      
   }

   // call arrival function
   cout << "Car arrived";
   pthread_exit(NULL);
}


int main () {
   printf("Please enter the number of cars to run:\n");
   cin >> numCars;

   sem_init(&turn, 0, 100);

   while (carsFinished < numCars) {

      // Generate a random number of cars, from 0 to 5
      int numCarsThisRound = round_up(rndom() * 4);
      int i = 0;
      while ((carsScheduled < numCars) && ((carsScheduled - carsFinished) < 110) && (i < numCarsThisRound)) {
         // assign each car to a random street
         int num = round_up(rndom() * 3);

         pthread_t pt;
         pthread_create(&pt, NULL, &car, &num);

         streets[num].push_back(pt);

         carsScheduled++;
         i++;
      }



      clk++;
   }

   return (0);
}

     
// car arrives at street[i]
void arrival(int i) {
   if (empty) {
      empty = false;
      drive(i);
   } else {
      sem_wait(&turn); // wait for someone to signal that it is street[i]’s turn to go 
      drive(i);
      sem_post(&turn);
   }
} //arrival(i)


// departure of car from intersection
void departure(int i) {
   // one car departs
   // pick the next intersection for next car to go

   // if no cars are in queues, empty = true 
   if (streets[0].empty() && streets[1].empty() && streets[2].empty() && streets[3].empty()) {
      empty = true;
   }

   carsFinished++;
}

void drive(int i) {
   clk += 2;
}

/* Returns a random number from 0 to 1 */
double rndom() { 
   const long A = 48271;
   const long M = 2147483647;
   const long Q = M/A;
   const long R = M%A;

   static long state = 1;
   long t = A * (state % Q) - R * (state / Q);
   if (t > 0)
      state = t;
   else
      state = t + M;
   return ((double) state/M);
}

int round_up(double n) {
   return (int)(floor(n + 0.5));
}