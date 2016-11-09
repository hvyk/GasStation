#include <stdio.h>
#include <vector>
#include <cassert>
#include "rt\rt.h"
#include "station.h"
#include "tank.h"

const char *GAS_STATION_PATH = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\GasStation\\Debug\\CustomerDisplay.exe";

CSemaphore ConsoleMutex("GSCConsoleMutex", 1);

const int numRendezvous = NUM_PUMPS + 1;
CRendezvous Initialize("GSCInitRendezvous", numRendezvous );
CRendezvous ThreadInit("ThreadInit", 2 );

char *states[4] = { "Arrived", "Ready", "Pumping", "Complete" };

void WritePumpStatus(transaction_t *transDP, transaction *trans);
void ReadPumpStatus(transaction_t *transDP, transaction *trans);


UINT __stdcall pumpThread(void *args);

int main(int argc, char *argv[])
{
	//printf("Starting child process\n");
	// Start the CustomerDisplay process
	CProcess GSCProcess(GAS_STATION_PATH, NORMAL_PRIORITY_CLASS, OWN_WINDOW, ACTIVE);
	
	// Wait until process is up and running yo
	Sleep(1000);

	vector< CThread *> threads;

	//printf("spawing %d threads\n", numRendezvous);
	// Create NUM_PUMPS pumps
	for (int i = 0; i < NUM_PUMPS; i++)
	{
		CThread *thread_i = new CThread(pumpThread, ACTIVE, &i);
		threads.push_back( thread_i );
		//Sleep(100);
		ThreadInit.Wait();
	}
	Initialize.Wait();

	for (int i = 0; i < NUM_PUMPS; i++)
	{
		threads[i]->WaitForThread();
	}

	GSCProcess.WaitForProcess();

	return 0;
}


void printCustomerInfo(transaction_t *trans, int id)
{
		ConsoleMutex.Wait();
		MOVE_CURSOR(X_SPACING, id * Y_SPACING);
		printf("%d)", id);
		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 1);
		printf("%s %s", trans->customer.firstName, trans->customer.lastName);
		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 2);
		printf("%s", trans->customer.ccNum);
		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 3);
		printf("%s", states[trans->state]);
		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 4);
		printf("xxx / %5.1f", trans->quantity);
		MOVE_CURSOR(X_SPACING, id * Y_SPACING + 5);
		printf("%d", trans->type);
		ConsoleMutex.Signal();
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
	ThreadInit.Wait();
	//printf("id: %d\n", id);

	// Create the status datapool
	std::string dpName = "PumpStatusDP" + std::to_string(id);
	CDataPool pumpDP(dpName, sizeof(transaction_t));
	transaction_t *transDP = (transaction_t *)(pumpDP.LinkDataPool());

	// Create the Producer and Consumer Semaphores and give them a name and
	// index aligned with this threads index/id
	std::string producerName1 = "StatusProducer1" + std::to_string(id);
	std::string consumerName1 = "StatusConsumer1" + std::to_string(id);
	std::string producerName2 = "StatusProducer2" + std::to_string(id);
	std::string consumerName2 = "StatusConsumer2" + std::to_string(id);
	std::string pumpingName = "Pump" + std::to_string(id);


	//printf("%s\n%s\n%s\n%s\n",
	//	producerName1.c_str(),
	//	consumerName1.c_str(),
	//	producerName2.c_str(),
	//	consumerName2.c_str()
	//);

	CSemaphore PS1(producerName1, 0, 1);
	CSemaphore CS1(consumerName1, 1, 1);
	CSemaphore PS2(producerName2, 0, 1);
	CSemaphore CS2(consumerName2, 1, 1);
	CSemaphore Pumping(pumpingName, 1, 1);
	
	// Initialization Rendezvous
	Initialize.Wait();


	for (int i = 0; i < NUM_CUSTOMERS; i++)
	{
		transaction trans;

		// Read the transaction - ready should be false
		PS1.Wait();
		//printf("Display Consuming\n");
		ReadPumpStatus(transDP, &trans);
		assert(trans.state == ARRIVAL);
		CS1.Signal();

		// Print details to console
		printCustomerInfo(&trans, id);

		// Inform the customer they are free to start pumping
		CS2.Wait();
		//printf("Display Producing\n");
		trans.state = READY;
		WritePumpStatus(transDP, &trans);
		PS2.Signal();
		Sleep(200);
	}


	return 0;
}

void WritePumpStatus(transaction_t *transDP, transaction *trans)
{
	transDP->state = trans->state;
	strcpy(transDP->customer.firstName, trans->customer.firstName);
	strcpy(transDP->customer.lastName, trans->customer.lastName);
	strcpy(transDP->customer.ccNum, trans->customer.ccNum);
	transDP->time = trans->time;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


void ReadPumpStatus(transaction_t *transDP, transaction *trans)
{
	trans->state = transDP->state;
	strcpy(trans->customer.firstName, transDP->customer.firstName);
	strcpy(trans->customer.lastName, transDP->customer.lastName);
	strcpy(trans->customer.ccNum, transDP->customer.ccNum);
	trans->time = transDP->time;
	trans->type = transDP->type;
	trans->quantity = transDP->quantity;
}

