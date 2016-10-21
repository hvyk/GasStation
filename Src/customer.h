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
	
	CSemaphore *sTransaction;

	// Creates a name and sets the member variables
	void generateName();
	// Creates a credit card number and sets the member variable
	void generateCC();

public:
	int main(void)
	{
		//std::srand(time(NULL));
		//// Create the customer and the transaction
		//generateName();
		//generateCC();
		//// amount of gas consumed - between 5 and 70 litres in 0.5 increments
		//float quantity = (float)(std::rand() % 130) / 2;

		//time_t rawtime;
		//time(&rawtime);
		//std::string now(ctime(&rawtime));

		//FuelType type = FuelType(std::rand() % 4);

		//sTransaction = new CSemaphore("transMutex", 1);
		//CPipe pipe1("transactionPipe", 1024);
		//sTransaction->Wait();

		//// Create transaction and driect it to the pipeline
		//Transaction trans = { fullName, ccNum, now, type, quantity };
		//printf("Transaction: %s, %s, %s, %d, %1.1f\n", fullName.c_str(), ccNum.c_str(), now.c_str(), type, quantity);

		//pipe1.Write(&trans, sizeof(trans));

		//sTransaction->Signal();

		//printf("done with customer\n");

		int i = 5;
		CPipe pipe("testPipe", 1024);
		pipe.Write(&i, sizeof(i));

		//getchar();

		printf("new customer created\n");
		getchar();
		return 0;
	}
};

#endif // __CUSTOMER_H__