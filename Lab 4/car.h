#ifndef CAR_H
#define CAR_H

class Car {
	#include <string>

	public:
		Car();
		Car(int);
		void setQueue(int);
		void setId(std::string);
		std::string id;
		int queue;
		bool isFront;
};

#endif
