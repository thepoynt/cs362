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
#include <math.h>
#define PORT "9123" // the port client will be connecting to

int intSize = sizeof(int);

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

int seive(std::vector<int>& v) {
    int first = v[0];
    int size = v.size();

    int i = 0;
    while (v[i] < first*first)
        i++;

    for( ; i < size; i++) {
        if((v[i] % first) == 0) {
          v.erase(v.begin()+i);
        }
    }

    return 0;
}

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

int main(int argc, char *argv[]) {
    int sockfd, numrecvbytes, numsendbytes, arraySize;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure

    while(true) {
        // get size of soon-to-come-in vector
        if ((numrecvbytes = recv(sockfd, &arraySize, intSize, 0)) == -1) {
            perror("error receiving array size");
            exit(1);
        }
        int size = arraySize;
        // cout << "size of incoming array: " << size << "\n";

        // get digits vector
        std::vector<int> digits;
        digits.resize(size);
        // for (int i=0; i<size; i++) {
        //     numrecvbytes = recv(sockfd, &digits[i], intSize, 0);
        //     i++;
        // }
        // if ((numrecvbytes = recv(sockfd, &digits[0], digits.size()*intSize, 0)) == -1) {
        //     perror("error receiving array");
        //     exit(1);
        // }
        numrecvbytes = recv(sockfd, &digits[0], size*intSize, 0);
        if (numrecvbytes == -1) {
            perror("error receiving array");
            exit(1);
        }
        while (numrecvbytes < size*intSize) {
            numrecvbytes += recv(sockfd, &digits[numrecvbytes/intSize], size*intSize - numrecvbytes, 0);
        }
        std::cout << "\nReceived: \n";
        printInOutVector(digits);
        // cout << "received " << numrecvbytes << " bytes of data\n\n";

        // seive
        seive(digits);
        std::cout << "Sending: \n";
        printInOutVector(digits);

        // send over the size of the digits vector
        size = digits.size();
        // size = htonl(temp);
        if(send(sockfd, &size, intSize, 0) == -1)
            perror("error sending array size");

        // cout << "size of array being written: " << digits.size();

        // send over the digits vector
        // for (int i=0; i<size; i++) {
        //     send(sockfd, &digits[i], intSize, 0);
        // }
        numsendbytes = send(sockfd, &digits[0], size*intSize, 0);
        if(numsendbytes == -1)
            perror("error sending array");
        while (numsendbytes < size*intSize) {
            numsendbytes += send(sockfd, &digits[numsendbytes/intSize], size*intSize - numsendbytes, 0);
        }
        // cout << "sent " << numsendbytes << " bytes of data\n\n";
        // if(send(sockfd, &digits[0], digits.size()*intSize, 0) == -1)
        // perror("error sending array");

        // cout << "\n\n-----------------------------\n\n";
    }

    close(sockfd);
    return 0;
}


