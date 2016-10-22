#include "rt/rt.h"
#include "customer.h"
#include "pump.h"


void Produce(struct Transaction *transDP, Transaction *trans);
void Consume(struct Transaction *transDP, Transaction *trans);


UINT __stdcall pumpThread(void *args);


// This is the gas station computer
int main(int argc, char* argv[])
{
	vector<CThread *> threads;
	printf("Creating %d threads\n", NUM_PUMPS);

	// Create NUM_PUMPS pumps
	for (int i = 0; i < NUM_PUMPS; i++)
	{
		CThread *ti = new CThread(pumpThread, ACTIVE, &i);
		threads.push_back( ti );
		Sleep(100);
	}

	printf("Done generating the threads, let them work\n");
	getchar();
	
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
	int id = *(int*)args;
	std::string dpName = "PumpDP" + std::to_string(id);
	
	CDataPool pumpDP(dpName, sizeof(Transaction));
	struct Transaction *transDP = (struct Transaction *)(pumpDP.LinkDataPool());

	std::string producerName = "Producer" + std::to_string(id);
	std::string consumerName = "Consumer" + std::to_string(id);

	CSemaphore *PS1 = new CSemaphore(producerName, 0, 1);
	CSemaphore *CS1 = new CSemaphore(consumerName, 1, 1);

	Pump pump(id);
	pump.Resume();


	for (int i = 0; i < 2; i++)
	{

		// We are the consumer
		PS1->Wait();
		Transaction trans;
		Consume(transDP, &trans);
		//bool readyFlg = transDP->ready;
		//std::string name = transDP->name;
		//FuelType type = transDP->type;
		//float quantity = transDP->quantity;
		CS1->Signal();

		printf("Reading from Pump %d as Consumer:\t%s, %s, %d, %1.1f\n\n",
			id,
			transDP->ready ? "true" : "false",
			transDP->name.c_str(),
			transDP->type,
			transDP->quantity);

		// Now we are the producer and the pump can start dispensing
		// Only setting trans.ready to true to signal to pump that its ready for use
		CS1->Wait();
		trans.ready = true;
		Produce(transDP, &trans);

		//transDP->ready = true;
		//transDP->name = name;
		//transDP->type = type;
		//transDP->quantity = quantity;
		PS1->Signal();

		printf("Writing to Pump %d as Producer:\t%s, %s, %d, %1.1f\n\n",
			id,
			trans.ready? "true" : "false",
			trans.name.c_str(),
			trans.type,
			trans.quantity);
	}

	pump.WaitForThread();

	return 0;
}


void Produce(struct Transaction *transDP, Transaction *trans)
{
	transDP->ready = trans->ready;
	transDP->name = trans->name;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


void Consume(struct Transaction *transDP, Transaction *trans)
{
	bool readyFlg = transDP->ready;
	std::string name = transDP->name;
	FuelType type = transDP->type;
	float quantity = transDP->quantity;
}

