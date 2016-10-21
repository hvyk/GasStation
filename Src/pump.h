#include <string>
#include "rt/rt.h"
#include "station.h"


class Pump : public ActiveClass
{
private:
	struct Transaction *transDP;
	CSemaphore *PS1;
	CSemaphore *CS1;

public:
	Pump();
	//Pump();
	~Pump();

	int main(void)
	{
		CDataPool pumpDP("PumpDP", sizeof(Transaction));
		transDP = (struct Transaction *)(pumpDP.LinkDataPool());

		for (int i = 0; i < 20; i++)
		{
			// Simulate a simplified customer transaction	
			std::string name = "Sean";
			float quantity = (float)(std::rand() % 130) / 2;
			FuelType type = FuelType(std::rand() % 4);

			// We are the producer
			CS1->Wait();
			transDP->ready = false;
			transDP->name = name;
			transDP->type = type;
			transDP->quantity = quantity;
			PS1->Signal();

			// Now we are the consumer
			PS1->Wait();
			bool readyFlg = transDP->ready;
			name = transDP->name;
			type = transDP->type;
			quantity = transDP->quantity;
			CS1->Signal();

			//printf("%s, %s, %d, %1.1f/n",
			//	readyFlg ? "true" : "false",
			//	name.c_str(),
			//	type,
			//	quantity);
		}

		return 0;
	}

};