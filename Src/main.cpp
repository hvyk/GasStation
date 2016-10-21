#include "rt/rt.h"
#include "customer.h"
#include "pump.h"


int main(int argc, char* argv[])
{
	CDataPool pumpDP("PumpDP", sizeof(Transaction));
	struct Transaction *transDP = (struct Transaction *)(pumpDP.LinkDataPool());

	CSemaphore *PS1 = new CSemaphore("Producer", 0, 1);
	CSemaphore *CS1 = new CSemaphore("Consumer", 1, 1);

	Pump pump1;
	pump1.Resume();

	for (int i = 0; i < 20; i++)
	{
		// We are the consumer
		PS1->Wait();
		bool readyFlg = transDP->ready;
		std::string name = transDP->name;
		FuelType type = transDP->type;
		float quantity = transDP->quantity;
		CS1->Signal();

		// Now we are the producer and the pump can start dispensing
		CS1->Wait();
		transDP->ready = true;
		transDP->name = name;
		transDP->type = type;
		transDP->quantity = quantity;
		PS1->Signal();
	}

	getchar();
	return 0;
}


