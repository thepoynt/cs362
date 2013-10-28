     _ ____  ____
	| |  _ \/ ___|
	| | |_| |/
	| |  __/ \___
	|_|_|   \____|

README

	IPC is a simple socket program written in C++ to find all the prime numbers up to a user-defined maximum number.

	It uses Eratosthenes' Seive to find the primes. The first socket starts with 2 and elimates all multiples of 2 and passes the remaining numbers to the client, possibly on another system. The client starts with the first number and elimitates all of its multiples. This process goes back and forth until all the primes have been found.


AUTHORS
	
	Christian Poynter and John Frederick


INSTALLATION AND EXECUTION
	1) execute "make" in the program's directory to compile. 
	2) have 2 separate instances of putty running (or whichever client you choose to use)
	3) on one, execute "./server", then after being prompted, enter an integer to be used as the max value
	4) on the other, execute "./client" followed by the IP/hostname of the server. 
		(ex: "./client localhost")
	

BUGS

