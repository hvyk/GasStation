#include <vector>
#include <iostream>
#include <fstream>
#include "rt/rt.h"
#include "station.h"
//#include "customer.h"
//#include "pump.h"
//#include "tank.h"

#define RED     12
#define GREEN   10

//#define FIRSTNAMES "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\firstnames.txt"
//#define LASTNAMES "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\lastnames.txt"

const std::string FIRSTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\firstnames.txt";
const std::string LASTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\lastnames.txt";

const int numRendezvous = NUM_PUMPS * 2 + 2;
//const int numRendezvous = NUM_PUMPS * 2;

CRendezvous Initialize("InitRendezvous", numRendezvous );
CSemaphore ConsoleMutex("ConsoleMutex", 1);



// rand is thread local, so let's change this up
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
	a = a - b;  a = a - c;  a = a ^ (c >> 13);
	b = b - c;  b = b - a;  b = b ^ (a << 8);
	c = c - a;  c = c - b;  c = c ^ (b >> 13);
	a = a - b;  a = a - c;  a = a ^ (c >> 12);
	b = b - c;  b = b - a;  b = b ^ (a << 16);
	c = c - a;  c = c - b;  c = c ^ (b >> 5);
	a = a - b;  a = a - c;  a = a ^ (c >> 3);
	b = b - c;  b = b - a;  b = b ^ (a << 10);
	c = c - a;  c = c - b;  c = c ^ (b >> 15);
	return c;
}


/*=================================================
 *					Customer
 *================================================*/
class Customer : public ActiveClass
{
private:
	// The id of the pump for semaphore management
	int id;

	std::string firstName;
	std::string lastName;
	std::string ccNum;
	FuelType type;
	float quantity;

	time_t transTime;

	CSemaphore *customerData;
	CPipe *customerPipeline;

	// Creates a name and sets the member variables
	std::string readName(std::string filename);
	std::string generateFirstName();
	std::string generateLastName();
	// Creates a credit card number and sets the member variable
	std::string generateCC();

	//struct custInfo generateCustomer();

public:
	Customer();
	Customer(int id);
	~Customer();

	int main(void)
	{
		// see the rand that uses more than just time (because rand() is thread local)
		unsigned long seed = mix(clock(), time(NULL), _getpid());
		srand(seed);

		// Generate a Customer using 'struct custInfo'
		this->firstName = generateFirstName();
		this->lastName = generateLastName();
		this->ccNum = generateCC();
		this->type = FuelType(std::rand() % 4);
		this->quantity = (float)(std::rand() % 110) / 2 + 10;

		struct custInfo cust = {
			firstName,
			lastName,
			ccNum,
			type,
			quantity
		};

		// Write to pipeline
		customerData->Wait();
		customerPipeline->Write(&cust, sizeof(cust));
		customerData->Signal();

		return 0;
	}
};

Customer::Customer()
{
	this->id = 0;
	customerData = new CSemaphore("customerData0", 1);
	customerPipeline = new CPipe("customerPipeline0");
}

Customer::Customer(int id)
{
	this->id = id;
	std::string customerDataName = "customerData" + std::to_string(id);
	std::string customerPipelineName = "customerPipeline" + std::to_string(id);
	customerData = new CSemaphore(customerDataName, 1);
	customerPipeline = new CPipe(customerPipelineName);
}


Customer::~Customer()
{
	delete customerData;
	delete customerPipeline;
}

std::string Customer::readName(std::string filename)
{
	ifstream infile(filename);
	if (infile)
	{
		std::string line;
		std::vector<std::string> lines;

		while (getline(infile, line)) {
			lines.push_back(line);
		}

		int len = lines.size();
		int idx = rand() % len;

		//printf("readName creating name %s from filename %s",
		//	lines[idx], filename.c_str());
		return lines[idx];
	}
	return "";
}


std::string Customer::generateFirstName()
{
	// read random line from firstnames.txt
	//std::string filename = "..\\Src\\names\\firstname.txt";
	std::string filename = FIRSTNAMES;
	return readName(filename);
}


std::string Customer::generateLastName()
{
	// read random line from lastnames.txt
	//std::string filename = "..\\Src\\names\\lastname.txt";
	std::string filename = LASTNAMES;
	return readName(filename);
}

//struct custInfo Customer::generateCustomer()
//{
//	return 
//}

