#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include "rt/rt.h"
#include "station.h"
//#include "customer.h"
//#include "pump.h"
#include "tank.h"


#define WHITE		7
#define RED			12
#define GREEN		10
#define YELLOW		14


const std::string FIRSTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\GasStation\\Src\\names\\firstnames.txt";
const std::string LASTNAMES = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\GasStation\\Src\\names\\lastnames.txt";

char *states[4] = { "Arrived", "Ready", "Pumping", "Complete" };

// One pump and one pumpStatus thread per pump, plus one tankThread and
// one pumpThread
//const int numRendezvous = NUM_PUMPS * 2 + 2;
const int numRendezvous = NUM_PUMPS + 2;

CRendezvous Initialize("InitRendezvous", numRendezvous );
CRendezvous Customers("CustomersRendezvous", NUM_CUSTOMERS );
CSemaphore ConsoleMutex("ConsoleMutex", 1);

void WritePumpStatus(transaction_t *transDP, transaction *trans);
void ReadPumpStatus(transaction_t *transDP, transaction *trans);


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

		Customers.Wait();

		// Generate a Customer using 'customer_t'
		generateFirstName(firstName, MAX_NAME_LEN);
		generateLastName(lastName, MAX_NAME_LEN);
		generateCC(ccNum);
		//this->type = FuelType(std::rand() % 4);
		//this->quantity = (float)(std::rand() % MAX_QUANTITY) / 2 + MIN_QUANTITY;

		customer_t cust;
		strcpy(cust.firstName, firstName);
		strcpy(cust.lastName, lastName);
		strcpy(cust.ccNum, ccNum);
		//cust.type = type;
		//cust.quantity = quantity;

		// Write to pipeline
		customerData->Wait();
		customerPipeline->Write(&cust, sizeof(cust));
		customerData->Signal();

		return 0;
	}
};

Customer::Customer()
{
	id = 0;

	customerData = new CSemaphore("customerData0", 1);
	customerPipeline = new CPipe("customerPipeline0");
}

Customer::Customer(int id)
{
	id = id;

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

		return lines[idx];
	}
	return "";
}


