#include "rt/rt.h"
#include "station.h"
#include "customer.h"
#include "pump.h"
#include "tank.h"

#define RED     12
#define GREEN   10

// Sends the data stored in trans to the 'datapool given by transDP
//void Produce(struct Transaction *transDP, Transaction *trans);
// Producer
void WritePumpStatus(struct Transaction *transDP, Transaction *trans);
// Receives the data stored in trans to the datapool given by transDP
//void Consume(struct Transaction *transDP, Transaction *trans);
// Consume
void ReadPumpStatus(struct Transaction *transDP, Transaction *trans);

// Test function to verify the transactions are right
void printTransaction(Transaction *trans);


// A thread to initialize the thread responsible for the pump
// @param
//		args: takes a pointer to an integer with the thread number which
//			  should be a number starting at 0, and incrementing by 1
//			  for every new thread spawned
UINT __stdcall pumpThread(void *args);


// This is the gas station computer
int main(int argc, char* argv[])
{
	// Create the fuel tanks
	vector<FuelTank *> fuelTanks;
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}

	vector<CThread *> threads;
	//printf("Creating %d threads\n", NUM_PUMPS);

	// Create NUM_PUMPS pumps
	for (int i = 0; i < NUM_PUMPS; i++)
	{
		CThread *thread_i = new CThread(pumpThread, ACTIVE, &i);
		threads.push_back( thread_i );
		Sleep(100);
	}

	printf("Done generating the threads, let them work\n");
	getchar();

	// Mutex for protecting the console window
	CSemaphore ConsoleMutex("ConsoleMutex", 1);

	// Application starts here
	//printf("Starting application\n");
	while (1)
	{
		ConsoleMutex.Wait();
		MOVE_CURSOR(10, 10);
		printf("%1.1f\n", fuelTanks[0]->getRemaining());
		ConsoleMutex.Signal();

		ConsoleMutex.Wait();
		MOVE_CURSOR(10, 20);
		printf("%1.1f\n", fuelTanks[1]->getRemaining());
		ConsoleMutex.Signal();

		ConsoleMutex.Wait();
		MOVE_CURSOR(10, 30);
		printf("%1.1f\n", fuelTanks[1]->getRemaining());
		ConsoleMutex.Signal();

		ConsoleMutex.Wait();
		MOVE_CURSOR(10, 40);
		printf("%1.1f\n\n", fuelTanks[1]->getRemaining());
		ConsoleMutex.Signal();

		Sleep(1000);
	}
	// Application ends here

	for (int i = 0; i < NUM_PUMPS; i++)
	{
		delete threads[i];
	}

	printf("Done\n");
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
	// Create a name for the data pool given by the index
	int id = *(int*)args;
	std::string dpName = "PumpStatusDP" + std::to_string(id);
	
	// Create the datapool 
	CDataPool pumpDP(dpName, sizeof(Transaction));
	struct Transaction *transDP = (struct Transaction *)(pumpDP.LinkDataPool());

	// Create the Producer and Consumer Semaphores and give them a name and
	// index aligned with this threads index/id
	std::string producerName = "Producer" + std::to_string(id);
	std::string consumerName = "Consumer" + std::to_string(id);

	// These shouldn't have to be pointers
	//CSemaphore *PS1 = new CSemaphore(producerName, 0, 1);
	//CSemaphore *CS1 = new CSemaphore(consumerName, 1, 1);
	CSemaphore PS1(producerName, 0, 1);
	CSemaphore CS1(consumerName, 1, 1);

	// Spawn a pump active object
	Pump pump(id);
	pump.Resume();

	// probably want to set the rendezvous here

	// Console Mutex for GSC
	CSemaphore ConsoleMutex("ConsoleMutex", 1);

	// why a for loop??? Is it just the number of customers we want?
	for (int i = 0; i < 2; i++)
	{
		// We are the consumer - read the data stored in the data pool and
		PS1.Wait();
		Transaction trans;
		ReadPumpStatus(transDP, &trans);
		CS1.Signal();

		ConsoleMutex.Wait();
		MOVE_CURSOR(10, id * 10 + 1);
		printf("%s %s", trans.firstName, trans.lastName);
		MOVE_CURSOR(10, id * 10 + 2);
		printf("OCTANE 4 Million");
		MOVE_CURSOR(10, id * 10 + 3);
		printf("$1");
		MOVE_CURSOR(10, id * 10 + 4);
		printf("42/406 Litres");
		MOVE_CURSOR(10, id * 10 + 5);
		printf("$401");
		ConsoleMutex.Signal();

		// For reassurance
		//printf("Reading from Pump %d as Consumer\n", id);
		//printTransaction(&trans);

		// Now we are the producer - The pump can start dispensing gas once
		// the flag trans.ready is set to true and sent back to the datapool
		// Only setting trans.ready to true to signal to pump that its ready for use
		CS1.Wait();
		trans.ready = true;
		WritePumpStatus(transDP, &trans);
		PS1.Signal();

		// For reassurance
		//printf("Writing to Pump %d as Producer\n", id);
		//printTransaction(&trans);
	}

	pump.WaitForThread();

	return 0;
}


void WritePumpStatus(struct Transaction *transDP, Transaction *trans)
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


void ReadPumpStatus(struct Transaction *transDP, Transaction *trans)
{
	trans->ready = transDP->ready;
	trans->pumping = transDP->pumping;
	trans->firstName = transDP->firstName;
	trans->lastName = transDP->lastName;
	trans->ccNum = transDP->ccNum;
	trans->time = transDP->time;
	trans->type = transDP->type;
	trans->quantity = transDP->quantity;
}

void printTransaction(Transaction *trans)
{
	std::string fType;
	switch (trans->type)
	{
	case OCTANE87:
		fType = "OCTANE87";
		break;
	case OCTANE89:
		fType = "OCTANE89";
		break;
	case OCTANE91:
		fType = "OCTANE91";
		break;
	case OCTANE94:
		fType = "OCTANE94";
		break;
	}

	std::string transTime = ctime(&(trans->time));

	printf("\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%1.1f",
		trans->firstName, trans->lastName, trans->ccNum,
		time, fType, trans->quantity);
	printf("\t%s\n", trans->firstName);
	printf("\t%s\n", trans->lastName);
	printf("\t%s\n", trans->ccNum);
	printf("\t%s\n", transTime);
	printf("\t%s\n", fType);
	printf("\t%1.1f\n", trans->quantity);
}