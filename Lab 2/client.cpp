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
    printf("... ");
    for (int i=(v.size()-10); i<v.size();i++){
      std::cout << v[i] << " ";
    }
  }
  printf("\n\n");
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
    printf("... ");
    for (int i=(v.size()-5); i<v.size();i++){
      std::cout << v[i] << " ";
    }
  }
  printf("\n\n");
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

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
        // decode size
        int size = ntohl(arraySize);

        // get digits vector
        std::vector<int> digits;
        digits.resize(size);
        numrecvbytes = recv(sockfd, &digits[0], size*intSize, 0);
        if (numrecvbytes == -1) {
            perror("error receiving array");
            exit(1);
        }
        while (numrecvbytes < size*intSize) {
            numrecvbytes += recv(sockfd, &digits[numrecvbytes/intSize], size*intSize - numrecvbytes, 0);
        }
        
        for (int i = 0; i < size; i++) { // decode vector
            digits[i] = ntohl(digits[i]);
        }
        printf("\nReceived: \n");
        printInOutVector(digits);

        // sieve
        sieve(digits);

        // send over the size of the digits vector
        size = digits.size();
        int encSize = htonl(size); // encode size
        if(send(sockfd, &encSize, intSize, 0) == -1)
            perror("error sending array size");

        // send over the vector
        printf("Sending: \n");
        printInOutVector(digits);
        for (int i = 0; i < size; i++) { // encode vector
            digits[i] = htonl(digits[i]);
        }
        numsendbytes = send(sockfd, &digits[0], size*intSize, 0);
        if(numsendbytes == -1)
            perror("error sending array");
        while (numsendbytes < size*intSize) {
            numsendbytes += send(sockfd, &digits[numsendbytes/intSize], size*intSize - numsendbytes, 0);
        }

        printf("----------------------------------------\n\n");
    }

    close(sockfd);
    return 0;
}