// Generates a credit card for a purchase
// will inevitably be moved to the Customer class when created
std::string Customer::generateCC()
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

	CPipe *customerPipeline;
	CSemaphore *customerData;

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
		// Add access to the fuel tanks
		vector<FuelTank *> fuelTanks;
		for (int octane = OCTANE87; octane <= OCTANE94; octane++)
		{
			FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
			fuelTanks.push_back(tank_i);
		}

		// Create the status datapool
		std::string dpName = "PumpStatusDP" + std::to_string(id);
		CDataPool *pumpDP = new CDataPool(dpName, sizeof(Transaction));
		transDP = (struct Transaction *)(pumpDP->LinkDataPool());

		// Add rendezvous event here
		printf("PumpMain %d waiting for rendezvous\n", id);
		Initialize.Wait();
		printf("Pump %d done waiting from rendezvous\n", id);

		// Generating customers
		Customer *theCustomers[100];
		for (int i = 0; i < 100; i++)
		{
			theCustomers[i] = new Customer(id);
			theCustomers[i]->Resume();
		}

		struct custInfo cust;
		
		customerPipeline = new CPipe("customerPipeline0");
		customerData->Signal();
		customerPipeline->Read(&cust, sizeof(cust));
		



		return 0;
	}

};


// Baaad, don't use this constructor for more than one pump
Pump::Pump()
{
	id = 0;

	PS1 = new CSemaphore("StatusProducer0", 0, 1);
	CS1 = new CSemaphore("StatusConsumer0", 1, 1);

	customerPipeline = new CPipe("customerPipeline0");
	customerData = new CSemaphore("customerData0", 1, 0);

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

	std::string producerName = "StatusProducer" + std::to_string(this->id);
	std::string consumerName = "StatusConsumer" + std::to_string(this->id);
	std::string customerDataName = "customerData" + std::to_string(this->id);
	std::string customerPipelineName = "customerPipeline" + std::to_string(this->id);

	PS1 = new CSemaphore(producerName, 0, 1);
	CS1 = new CSemaphore(consumerName, 1, 1);

	customerPipeline = new CPipe(customerPipelineName);
	customerData = new CSemaphore(customerDataName, 1, 0);

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
	delete customerData;
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

std::string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

// This is the gas station computer
int main(int argc, char* argv[])
{
	//srand(time(NULL));
	//printf("{ ");
	//for (int i = 0; i < 20; i++)
	//{
	//	printf("%d, ", rand() % 10);
	//}
	//printf("}\n\n");
	//cout << "my directory is " << ExePath() << "\n";

	CSemaphore *customerData = new CSemaphore("customerData0", 1);
	CPipe *customerPipeline = new CPipe("customerPipeline0");

	struct custInfo cust;

	Customer *theCustomers[10];
	for (int i = 0; i < 10; i++)
	{
		theCustomers[i] = new Customer();
		theCustomers[i]->Resume();
		Sleep(200);
	//}


	//for (int i = 0; i < 10; i++)
	//{
		// read from the pipeline
		//customerData->Wait();
		customerPipeline->Read(&cust, sizeof(cust));
		//customerData->Signal();

		printf("=============================\n");
		printf("%s %s\n", cust.firstName.c_str(), cust.lastName.c_str());
		printf("%s\n", cust.ccNum.c_str());
		printf("%d\n", cust.type);
		printf("%1.1f\n", cust.quantity);
		printf("=============================\n");
	}

	//CThread tankThread(tankThread, ACTIVE, NULL);

	//vector<CThread *> threads;
	//vector<Pump *> pumps;

	//// Create NUM_PUMPS pumps
	//for (int i = 0; i < NUM_PUMPS; i++)
	//{
	//	CThread *thread_i = new CThread(pumpThread, ACTIVE, &i);
	//	threads.push_back( thread_i );

	//	Pump *pump_i = new Pump(i);
	//	pump_i->Resume();
	//	pumps.push_back( pump_i );

	//	Sleep(100);
	//}


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
	int id = *(int*)args;

	// Create the status datapool
	std::string dpName = "PumpStatusDP" + std::to_string(id);
	CDataPool pumpDP(dpName, sizeof(Transaction));
	struct Transaction *transDP = (struct Transaction *)(pumpDP.LinkDataPool());
	
	printf("pumpThread %d waiting for rendezvous\n", id);
	Initialize.Wait();
	printf("pumpThread %d done waiting from rendezvous\n", id);

	return 0;
}


UINT __stdcall tankThread(void *args)
{
	// Create the fuel tanks
	vector<FuelTank *> fuelTanks;
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}

	printf("tankThread waiting for rendezvous\n");
	Initialize.Wait();
	printf("tankThread done waiting from rendezvous\n");

	return 0;
}
