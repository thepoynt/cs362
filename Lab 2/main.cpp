#include <iostream>
#include <vector>
using namespace std;

vector<int> digits;
vector<int> primes;

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
  primes.push_back(first);

  // for(int i = 1; i < v.size(); i += first) {
  //   v.erase(v.begin()+i);
  // }
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
    seive(digits);
  }

  cout << "\nAll the primes up to " << max << " are:\n";
  printPrimes(primes);
  return 0; 
}


