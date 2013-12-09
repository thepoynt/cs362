import java.util.Random;
import java.util.concurrent.Semaphore;

public class Car implements Runnable {

	Semaphore carCanGo;
	Semaphore checkForFinished;
	int streetNum;
	int carID;
	
	public Car(Semaphore s1, Semaphore s2, int i) {
		carCanGo = s1;
		checkForFinished = s2;
		Random randomGen = new Random();
		streetNum = randomGen.nextInt(4);
		carID = i;
	}
	
	public void run() {
		try {
			carCanGo.acquire();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		Intersection.changeStats(1, -1, 1);
		System.out.println("Thread #" + carID + " finished at clk: " + Intersection.getClk() + " from Street #" + streetNum);
		checkForFinished.release();
	}
}
