#ifndef __CUSTOMER_H__
#define __CUSTOMER_H__

#include <stdio.h>
#include <vector>
#include <time.h>
#include "rt/rt.h"
#include "station.h"



class Customer : public ActiveClass
{
private:
	std::string firstName;
	std::string lastName;
	std::string fullName;
	std::string ccNum;
	
	CMutex *mTransaction;

	// Creates a name and sets the member variables
	void generateName();
	// Creates a credit card number and sets the member variable
	void generateCC();

public:
	int main(void)
	{
		// Create the customer and the transaction
		generateName();
		generateCC();
		// amount of gas consumed - between 5 and 70litres in 0.5 increments
		float quantity = (float)(std::rand() % 130) / 2;

		time_t rawtime;
		time(&rawtime);
		std::string now(ctime(&rawtime));

		FuelType type = FuelType(std::rand() % 4);

		Transaction trans = { fullName, ccNum, now, type, quantity };
		printf("Transaction: %s, %s, %s, %d, %1.1f\n", fullName.c_str(), ccNum.c_str(), now.c_str(), type, quantity);

		return 0;
	}
};

#endif // __CUSTOMER_H__