bool Customer::generateFirstName(char *fName, int len)
{
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
 *					Pump
 *================================================*/
class Pump : public ActiveClass
{
private:
	// The pump number/id
	int id;
	bool dispensingFlg;

	CDataPool *pumpDP;
	transaction_t *transDP;

	CSemaphore *PS1;
	CSemaphore *CS1;
	CSemaphore *PS2;
	CSemaphore *CS2;
	CSemaphore *Pumping;

	CPipe *customerPipeline;
	CSemaphore *customerData;

	// Sends the data stored in trans to the datapool given by transDP
	void WriteStatus(transaction_t *trans);
	// Receives the data stored in trans to the datapool given by transDP
	void ReadStatus(transaction_t *trans);

	// Dummy code for testing until a Customer class is created
	std::string generateCC();
	void printTransaction(transaction_t *trans, int id, bool producing);

	void dispense(FuelType octane, float quantity);
	void generateTransaction(transaction_t *trans, customer_t *cust);

	std::vector<FuelTank *> fuelTanks;

public:
	Pump();
	Pump(int id);
	~Pump();

	int pump(FuelType type, float quantity);

	int main(void)
	{
		// see the rand that uses more than just time (because rand() is thread local)
		unsigned long seed = mix(clock(), time(NULL), _getpid());
		srand(seed);

		// Add access to the fuel tanks
		vector<FuelTank *> fuelTanks;
		for (int octane = OCTANE87; octane <= OCTANE94; octane++)
		{
			FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
			fuelTanks.push_back(tank_i);
		}

		customer_t cust;

		Customer *theCustomers[NUM_CUSTOMERS];
		for (int i = 0; i < NUM_CUSTOMERS; i++)
		{
			theCustomers[i] = new Customer(id);
			theCustomers[i]->Resume();
		}

		// Add rendezvous event here
		Initialize.Wait();

		for (int i = 0; i < NUM_CUSTOMERS; i++)
		{
			// get customer info and create a transaction object
			transaction_t trans;
			customerPipeline->Read(&cust, sizeof(cust));
			generateTransaction(&trans, &cust);
			trans.state = ARRIVAL;

			// send to GSC with state == ARRIVAL
			// We are the producer - want to send status info to GSC
			CS1->Wait();
			WriteStatus(&trans);
			//printf("GSC Producing\n");
			PS1->Signal();

			// wait for GSC to respond with state == READY
			// Now we are the consumer - waiting for GSC's go-ahead on when
			// to start the pump
			PS2->Wait();
			ReadStatus(&trans);
			assert(trans.state == READY);

			Pumping->Wait();
			//printf("GSC Consuming\n");
			CS2->Signal();

			dispense(trans.type, trans.quantity);
			Pumping->Signal();
			Sleep(200);
		}


		for (int octane = OCTANE87; octane <= OCTANE94; octane++)
		{
			delete fuelTanks[octane];
		}

		return 0;
	}

};


// Baaad, don't use this constructor for more than one pump
Pump::Pump()
{
	this->id = 0;
	this->dispensingFlg = false;

	PS1 = new CSemaphore("StatusProducer10", 0, 1);
	CS1 = new CSemaphore("StatusConsumer10", 1, 1);
	PS2 = new CSemaphore("StatusProducer20", 0, 1);
	CS2 = new CSemaphore("StatusConsumer20", 1, 1);

	Pumping = new CSemaphore("Pump0", 1, 1);

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
	//printf("id: %d\n", id);
	this->dispensingFlg = false;

	std::string producerName1 = "StatusProducer1" + std::to_string(this->id);
	std::string consumerName1 = "StatusConsumer1" + std::to_string(this->id);
	std::string producerName2 = "StatusProducer2" + std::to_string(this->id);
	std::string consumerName2 = "StatusConsumer2" + std::to_string(this->id);
	std::string pumpingName = "Pump" + std::to_string(this->id);
	std::string customerDataName = "customerData" + std::to_string(this->id);
	std::string customerPipelineName = "customerPipeline" + std::to_string(this->id);

	//printf("%s\n%s\n%s\n%s\n",
	//	producerName1.c_str(),
	//	consumerName1.c_str(),
	//	producerName2.c_str(),
	//	consumerName2.c_str()
	//);

	PS1 = new CSemaphore(producerName1, 0, 1);
	CS1 = new CSemaphore(consumerName1, 1, 1);
	PS2 = new CSemaphore(producerName2, 0, 1);
	CS2 = new CSemaphore(consumerName2, 1, 1);
	Pumping = new CSemaphore(pumpingName, 1, 1);

	// Create the status datapool
	std::string dpName = "PumpStatusDP" + std::to_string(id);
	pumpDP = new CDataPool(dpName, sizeof(transaction_t));
	transDP = (transaction_t *)(pumpDP->LinkDataPool());

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
	delete PS2;
	delete CS2;
	delete customerData;
}

void Pump::dispense(FuelType octane, float quantity)
{
	float val = quantity;
	dispensingFlg = true;
	while (val > 0)
	{
		fuelTanks[octane]->decrement();
		val -= 0.5;
		Sleep(HALF_SECOND);
	}
	dispensingFlg = false;
}



void Pump::generateTransaction(transaction_t *trans, customer_t *cust)
{
		trans->time = time(NULL);
		trans->type = FuelType(std::rand() % 4);
		trans->quantity = (float)(std::rand() % MAX_QUANTITY) / 2 + MIN_QUANTITY;
		strcpy(trans->customer.firstName, cust->firstName);
		strcpy(trans->customer.lastName, cust->lastName);
		strcpy(trans->customer.ccNum, cust->ccNum);
}

//void Pump::Produce(Transaction *trans)

// Write data to be stored in the 'Status' datapool
void Pump::WriteStatus(transaction_t *trans)
{
	transDP->state = trans->state;
	strcpy(transDP->customer.firstName, trans->customer.firstName);
	strcpy(transDP->customer.lastName, trans->customer.lastName);
	strcpy(transDP->customer.ccNum, trans->customer.ccNum);
	transDP->time = trans->time;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}



// read the data stored in the 'Status' datapool
void Pump::ReadStatus(transaction_t *trans)
{
	trans->state = transDP->state;
	strcpy(trans->customer.firstName, transDP->customer.firstName);
	strcpy(trans->customer.lastName, transDP->customer.lastName);
	strcpy(trans->customer.ccNum, transDP->customer.ccNum);
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
	if (fuelTanks[type]->getRemaining() < quantity)
	{
		// Don't have enough gas to pump - should we wait until it's full?
		return -1;
	}
	while (quantity > 0)
	{
		fuelTanks[type]->decrement();
		quantity -= 0.5;
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

	//vector<CThread *> threads;
	vector<Pump *> pumps;

	// Create NUM_PUMPS pumps
	for (int i = 0; i < NUM_PUMPS; i++)
	{
		//CThread *thread_i = new CThread(pumpThread, ACTIVE, &i);
		//threads.push_back( thread_i );

		Pump *pump_i = new Pump(i);
		pump_i->Resume();
		pumps.push_back( pump_i );

		Sleep(100);
	}

	// Initialization Rendezvous
	Initialize.Wait();
	//ConsoleMutex.Wait();
	//MOVE_CURSOR(0, 0);
	//printf(".\n");
	//fflush(stdout);
	//ConsoleMutex.Signal();

	for (int i = 0; i < NUM_PUMPS; i++)
	{
		pumps[i]->WaitForThread();
	}

	//ConsoleMutex.Wait();
	//MOVE_CURSOR(0, 1);
	//printf(".\n");
	//fflush(stdout);
	//ConsoleMutex.Signal();

	getchar();
	return 0;
}

//void printCustomerInfo(transaction_t *trans, int id)
//{
//		ConsoleMutex.Wait();
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING);
//		printf("%d)", id);
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 1);
//		printf("%s %s", trans->customer.firstName, trans->customer.lastName);
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 2);
//		printf("%s", trans->customer.ccNum);
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 3);
//		printf("%s", states[trans->state]);
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 4);
//		printf("xxx / %5.1f", trans->quantity);
//		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 5);
//		printf("%d", trans->type);
//		ConsoleMutex.Signal();
//}
//
///**
// * This represents the communication between the pump and the GSC
// * @param
// *		A pointer to an integer with the thread number to be used to
// *		create the producer and consumer semaphores
// */
//UINT __stdcall pumpThread(void *args)
//{
//	int id = *(int*)args;
//
//	// Create the status datapool
//	std::string dpName = "PumpStatusDP" + std::to_string(id);
//	CDataPool pumpDP(dpName, sizeof(transaction_t));
//	transaction_t *transDP = (transaction_t *)(pumpDP.LinkDataPool());
//
//	// Create the Producer and Consumer Semaphores and give them a name and
//	// index aligned with this threads index/id
//	std::string producerName1 = "StatusProducer1" + std::to_string(id);
//	std::string consumerName1 = "StatusConsumer1" + std::to_string(id);
//	std::string producerName2 = "StatusProducer2" + std::to_string(id);
//	std::string consumerName2 = "StatusConsumer2" + std::to_string(id);
//	std::string pumpingName = "Pump" + std::to_string(id);
//
//	CSemaphore PS1(producerName1, 0, 1);
//	CSemaphore CS1(consumerName1, 1, 1);
//	CSemaphore PS2(producerName2, 0, 1);
//	CSemaphore CS2(consumerName2, 1, 1);
//	CSemaphore Pumping(pumpingName, 1, 1);
//	
//	// Initialization Rendezvous
//	Initialize.Wait();
//
//	for (int i = 0; i < NUM_CUSTOMERS; i++)
//	{
//		transaction trans;
//
//		// Read the transaction - ready should be false
//		PS1.Wait();
//		ReadPumpStatus(transDP, &trans);
//		assert(trans.state == ARRIVAL);
//		CS1.Signal();
//
//		// Print details to console
//		printCustomerInfo(&trans, id);
//
//		// Inform the customer they are free to start pumping
//		CS2.Wait();
//		trans.state = READY;
//		WritePumpStatus(transDP, &trans);
//		PS2.Signal();
//	}
//
//
//	return 0;
//}


