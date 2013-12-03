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
sem_t turn;



// Each car thread runs this
void* car(void*) {
   cout << "Car arrived";
   return 0;
}


int main () {
   printf("Please enter the number of cars to run:\n");
   cin >> numCars;

   sem_init(&turn, 0, 10);

   while (carsFinished < numCars) {

      if (numCars < 3) {
         int num = round_up(rndom() * 3);

         pthread_t pt;
         pthread_create(&pt, NULL, &car, NULL);

         streets[num].push_back(pt);

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