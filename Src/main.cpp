#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include "rt/rt.h"
#include "station.h"
//#include "customer.h"
//#include "pump.h"
//#include "tank.h"

#define RED     12
#define GREEN   10

const std::string FIRSTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\firstnames.txt";
const std::string LASTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\Src\\names\\lastnames.txt";

char *states[4] = { "Arrived", "Ready", "Pumping", "Complete" };

const int numRendezvous = NUM_PUMPS * 2 + 2;
//const int numRendezvous = NUM_PUMPS * 2;

CRendezvous Initialize("InitRendezvous", numRendezvous );
CSemaphore ConsoleMutex("ConsoleMutex", 1);

void WritePumpStatus(struct transaction *transDP, transaction *trans);
void ReadPumpStatus(struct transaction *transDP, transaction *trans);


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

	char firstName[MAX_NAME_LEN];
	char lastName[MAX_NAME_LEN];
	char ccNum[CC_NUM_LEN];
	FuelType type;
	float quantity;
	time_t transTime;

	CSemaphore *customerData;
	CPipe *customerPipeline;

	// Creates a name and sets the member variables
	std::string readName(std::string filename);
	bool generateFirstName(char *fName, int len);
	bool generateLastName(char *lName, int len);
	// Creates a credit card number and sets the member variable
	void generateCC(char *ccNum);

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
		generateFirstName(firstName, MAX_NAME_LEN);
		generateLastName(lastName, MAX_NAME_LEN);
		generateCC(ccNum);
		this->type = FuelType(std::rand() % 4);
		this->quantity = (float)(std::rand() % 110) / 2 + 10;

		struct custInfo cust;
		strcpy(cust.firstName, firstName);
		strcpy(cust.lastName, lastName);
		strcpy(cust.ccNum, ccNum);
		cust.type = type;
		cust.quantity = quantity;

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


bool Customer::generateFirstName(char *fName, int len)
{
	// read random line from firstnames.txt
	//std::string filename = "..\\Src\\names\\firstname.txt";
	std::string filename = FIRSTNAMES;
	std::string tmp = readName(filename);
	if (tmp == "")
	{
		return false;
	}
	for (int i = 0; i < len && i < tmp.length(); i++)
	{
		if (i == len-1)
		{
			fName[i] = '\0';
		}
		else
		{
			fName[i] = tmp.at(i);
		}
	}
	return true;
}


bool Customer::generateLastName(char *lName, int len)
{
	// read random line from lastnames.txt
	//std::string filename = "..\\Src\\names\\lastname.txt";
	std::string filename = LASTNAMES;
	std::string tmp = readName(filename);
	if (tmp == "")
	{
		return false;
	}
	for (int i = 0; i < len && i < tmp.length(); i++)
	{
		if (i == len-1)
		{
			lName[i] = '\0';
		}
		else
		{
			lName[i] = tmp.at(i);
		}
	}
	return true;
}

//struct custInfo Customer::generateCustomer()
//{
//	return 
//}

