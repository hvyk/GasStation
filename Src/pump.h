#include <string>
#include "rt/rt.h"
#include "station.h"


class Pump : public ActiveClass
{
private:
	// The pump number/id
	int id;

	CDataPool *pumpDP;
	struct Transaction *transDP;

	CSemaphore *PS1;
	CSemaphore *CS1;

	void Produce(Transaction *trans);
	void Consume(Transaction *trans);

public:
	Pump();
	Pump(int id);
	~Pump();

	int main(void)
	{
		std::string dpName = "PumpDP" + std::to_string(id);
		CDataPool *pumpDP = new CDataPool(dpName, sizeof(Transaction));
		transDP = (struct Transaction *)(pumpDP->LinkDataPool());

		for (int i = 0; i < 2; i++)
		{
			// Simulate a simplified customer transaction	
			std::string name = "Sean";
			float quantity = (float)(std::rand() % 130) / 2;
			FuelType type = FuelType(std::rand() % 4);


			// We are the producer
			CS1->Wait();
			Transaction trans = { false, name, type, quantity };
			Produce(&trans);
			//transDP->ready = false;
			//transDP->name = name;
			//transDP->type = type;
			//transDP->quantity = quantity;
			PS1->Signal();

			printf("Pump %d Producing:\t%s, %s, %d, %1.1f\n\n",
				id,
				"false",
				name.c_str(),
				type,
				quantity);

			// Now we are the consumer
			PS1->Wait();
			Consume(&trans);
			//bool readyFlg = transDP->ready;
			//name = transDP->name;
			//type = transDP->type;
			//quantity = transDP->quantity;
			CS1->Signal();

			printf("Pump %d Consuming:\t%s, %s, %d, %1.1f\n\n",
				id,
				trans.ready? "true" : "false",
				trans.name.c_str(),
				trans.type,
				trans.quantity);
		}

		return 0;
	}

};