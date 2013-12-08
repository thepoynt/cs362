#include <string>
#include "car.h"

Car::Car() {

}

Car::Car(int q) {
	queue = q;
}

void Car::setQueue(int q) {
	queue = q;
}

void Car::setId(std::string i) {
	id = i;
}