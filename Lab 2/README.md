
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
	execute "make" in the program's directory to compile. Execute "./run" to run the program.
	

BUGS

