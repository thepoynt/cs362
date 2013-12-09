package phore;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;
import java.util.Scanner;
import java.util.concurrent.Semaphore;

public class Intersection implements Runnable{
	
	static ArrayList<Queue<Thread>> streets = new ArrayList<Queue<Thread>>();
	static ArrayList<Thread> allThreads = new ArrayList<Thread>();
	static int clk = 1;
	static int numCars = 0;
	static boolean empty = true;
	static long state = 1;
	static int carsRan = 0; 
	static int carsMade = 0;
	static int carsToRun = 0;

	static Random randomGen = new Random();
	
	static Semaphore carCanGo = new Semaphore(0);
	static Semaphore clkUp = new Semaphore(1);
	
	public static void main(String[] args) {
		
		for(int i = 0; i < 4; i++){
			Queue<Thread> newStreet = new LinkedList<Thread>();
			streets.add(newStreet);
		}
//		Scanner user_input = new Scanner( System.in );
//		String first_name;
//		System.out.print("Enter number of cars you want to run (under 100 for stability reasons): ");
//		first_name = user_input.next( );
//		user_input.close();
//		numCars = Integer.parseInt(first_name);
		numCars = 50;
		
		if(numCars > 100){
			numCars = 100;
		}

		while(carsRan  < numCars && clk < 30){
			if(carsMade < numCars){
				newCarsArrive();
				if(carsToRun == 0){
					clkUp.release();
				}
			}
			
			try {
				System.out.println("clockups");
				
				clkUp.acquire();
				clk++;
				carCanGo.release();
				System.out.println("clk: " + clk);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			//check for dead processes
			for(int i = 0; i < allThreads.size(); i++){
				if(!(allThreads.get(i).isAlive())){
					try {
						Thread t = allThreads.get(i);
						System.out.println("--Thread " + t.getId() + " Done--");
						allThreads.remove(i);
						t.join();
//						i = allThreads.size();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}else{
					System.out.println("~~Thread " + allThreads.get(i).getId() + " Not Done~~");
				}
			}
		}
	}
	
	private static int newCarsArrive() {
		int newcars = 0;
		int randInt = randomGen.nextInt(10);
		for(int i = 0; (i <  randInt) && (carsMade <= numCars) && (carsToRun < 100); i++){
			Thread t = new Thread();
			allThreads.add(t);
			newcars++;
			carsToRun++;
			carsMade++;
			System.out.println("made new car " + t.getId() + " | cars made: " + carsMade + " | carsToRun: " + carsToRun);
			t.start();
		}
		return newcars;
	}

	static double rndom() { 
		long A = 48271; 
		long M = 2147483647; 
		long Q = M/A; 
		long R = M%A; 
		 
		state = 1; 
		long t = A * (state % Q) - R * (state / Q); 
		 
		if (t > 0) 
			state = t;
		else 
			state = t + M;
		return ((double) state/M); 
	} 
	
	public void run() {
		System.out.println("here");
		//aka, this is the arrive function.
		Thread t = Thread.currentThread();
		if (empty) { 
			System.out.println("empty means go");
			empty = false; 
			try {
				drive(0, t);
			} catch (InterruptedException e) {
				e.printStackTrace();
			} 
		}else { 
			Random randomGen = new Random();
			int randomNum = randomGen.nextInt(100);
			int streetNum = randomNum;
			streets.get(streetNum).add(t);
			System.out.println(t.getId() + " | " + streetNum + " | " + randomNum);
			
			// wait for someone to signal that it is street[i]’s turn to go 
			boolean spin = true;
			while(spin){
				if(t==streets.get(streetNum).peek()){
					spin = false;
				}
			}

			try {
				carCanGo.acquire();
				drive(streetNum, t);
				clkUp.release();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		} 
	}
	
	void drive(int street, Thread t) throws InterruptedException{
		//critical section
		Thread.sleep(10);
		
		//clk++;
		
		//poll() removes from a Queue
		streets.get(street).poll();
		
		if (streets.get(0).isEmpty() && streets.get(1).isEmpty() && streets.get(2).isEmpty() && streets.get(3).isEmpty()) {
		      empty = true;
		}
		
		System.out.println("Car #" + t.getId() + " crossed street[" + street + "] at clk: " + clk);
		carsRan++;
		carsToRun--;
	}
}
