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
#include <sstream>
#include "car.h"

using std::cout;
using std::cin;
using std::deque;
using std::string;

// #define DEBUG 0;

void* arrival(void*);
void departure(int);
double rndom();
void drive();
int round_up(double);

int numCars = 0;
deque<deque<Car> > streets; //each queue for each street
bool empty = true; // intersection is empty (Boolean)
int clk = 0; // clock counter
int carsFinished = 0;
int carsScheduled = 0;
sem_t turn;
sem_t clockSem;


int main () {
   deque<Car> street0, street1, street2, street3;
   streets.push_back(street0);
   streets.push_back(street1);
   streets.push_back(street2);
   streets.push_back(street3);

   printf("Please enter the number of cars to run:  ");
   cin >> numCars;
   cout << "\nRunning with " << numCars << " cars\n";

   sem_init(&turn, 0, 1);
   sem_init(&clockSem, 0, 1);

   while (carsFinished < numCars) {

      // Generate a random number of cars, from 0 to 5
      double r = rndom();
      int numCarsThisRound;
      if (r == 1.0) {
         numCarsThisRound = 5; // so it won't go to 6
      } else {
         numCarsThisRound = r * 6.0;
      }
      int i = 0;

      if ((carsScheduled < numCars) && ((carsScheduled - carsFinished) < 110) && (i < numCarsThisRound)) {
         cout << clk << ": Generating " << numCarsThisRound << " cars. Already scheduled: " << carsScheduled << ", finished: " << carsFinished << "\n";
      }

      sem_wait(&clockSem);
      //       there are still cars      there aren't more than 110 cars waiting    we don't go over the number of cars for this clock
      while ((carsScheduled < numCars) && ((carsScheduled - carsFinished) < 110) && (i < numCarsThisRound)) {
         
         // assign each car to a random street
         double r = rndom();
         int num;
         if (r == 1.0) {
            num = 3; // so it won't go to 4
         } else {
            num = r * 4.0;
         }
         
         // make new car
         Car car;
         car.setQueue(num);
         std::ostringstream s;
         s << clk << "-" << i;
         std::string id(s.str());
         car.setId(id);

         cout << clk << ": Generating car " << car.id << " at street " << num << "\n";

         // make new thread for that car
         pthread_t pt;
         pthread_create(&pt, NULL, &arrival, (void *)&car);

         streets[num].push_back(car);

         carsScheduled++;
         i++;
      }

      clk++;
      sem_post(&clockSem);
   }

   return (0);
}

     
// car arrives at street[i]
void* arrival(void *v) {
   Car car = *(Car*)v;

   if (empty) {
      empty = false;
      cout << clk << ": Car " << car.id << " driving from street " << car.queue << " - it was empty\n";
      sem_wait(&clockSem);
         drive();
         departure(car.queue);
      sem_post(&clockSem);
   } else {
      // wait until I'm at the front of the queue
      while (true) {
         if (streets[car.queue].front().id.compare(car.id) == 0) {
            cout << "Car " << car.id << " At the Front!\n";
            break;
         }
      }
      sem_wait(&turn); // wait for someone to signal that it is street[i]’s turn to go 
         cout << clk << ": Car " << car.id << " driving from street " << car.queue << "\n";
         sem_wait(&clockSem);
            drive();
            departure(car.queue);
         sem_post(&clockSem);
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