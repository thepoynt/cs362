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
#define PORT "9123" // the port users will be connecting to
#define BACKLOG 1 // how many pending connections queue will hold
using namespace std;

vector<int> digits;
vector<int> primes;

int run();

void printPrimes (const vector<int>& v){
  if(v.size() <= 20){
    for (int i=0; i<v.size();i++){
      cout << v[i] << " ";
    }
  } else {
    for (int i=0; i<10;i++){
      cout << v[i] << " ";
    }
    cout << "... ";
    for (int i=(v.size()-10); i<v.size();i++){
      cout << v[i] << " ";
    }
  }
  cout << "\n\n";
}

int seive(vector<int>& v) {
  int first = v[0];

  for(int i = 0; i < v.size(); i++) {
    if((v[i] % first) == 0) {
      v.erase(v.begin()+i);
    }
  }

  return 0;
}


int main () {
  int max;
  cout << "Please enter an integer value: ";
  cin >> max;

  // if the number passed in is less than 2, then there are no primes
  if (max < 2) {
    return 0;
  }

  // populate the digits with the pool of numbers up to max. This is what will be subtracted from
  for (int i =2; i <= max; i++ ) {
    digits.push_back(i);
  }
  // start the primes list with 2
  primes.push_back(2);
 
  // start off by getting rid of all even numbers before sending to first socket
  for(int i = 0; i < digits.size(); i++) {
    if(digits[i] % 2 == 0) {
      digits.erase(digits.begin()+i);
    }
  }


  // while(digits.size() > 0) {
  //   primes.push_back(digits[0]);
  //   seive(digits);
  // }

  // cout << "\nServer: All the primes up to " << max << " are:\n";
  // printPrimes(primes);
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

int is_big_endian(void){
  union {
      uint32_t i;
      char c[4];
  } bint = {0x01020304};

  return bint.c[0] == 1; 
}

int run() {
  int numbytes, arraySize;
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

      while(digits.size() > 0) {

        // Save next prime - first element of digits will be next prime
        primes.push_back(digits[0]);

        // ====== Send vector to client ======

        cout << "size of array being written: " << digits.size();
        cout << "\nSending: \n";
        printPrimes(digits);

        // send over the size of the digits vector
        int temp = digits.size();
        uint32_t size = htonl(temp);
        if(send(new_fd, &size, sizeof(size), 0) == -1)
            perror("error sending array size");

        // send over the digits vector
        if(send(new_fd, &digits[0], digits.size()*4, 0) == -1)
            perror("error sending array");

        // ====== Get vector back ======

        // get size of soon-to-come-in vector
        if ((numbytes = recv(new_fd, &arraySize, 4, 0)) == -1) {
            perror("error receiving array size");
            exit(1);
        }
        size = ntohl(arraySize);
        cout << "size of incoming array: " << size << "\n";

        // get digits vector
        digits.resize(size);
        if ((numbytes = recv(new_fd, &digits[0], size*4, 0)) == -1) {
            perror("error receiving array");
            exit(1);
        }
        digits[numbytes] = '\0';
        cout << "Received: \n";
        printPrimes(digits);
        cout << "-----------------------------\n\n";

        // store first element in primes
        primes.push_back(digits[0]);

        // seive
        if (digits.size() > 0) {
            seive(digits);
        }
      }

      // Let client know we're done by sending 0 - DOESN'T WORK
      if ((numbytes = recv(new_fd, NULL, 0, 0)) == -1) {
        perror("error sending finished message to client");
        exit(1);
      }

      cout << "Final list of primes: ";
      printPrimes(primes);

      close(new_fd);
      exit(0);
    }
    close(new_fd); // parent doesn't need this
  }
  return 0;



  // int sockfd, newsockfd, portno;
  // socklen_t clilen;
  // char buffer[32];
  // struct sockaddr_in serv_addr, cli_addr;
  // int n;

  // // Make the socket
  // sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // if (sockfd < 0) {
  //   error("ERROR opening socket");
  // }

  // bzero((char *) &serv_addr, sizeof(serv_addr));

  // portno = 9123;
  // serv_addr.sin_family = AF_INET;
  // serv_addr.sin_addr.s_addr = INADDR_ANY;
  // serv_addr.sin_port = htons(portno);

  // if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
  //   error("ERROR on binding");
  // }

  // listen(sockfd,5);
  // clilen = sizeof(cli_addr);
  // newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  // if (newsockfd < 0) {
  //   error("ERROR on accept");
  // }

  // bzero(buffer,1000);
  // n = read(newsockfd,buffer,999);
  // if (n < 0) {
  //   error("ERROR reading from socket");
  // }
  // printf("Here is the message: %s\n",buffer);

  // n = write(newsockfd,"I got your message",18);
  // if (n < 0) {
  //   error("ERROR writing to socket");
  // }

  // close(newsockfd);
  // close(sockfd);
  // return 0; 
}


