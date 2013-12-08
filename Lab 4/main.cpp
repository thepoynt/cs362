#include <iostream>
#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <cmath>
#include <time.h>
#include <typeinfo>
#include <pthread.h>
#include <semaphore.h>
#include "car.h"

using std::cout;
using std::cin;
using std::deque;
using std::string;
// using std::pthread_create;

// #define DEBUG 0;

void* arrival(void*);
void departure(int);
double rndom();
void drive();
int round_up(double);

int numCars = 0;
deque<deque<Car> > streets; //each queue for each street
bool empty = false; // intersection is empty (Boolean)
int clk = 0; // clock counter
int carsFinished = 0;
int carsScheduled = 0;
sem_t turn;


int main () {
   printf("Please enter the number of cars to run:\n");
   cin >> numCars;

   sem_init(&turn, 0, 1);

   while (carsFinished < numCars) {

      // Generate a random number of cars, from 0 to 5
      int numCarsThisRound = round_up(rndom() * 4);
      int i = 0;
      while ((carsScheduled < numCars) && ((carsScheduled - carsFinished) < 110) && (i < numCarsThisRound)) {
         // assign each car to a random street
         int num = round_up(rndom() * 3);

         Car car;
         car.setQueue(num);

         pthread_t pt;
         pthread_create(&pt, NULL, &arrival, (void *)&car);

         streets[num].push_back(car);

         carsScheduled++;
         i++;
      }

      clk++;
   }

   return (0);
}

     
// car arrives at street[i]
void* arrival(void *v) {
   Car car = *(Car*)v;

   if (empty) {
      empty = false;
      drive();
   } else {
      // wait until I'm at the front of the queue
      while (true) {
         if (&(streets[car.queue].front()) == &car) {
            break;
         }
      }
      sem_wait(&turn); // wait for someone to signal that it is street[i]’s turn to go 
      drive();
      departure(car.queue);
      sem_post(&turn);
   }
}


// departure of car from intersection
void departure(int i) {
   // one car departs
   streets[i].pop_front();

   // if no cars are in queues, empty = true 
   if (streets[0].empty() && streets[1].empty() && streets[2].empty() && streets[3].empty()) {
      empty = true;
   }

   carsFinished++;
}

void drive() {
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