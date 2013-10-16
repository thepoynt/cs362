#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

vector<int> digits;
vector<int> primes;

void error(string msg) {
  cout << msg <<"\n";
  exit(1);
}

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


  while(digits.size() > 0) {
    primes.push_back(digits[0]);
    seive(digits);
  }

  cout << "\nAll the primes up to " << max << " are:\n";
  printPrimes(primes);
  return 0; 
}

int run() {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[1000];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  // Make the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));

  portno = 9123;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) {
    error("ERROR on accept");
  }

  bzero(buffer,1000);
  n = read(newsockfd,buffer,999);
  if (n < 0) {
    error("ERROR reading from socket");
  }
  printf("Here is the message: %s\n",buffer);

  n = write(newsockfd,"I got your message",18);
  if (n < 0) {
    error("ERROR writing to socket");
  }

  close(newsockfd);
  close(sockfd);
  return 0; 
}


