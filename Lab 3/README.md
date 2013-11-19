	 ____                                    ____      _              _       _
	|  _ \ _ _  ___   ___  ___  ___  ___    / ___| ___| |__   ___  __| |_   _| | ___ _ _
	| |_| | `_|/ _ \ / __|/ _ \/ __|/ __|  | (__  / __| `_  \/ _ \/ _  | | | | |/ _ \ `_|
	|  __/| | | (_) | |__|  __/\__ \\__ \   \___ \ |__| | | |  __/ (_) | |_| | |  __/ |
	|_|   |_|  \___/ \___|\___/|___/|___/  |_____/\___|_| |_|\___/\__,_|\__,_|_|\___/_|


README

	A process scheduler in C++ that implements a Real-Time scheduler, a Multi-Level Feedback Queue scheduler, and a Hybrid Windows scheduler. 
	

AUTHORS
	
	Christian Poynter and John Frederick


INSTALLATION AND EXECUTION
	
	To compile, execute "make" in the project directory. To run, execute "./run". Select the sceduler you wish to run and input your desired values as prompted.
	

BUGS

	The Hybrid scheduler does not complete all the processes when given a large set of processes. This has been thuroughly investigated and as of yet the cause is unknown.