// Generates a credit card for a purchase
// will inevitably be moved to the Customer class when created
void Customer::generateCC(char *ccnum)
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
	for (int i = 0; i < CC_NUM_LEN; i++)
	{
		if (i == CC_NUM_LEN - 1)
		{
			ccnum[i] = '\0';
		}
		else
		{
			ccnum[i] = tmp[i];
		}
	}
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
	struct transaction *transDP;

	CSemaphore *PS1;
	CSemaphore *CS1;

	CPipe *customerPipeline;
	CSemaphore *customerData;

	// Sends the data stored in trans to the datapool given by transDP
	//void Produce(transaction *trans);
	void WriteStatus(transaction *trans);
	// Receives the data stored in trans to the datapool given by transDP
	//void Consume(transaction *trans);
	void ReadStatus(transaction *trans);

	// Dummy code for testing until a Customer class is created
	std::string generateCC();
	void printTransaction(transaction *trans, int id, bool producing);

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

		struct custInfo cust;

		Customer *theCustomers[10];
		for (int i = 0; i < NUM_CUSTOMERS; i++)
		{
			theCustomers[i] = new Customer(id);
			theCustomers[i]->Resume();
		}


		// Add rendezvous event here
		//printf("PumpMain %d waiting for rendezvous\n", id);
		Initialize.Wait();
		//printf("Pump %d done waiting from rendezvous\n", id);

		for (int i = 0; i < NUM_CUSTOMERS; i++)
		{
			// get customer info
			customerPipeline->Read(&cust, sizeof(cust));

			// create transaction object
			// Maybe a better way to do this is to just put the customer
			//	struct inside the transaction struct?
			struct transaction trans;
			trans.state = ARRIVAL;
			strcpy(trans.firstName, cust.firstName);
			strcpy(trans.lastName, cust.lastName);
			strcpy(trans.ccNum, cust.ccNum);
			trans.time = time(NULL);
			trans.type = cust.type;
			trans.quantity = cust.quantity;

			// send to GSC with ready=false
			// We are the producer - want to send status info to GSC
			CS1->Wait();
			WriteStatus(&trans);
			PS1->Signal();

			// wait for GSC to respond with state == READY
			// Now we are the consumer - waiting for GSC's go-ahead on when
			// to start the pump
			PS1->Wait();
			ReadStatus(&trans);
			assert(trans.state == READY);
			CS1->Signal();

			// send to GSC with pumping = true
			// We are the producer - want to send status info to GSC
			trans.state = PUMPING;
			CS1->Wait();
			WriteStatus(&trans);
			PS1->Signal();

			//// Now we are the consumer - waiting for GSC's go-ahead on when
			//// to start the pump
			//trans.pumping = false;
			//PS1->Wait();
			//ReadStatus(&trans);
			//assert(!trans.pumping);
			//CS1->Signal();

			//// dispense gas
			//// send to GSC with pumping = false
		}

		//printf("=============================\n");
		//printf("%s %s\n", cust.firstName, cust.lastName);
		//printf("%s\n", cust.ccNum);
		//printf("%d\n", cust.type);
		//printf("%1.1f\n", cust.quantity);
		//printf("=============================\n");


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
	customerData = new CSemaphore("customerData0", 1);

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


	// Create the status datapool
	std::string dpName = "PumpStatusDP" + std::to_string(id);
	pumpDP = new CDataPool(dpName, sizeof(transaction));
	transDP = (struct transaction *)(pumpDP->LinkDataPool());

	customerPipeline = new CPipe(customerPipelineName);
	customerData = new CSemaphore(customerDataName, 1);

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
void Pump::WriteStatus(transaction *trans)
{
	transDP->state = trans->state;
	strcpy(transDP->firstName, trans->firstName);
	strcpy(transDP->lastName, trans->lastName);
	strcpy(transDP->ccNum, trans->ccNum);
	transDP->time = trans->time;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


//void Pump::Consume(transaction *trans)

// read the data stored in the 'Status' datapool
void Pump::ReadStatus(transaction *trans)
{
	trans->state = transDP->state;
	strcpy(trans->firstName, transDP->firstName);
	strcpy(trans->lastName, transDP->lastName);
	strcpy(trans->ccNum, transDP->ccNum);
	trans->time	= transDP->time;
	trans->type	= transDP->type;
	trans->quantity = transDP->quantity;
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




// Needs to pass information to the FuelTanks to reduce their ammount
// Also need to pass real time information to the real-time monitoring window
// TODO: Rename this to dispense
int Pump::pump(FuelType type, float quantity)
{
	//printf("\n\tStarting to pump\n");
	if (fuelTanks[type]->getRemaining() < quantity)
	{
		// Don't have enough gas to pump - should we wait until it's full?
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
	CThread fuelTankThread(tankThread, ACTIVE, NULL);

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

	// Initialization Rendezvous
	Initialize.Wait();

	for (int i = 0; i < NUM_PUMPS; i++)
	{
		pumps[i]->WaitForThread();
	}

	MOVE_CURSOR(10, 50);
	getchar();
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
	CDataPool pumpDP(dpName, sizeof(transaction));
	struct transaction *transDP = (struct transaction *)(pumpDP.LinkDataPool());

	// Create the Producer and Consumer Semaphores and give them a name and
	// index aligned with this threads index/id
	std::string producerName = "StatusProducer" + std::to_string(id);
	std::string consumerName = "StatusConsumer" + std::to_string(id);

	CSemaphore PS1(producerName, 0, 1);
	CSemaphore CS1(consumerName, 1, 1);
	
	// Initialization Rendezvous
	Initialize.Wait();

	// Read the transaction - ready should be false
	PS1.Wait();
	transaction trans;
	ReadPumpStatus(transDP, &trans);
	//assert(!trans.ready);
	CS1.Signal();

	// Print details to console
	ConsoleMutex.Wait();
	MOVE_CURSOR(10, id * 10);
	printf("%d)", id);
	MOVE_CURSOR(10, id * 10 + 1);
	printf("%s %s", trans.firstName, trans.lastName);
	MOVE_CURSOR(10, id * 10 + 2);
	printf("%s", trans.ccNum);
	MOVE_CURSOR(10, id * 10 + 3);
	printf("%s", states[trans.state]);
	MOVE_CURSOR(10, id * 10 + 4);
	printf("%1.1f", trans.quantity);
	MOVE_CURSOR(10, id * 10 + 5);
	printf("%d", trans.type);
	ConsoleMutex.Signal();

	// shou
	assert(trans.state == ARRIVAL);
	trans.state = READY;
	CS1.Wait();
	WritePumpStatus(transDP, &trans);
	PS1.Signal();

	Sleep(500);

	// Read the transaction - ready should be false
	PS1.Wait();
	ReadPumpStatus(transDP, &trans);
	assert(trans.state == PUMPING);
	CS1.Signal();

	// Print details to console
	ConsoleMutex.Wait();
	MOVE_CURSOR(10, id * 10 + 3);
	printf("%s", states[trans.state]);
	ConsoleMutex.Signal();


	return 0;
}


void WritePumpStatus(struct transaction *transDP, transaction *trans)
{
	transDP->state = trans->state;
	strcpy(transDP->firstName, trans->firstName);
	strcpy(transDP->lastName, trans->lastName);
	strcpy(transDP->ccNum, trans->ccNum);
	transDP->time = trans->time;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


void ReadPumpStatus(struct transaction *transDP, transaction *trans)
{
	trans->state = transDP->state;
	strcpy(trans->firstName, transDP->firstName);
	strcpy(trans->lastName, transDP->lastName);
	strcpy(trans->ccNum, transDP->ccNum);
	trans->time = transDP->time;
	trans->type = transDP->type;
	trans->quantity = transDP->quantity;
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

	// Initializaion Rendezvous
	Initialize.Wait();

	return 0;
}
