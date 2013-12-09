import java.util.ArrayList;
import java.util.Random;
import java.util.Scanner;
import java.util.concurrent.Semaphore;

public class Intersection {
	
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

		
		Scanner user_input = new Scanner( System.in );
		String first_name;
		System.out.print("Enter number of cars you want to run (under 100 for stability reasons): ");
		first_name = user_input.next( );
		user_input.close();
		numCars = Integer.parseInt(first_name);
		
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
						i = allThreads.size() + 1;
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
	
	//found that this function doesn't work for Java, so the built-in Random was used.
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

}
