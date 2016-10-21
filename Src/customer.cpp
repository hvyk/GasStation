#include <time.h>
#include "customer.h"

void Customer::generateName()
{
	std::vector<std::string> firstNames = { "Bobby", "Johnny", "Jimbo", "Sean", "Don", "R2" };
	std::vector<std::string> lastNames = { "Dan", "McCallum", "Baggins", "Piccard", "Riker", "D2" };

	std::srand(time(NULL));

	this->firstName = firstNames[std::rand() % 6];
	this->lastName = lastNames[std::rand() % 6];
	this->fullName = firstName + " " + lastName;

	printf(">>> DEBUG: %s, %s", this->lastName.c_str(), this->firstName.c_str());
}


void Customer::generateCC()
{
	std::string tmp;
	int num;
	for (int i = 0; i < CC_LENGTH; i++)
	{
		num = rand() % 9;
		if (i % 4 == 0)
			tmp += " ";
		tmp += std::to_string(num);
	}
	printf(">> DEBUG: %s\n", tmp.c_str());
	this->ccNum = tmp;
}