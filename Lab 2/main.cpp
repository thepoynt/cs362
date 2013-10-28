#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#define PORT "9123" // the port users will be connecting to
#define BACKLOG 1 // how many pending connections queue will hold

std::vector<int> digits;
std::vector<int> primes;
int intSize = sizeof(int);
time_t start,end;
int max;

int run();

void printPrimes (const std::vector<int>& v){
  if(v.size() <= 20){
    for (int i=0; i<v.size();i++){
      std::cout << v[i] << " ";
    }
  } else {
    for (int i=0; i<10;i++){
      std::cout << v[i] << " ";
    }
    std::cout << "... ";
    for (int i=(v.size()-10); i<v.size();i++){
      std::cout << v[i] << " ";
    }
  }
  std::cout << "\n\n";
}

void printInOutVector (const std::vector<int>& v){
  if(v.size() <= 10){
    for (int i=0; i<v.size();i++){
      std::cout << v[i] << " ";
    }
  } else {
    for (int i=0; i<5;i++){
      std::cout << v[i] << " ";
    }
    std::cout << "... ";
    for (int i=(v.size()-5); i<v.size();i++){
      std::cout << v[i] << " ";
    }
  }
  std::cout << "\n\n";
}

int sieve(std::vector<int>& v) {
    int first = v[0];
    int size = v.size();
    std::vector<int> newV;

    for(int i = 0; i < size; i++) {
        if((v[i] % first) == 0) {
            v[i] = 0;
        }
    }
    for(int i = 0; i < size; i++) {
        if(v[i] != 0) {
            newV.push_back(v[i]);
        }
    }

    v = newV;

    return 0;
}


int main () {
    printf("Please enter an integer value: ");
    std::cin >> max;

    // if the number passed in is less than 2, then there are no primes
    if (max < 2) {
    return 0;
    }

    // populate the digits with the pool of numbers up to max. This is what will be subtracted from
    for (int i =3; i <= max; i += 2 ) {
        digits.push_back(i);
    }
    // start the primes list with 2
    primes.push_back(2);

    run();
    return 0; 
}

void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
  }

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int run() {
  int numrecvbytes, numsendbytes, arraySize;
  int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
    continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); // all done with this structure
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  printf("server: waiting for connections...\n");

  while(1) { // main accept() loop
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n\n", s);
    if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener

      time (&start); // start timer

      while((digits.size() > 0) && (digits[0] < sqrt(max))) {

        // Save next prime - first element of digits will be next prime
        primes.push_back(digits[0]);

        // ====== Send vector to client ======

            // send over the size of the digits vector
            int size = digits.size();
            int encSize = htonl(size); //encode size
            if(send(new_fd, &encSize, intSize, 0) == -1)
                perror("error sending array size");

            // send over the digits vector
            printf("\nSending: \n");
            printInOutVector(digits);
            printf("----------------------------------------\n\n");
            
            for (int i = 0; i < size; i++) { // encode vector
                digits[i] = htonl(digits[i]);
            }

            numsendbytes = send(new_fd, &digits[0], size*intSize, 0);
            if(numsendbytes == -1)
                perror("error sending array");
            while (numsendbytes < size*intSize) {
                numsendbytes += send(new_fd, &digits[numsendbytes/intSize], size*intSize - numsendbytes, 0);
            }

        // ====== Get vector back ======

            // get size of soon-to-come-in vector
            if ((numrecvbytes = recv(new_fd, &arraySize, intSize, 0)) == -1) {
                perror("error receiving array size");
                exit(1);
            }
            size = ntohl(arraySize); //decode size

            // get digits vector
            digits.resize(size);
            numrecvbytes = recv(new_fd, &digits[0], size*intSize, 0);

            if (numrecvbytes == -1) {
                perror("error receiving array");
                exit(1);
            }
            while (numrecvbytes < size*intSize) {
                numrecvbytes += recv(new_fd, &digits[numrecvbytes/intSize], size*intSize - numrecvbytes, 0);
            }

            for (int i = 0; i < size; i++) { // decode vector
                digits[i] = ntohl(digits[i]);
            }

            std::cout << "Received: \n";
            printInOutVector(digits);

            // store first element in primes
            primes.push_back(digits[0]);

            // sieve
            if (digits.size() > 0) {
                sieve(digits);
            }
      }

      // Let client know we're done by sending 0
      if (send(new_fd, NULL, 0, 0) == -1) {
        perror("error sending finished message to client");
        exit(1);
      }

      for (int i =0; i < digits.size(); i++ ) {
        primes.push_back(digits[i]);
      }

      std::cout << "Final list of primes: ";
      printPrimes(primes);
      std::cout << "Number of primes: " << primes.size();
      time (&end);
      double duration = difftime (end,start);
      std::cout << "\nTime elapsed: " << duration;

      close(new_fd);
      exit(0);
    }
    close(new_fd); // parent doesn't need this
  }
  return 0;

}


