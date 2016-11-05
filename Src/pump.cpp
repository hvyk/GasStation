#include <time.h>
#include <vector>
#include "tank.h"
#include "pump.h"

// Baaad, don't use this constructor for more than one pump
Pump::Pump()
{
	//CDataPool pumpDP("PumpStatusDP", sizeof(Transaction));
	//transDP = (struct Transaction *)(pumpDP.LinkDataPool());
	id = 0;

	PS1 = new CSemaphore("StatusProducer0", 0, 1);
	CS1 = new CSemaphore("StatusConsumer0", 1, 1);


	// Need access to the fuel tank data pools
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}

	int i = 0;
}


Pump::Pump(int id)
{
	//CDataPool pumpDP("PumpDP", sizeof(Transaction));
	//transDP = (struct Transaction *)(pumpDP.LinkDataPool());
	this->id = id;

	std::string producerName = "StatusProducer" + this->id;
	std::string consumerName = "StatusConsumer" + this->id;

	PS1 = new CSemaphore(producerName, 0, 1);
	CS1 = new CSemaphore(consumerName, 1, 1);


	// Need access to the fuel tank data pools
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}
}


Pump::~Pump()
{
	delete PS1;
	delete CS1;
}


//void Pump::Produce(Transaction *trans)

// Write data to be stored in the 'Status' datapool
void Pump::WriteStatus(Transaction *trans)
{
	transDP->ready = trans->ready;
	transDP->pumping = trans->pumping;
	transDP->firstName = trans->firstName;
	transDP->lastName = trans->lastName;
	transDP->ccNum = trans->ccNum;
	transDP->time = trans->time;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


//void Pump::Consume(Transaction *trans)

// read the data stored in the 'Status' datapool
void Pump::ReadStatus(Transaction *trans)
{
	trans->ready = transDP->ready;
	trans->pumping = transDP->pumping;
	trans->firstName = transDP->firstName;
	trans->lastName = transDP->lastName;
	trans->ccNum = transDP->ccNum;
	trans->time	= transDP->time;
	trans->type	= transDP->type;
	trans->quantity = transDP->quantity;
}


// Generate the information required for a transaction - all
// of which is dummy sample data
Transaction Pump::generateTransaction()
{
	std::string firstName = "Bobby";
	std::string lastName = "Dan";
	std::string ccNum = generateCC();
	time_t		now = time(0);
	FuelType	type = FuelType(std::rand() % 4);
	float		quantity = (float)(std::rand() % 130) / 2;

	Transaction trans = { false, false, firstName, lastName, ccNum, now, type, quantity };
	return trans;
}


// Generates a credit card for a purchase
// will inevitably be moved to the Customer class when created
std::string Pump::generateCC()
{
	std::string tmp;
	int num;
	for (int i = 0; i < CC_LENGTH; i++)
	{
		num = rand() % 9;
		if ( i > 0 && i % 4 == 0)
			tmp += " ";
		tmp += std::to_string(num);
	}
	return tmp;
}


// Prints the details of a transaction, which is generated randomly
void Pump::printTransaction(Transaction *trans, int id, bool producing)
{
	char *fType = "OCTANE 87";
	switch (trans->type)
	{
	case OCTANE87:
		fType = "OCTANE 87";
		break;
	case OCTANE89:
		fType = "OCTANE 89";
		break;
	case OCTANE91:
		fType = "OCTANE 91";
		break;
	case OCTANE94:
		fType = "OCTANE 94";
		break;
	}

	char *transTime = ctime(&(trans->time));

	printf("\tPump %d is %s\n", id, producing ? "Producing" : "Consuming");
	printf("\t%s %s\n", trans->firstName.c_str(), trans->lastName.c_str());
	printf("\t%s\n", trans->ccNum.c_str());
	printf("\t%s", transTime); // \n char added by ctime(...)
	printf("\t%s\n", fType);
	printf("\t%1.1f\n\n", trans->quantity);
}


// Needs to pass information to the FuelTanks to reduce their ammount
// Also need to pass real time information to the real-time monitoring window
int Pump::pump(FuelType type, float quantity)
{
	//printf("\n\tStarting to pump\n");
	if (fuelTanks[type]->getRemaining() < quantity)
	{
		// Don't have enough gas to pump - should we wait until it's full?
		//printf("\t\tno fuel remaining in tank\n");
		return -1;
	}
	while (quantity > 0)
	{
		fuelTanks[type]->decrement();
		quantity -= 0.5;
		//printf("decrementing pump id %d to %1.1f >>> %1.1f\n", id, quantity, fuelTanks[type]->getRemaining());
		Sleep(50);
	}
	return 0;
}
