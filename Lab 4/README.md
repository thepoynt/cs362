	 _____            ___  ___            ____  _                _       _
	|_   _|_ _  __ _ / __|/ __|  ___     / ___|(_) _ __ __ _   _| | __ _| |_   ___  _ _
	  | | | `_|/ _` | |_ | |_| |/ __|   | (__  | || `  `  \ | | | |/ _` |  _| / _ \| `_|
	  | | | | | (_| |  _||  _| | (__     \___ \| || || || | |_| | | (_| | |__| (_) | |
	  |_| |_|  \__,_|_|  |_| |_|\___|   |_____/|_||_||_||_|\__,_|_|\__,_|\___|\___/|_|


README

	A C++ program using threads and semaphores to simulate a 4-way traffic intersection
	

AUTHORS
	
	Christian Poynter and John Frederick


INSTALLATION AND EXECUTION
	
	C++: To compile, execute "make" in the project directory. To run, execute "./run". Enter the total number of cars you wish to have simulated when prompted.
	Java for Windows: Make a new project with a single package within, with whatever names you choose, then copy the Intersection.java and Car.java classes into the project. Enter the total number of cars you wish to simulate, just as you did with the C++ code.
	

BUGS
	While the java version works, the c++ version does not. It runs correctly for a bit, but then starts behaving strangely and eventually hangs. We spent about 30 hours on the c++, but neither of us had any previous experience with pthreads, and we struggled with them.
