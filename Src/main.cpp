#include <vector>
#include "rt/rt.h"
#include "station.h"
//#include "customer.h"
//#include "pump.h"
//#include "tank.h"

#define RED     12
#define GREEN   10

const int numRendezvous = NUM_PUMPS * 2 + 1;
//const int numRendezvous = NUM_PUMPS * 2;

CRendezvous Initialize("InitRendezvous", numRendezvous );
CSemaphore ConsoleMutex("ConsoleMutex", 1);


/*=================================================
 *					Customer
 *================================================*/



/*=================================================
 *					Tank
 *================================================*/
class FuelTank
{
private:
	// The Tank datapool
	struct Tank {
		float remaining;
		FuelType type;
	};

	CDataPool *theDataPool;
	struct Tank *tank;

	// Mutex to project the remaining amount of fuel
	CMutex *mRemain;

public:
	FuelTank();
	FuelTank(FuelType type, float remaining);
	FuelTank(const FuelTank &obj);
	~FuelTank();

	// decrements the tank by 1 litre and returns the number of litres left
	float decrement();
	// Fills the tank to full
	float fill();
	// returns the amount of fuel remaining
	float getRemaining();
	// returns the type of fuel in the tank
	FuelType getType();
};


FuelTank::FuelTank()
{
	//printf("FuelTank default constructor\n");
	mRemain = new CMutex("remaining", 1);

	theDataPool = new CDataPool(string("OCTANE") + to_string(0), sizeof(struct Tank));
	tank = (struct Tank *)(theDataPool->LinkDataPool()); 

	mRemain->Wait();
	tank->remaining = MAX_CAPACITY;
	tank->type = OCTANE87;
	mRemain->Signal();
}

FuelTank::FuelTank(FuelType type, float remaining)
{
	//printf("FuelTank constructor %d %1.1f\n", type, remaining);
	mRemain = new CMutex("remaining", 1);

	theDataPool = new CDataPool(string("OCTANE") + to_string(type), sizeof(struct Tank));
	tank = (struct Tank *)(theDataPool->LinkDataPool()); 

	mRemain->Wait();
	tank->remaining = remaining;
	tank->type = OCTANE87;
	mRemain->Signal();
}

FuelTank::FuelTank(const FuelTank &obj)
{
	tank->remaining = obj.tank->remaining;
}

FuelTank::~FuelTank()
{
	delete theDataPool;
}

float FuelTank::decrement()
{
	float newVal;
	
	mRemain->Wait();
	tank->remaining = tank->remaining - (float)0.5;
	newVal = tank->remaining;
	mRemain->Signal();

	return newVal;
}


float FuelTank::fill()
{
	float newVal;

	mRemain->Wait();
	tank->remaining = MAX_CAPACITY;
	newVal = tank->remaining;
	mRemain->Signal();

	return newVal;
}

float FuelTank::getRemaining()
{
	float retVal;

	mRemain->Wait();
	retVal = tank->remaining;
	mRemain->Signal();

	//printf(">>> remaining = %1.1f\n", retVal);
	return retVal;
}

FuelType FuelTank::getType()
{
	return tank->type;
}


/*=================================================
 *					Pump
 *================================================*/
class Pump : public ActiveClass
{
private:
	// The pump number/id
	int id;

	CDataPool *pumpDP;
	struct Transaction *transDP;

	CSemaphore *PS1;
	CSemaphore *CS1;

	// Sends the data stored in trans to the datapool given by transDP
	//void Produce(Transaction *trans);
	void WriteStatus(Transaction *trans);
	// Receives the data stored in trans to the datapool given by transDP
	//void Consume(Transaction *trans);
	void ReadStatus(Transaction *trans);

	// Dummy code for testing until a Customer class is created
	Transaction generateTransaction();
	std::string generateCC();
	void printTransaction(Transaction *trans, int id, bool producing);

	std::vector<FuelTank *> fuelTanks;

public:
	Pump();
	Pump(int id);
	~Pump();

	int pump(FuelType type, float quantity);

	int main(void)
	{
		// Initialize everything necessary for this thread
		std::string dpName = "PumpStatusDP" + std::to_string(id);
		CDataPool *pumpDP = new CDataPool(dpName, sizeof(Transaction));
		transDP = (struct Transaction *)(pumpDP->LinkDataPool());

		// Add rendezvous event here
		printf("PumpMain %d waiting for rendezvous\n", id);
		Initialize.Wait();
		printf("Pump %d done waiting from rendezvous\n", id);

		return 0;
	}

};


// Baaad, don't use this constructor for more than one pump
Pump::Pump()
{
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
// TODO: Rename this to dispense
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



/*=================================================
 *					GSC
 *================================================*/

UINT __stdcall pumpThread(void *args);
UINT __stdcall tankThread(void *args);


// This is the gas station computer
int main(int argc, char* argv[])
{

	vector<CThread *> threads;
	vector<Pump *> pumps;

	// Create NUM_PUMPS pumps
	for (int i = 0; i < NUM_PUMPS; i++)
	{
		CThread *thread_i = new CThread(pumpThread, ACTIVE, &i);
		threads.push_back( thread_i );

		Pump *pump_i = new Pump(i);
		pump_i->Resume();
		pumps.push_back( pump_i );

		Sleep(100);
	}


	// Add rendezvous event here
	printf("main waiting for rendezvous\n");
	Initialize.Wait();
	printf("main done waiting from rendezvous\n");

	return 0;
}


/**
 * This represents the communication between the pump and the GSC
 * @param
 *		A pointer to an integer with the thread number to be used to
 *		create the producer and consumer semaphores
 */
UINT __stdcall pumpThread(void *args)
{
	int id = *(int *)args;
	printf("pumpThread %d waiting for rendezvous\n", id);
	Initialize.Wait();
	printf("pumpThread %d done waiting from rendezvous\n", id);

	return 0;
}


UINT __stdcall tankThread(void *args)
{
	printf("tankThread waiting for rendezvous\n");
	Initialize.Wait();
	printf("tankThread done waiting from rendezvous\n");

	return 0;
}