//void WritePumpStatus(transaction_t *transDP, transaction *trans)
//{
//	transDP->state = trans->state;
//	strcpy(transDP->customer.firstName, trans->customer.firstName);
//	strcpy(transDP->customer.lastName, trans->customer.lastName);
//	strcpy(transDP->customer.ccNum, trans->customer.ccNum);
//	transDP->time = trans->time;
//	transDP->type = trans->type;
//	transDP->quantity = trans->quantity;
//}
//
//
//void ReadPumpStatus(transaction_t *transDP, transaction *trans)
//{
//	trans->state = transDP->state;
//	strcpy(trans->customer.firstName, transDP->customer.firstName);
//	strcpy(trans->customer.lastName, transDP->customer.lastName);
//	strcpy(trans->customer.ccNum, transDP->customer.ccNum);
//	trans->time = transDP->time;
//	trans->type = transDP->type;
//	trans->quantity = transDP->quantity;
//}


UINT __stdcall tankThread(void *args)
{
	int numBars;

	// Create the fuel tanks
	vector<FuelTank *> fuelTanks;
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}

	// Initializaion Rendezvous
	Initialize.Wait();

	// Initializaion Rendezvous
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (1)
	{
		for (int octane = OCTANE87; octane <= OCTANE94; octane++)
		{
			float remain = fuelTanks[octane]->getRemaining();

			ConsoleMutex.Wait();

			if (remain > 0.25 * (float)MAX_CAPACITY)
			{
				SetConsoleTextAttribute(hConsole, GREEN);
			}
			else
			{
				SetConsoleTextAttribute(hConsole, RED);
			}

			numBars = remain / MAX_CAPACITY * 50;

			MOVE_CURSOR(X_OFFSET, TANK_STATUS_Y_OFFSET + 4 * octane);

			for (int ii = 0; ii < 50; ii++)
			{
				if (remain > 0.5 * (float)MAX_CAPACITY)
				{
					SetConsoleTextAttribute(hConsole, GREEN);
				}
				else if (remain > 0.25 * (float)MAX_CAPACITY)
				{
					SetConsoleTextAttribute(hConsole, YELLOW);
				}
				else
				{
					SetConsoleTextAttribute(hConsole, RED);
				}

				if (ii < numBars)
				{
					printf("|");
				}
				else
				{
					printf(" ");
				}
			}
			SetConsoleTextAttribute(hConsole, WHITE);
			printf("\nFuel %d: %5.1f", octane, fuelTanks[octane]->getRemaining());
			ConsoleMutex.Signal();
		}
		Sleep(200);
	}

	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		delete fuelTanks[octane];
	}

	return 0;
}
