#include <string>
#include "car.h"

Car::Car() {
	isFront = false;
}

Car::Car(int q) {
	queue = q;
	isFront = false;
}

void Car::setQueue(int q) {
	queue = q;
}

void Car::setId(std::string i) {
	id = i;
}