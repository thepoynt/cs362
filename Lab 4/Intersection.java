package phore;

import java.io.Console;
import java.util.ArrayList;
<<<<<<< HEAD
import java.util.LinkedList;
import java.util.Queue;
import java.util.Scanner;
import java.util.concurrent.Semaphore;

public class Intersection {
	
	static ArrayList<Queue<Thread>> streets = new ArrayList<Queue<Thread>>();
	int carsFinished = 0;
	static int clk = 0;
	static int numCars = 0;
	static boolean empty = true;

	
	public static void main(String[] args) {
		Semaphore carCanGo = new Semaphore(1);
		
		for(int i = 0; i < 4; i++){
			Queue<Thread> newStreet = new LinkedList<Thread>();
			streets.add(newStreet);
		}
=======
import java.util.Random;
import java.util.Scanner;
import java.util.concurrent.Semaphore;

public class Intersection2 {
	
	static ArrayList<Thread> allThreads = new ArrayList<Thread>();
	static int clk = 1;
	public static int getClk() {
		return clk;
	}

	static int numCars = 0;
	static boolean empty = true;
	static int carsRan = 0; 
	static int carsMade = 0;
	static int carsToRun = 0;
	static int carId = 0;
	
	static Random randomGen = new Random();
	
	static volatile Semaphore carCanGo;
	static volatile Semaphore checkForFinished;
	
	public static void main(String[] args) {

		carCanGo = new Semaphore(1);
		checkForFinished = new Semaphore(0);

		
>>>>>>> aa2ef6daddb3959bf0038a77694eb618c624eb9d
		Scanner user_input = new Scanner( System.in );
		String first_name;
		System.out.print("Enter number of cars you want to run (under 100 for stability reasons): ");
		first_name = user_input.next( );
		user_input.close();
		numCars = Integer.parseInt(first_name);
<<<<<<< HEAD
		
		
		
		if(numCars > 100){
			numCars = 100;
		}
		for(int i = 0; i < numCars; i++){
			Thread t = new Thread(new Car(carCanGo));
			int streetNum = (int) (rndom() * 4);
			streets.get(streetNum).add(t);
			System.out.println(t.getId() + " | " + streetNum);
		}
		
	}
	
	static double rndom() { 
		long A = 48271; 
		long M = 2147483647; 
		long Q = M/A; 
		long R = M%A; 
		 
		long theState = 1; 
		long t = A * (theState % Q) - R * (theState / Q); 
		 
		if (t > 0) 
			theState = t;
		else 
			theState = t + M;
		return ((double) theState/M); 
	} 

	private static class Car implements Runnable{

		private Semaphore s;
		
		public Car(Semaphore s1) {
			s = s1;
		}
		
		public void run() {
			Thread t = Thread.currentThread();
			while(true){
				try{
				//peek() looks at the first element of a Queue w/o removing it
				if(t==streets.get(0).peek()){
					drive(0, t);
					break;
				}else if(t==streets.get(1).peek()){
					drive(1, t);
					break;
				}else if(t==streets.get(2).peek()){
					drive(2, t);
					break;
				}else if(t==streets.get(3).peek()){
					drive(3, t);
					break;
				}
				
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		void drive(int street, Thread t) throws InterruptedException{
			s.acquire();
			
			//critical section
			
			clk++;
			
			//poll() removes from a Queue
			streets.get(street).poll();
			
			if (streets.get(0).isEmpty() && streets.get(1).isEmpty() && streets.get(2).isEmpty() && streets.get(3).isEmpty()) {
			      empty = true;
			}
			
			System.out.println("Car #" + t.getId() + " crossed street[" + street + "] at clk: " + clk);
			
			s.release();
		}
	}
=======
		
		long startTime = System.currentTimeMillis();

		while(carsRan<numCars){
			if(carsMade < numCars){
				newCarsArrive();
			}
			
			try {
				checkForFinished.acquire();
			} catch (InterruptedException e1) {
				e1.printStackTrace();
			}
			for(int i = 0; i < allThreads.size(); i++){
				if(!(allThreads.get(i).isAlive())){
					try {
						Thread t = allThreads.get(i);
						allThreads.remove(i);
						t.join();
						i = allThreads.size();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
			carCanGo.release();
		}
		
		long endTime   = System.currentTimeMillis();
		long totalTime = endTime - startTime;
		System.out.println("Ran in " + totalTime + " ms");
	}
	
	private static void newCarsArrive() {
		int randInt = randomGen.nextInt(5) + 1;
		for(int i = 0; (i < randInt) && (carsMade <= numCars) && (carsToRun < 100); i++){
			Thread t = new Thread(new Car(carCanGo, checkForFinished, carId));
			carId++;
			allThreads.add(t);
			carsToRun++;
			carsMade++;
			t.start();
		}
	}
	
	public static void changeStats(int a, int b, int c){
		carsRan = carsRan + a;
		carsToRun = carsToRun + b;
		clk = clk + c;
	}
	
	//was going to use this code, but it doesn't work in Java, so i used the built-in random functions.
//	static double rndom() { 
//		long A = 48271; 
//		long M = 2147483647; 
//		long Q = M/A; 
//		long R = M%A; 
//		 
//		state = 1; 
//		long t = A * (state % Q) - R * (state / Q); 
//		 
//		if (t > 0) 
//			state = t;
//		else 
//			state = t + M;
//		return ((double) state/M); 
//	} 

>>>>>>> aa2ef6daddb3959bf0038a77694eb618c624eb9d
}
