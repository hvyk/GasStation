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


	// Sends the data stored in trans to the datapool given by transDP
	void Produce(Transaction *trans);
	// Receives the data stored in trans to the datapool given by transDP
	void Consume(Transaction *trans);

	// Dummy code for testing until a Customer class is created
	Transaction generateTransaction();
	std::string generateCC();
	void printTransaction(Transaction *trans, int id, bool producing);

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
			Transaction trans = generateTransaction();

			// We are the producer - want to send status info to GSC
			CS1->Wait();
			Produce(&trans);
			printTransaction(&trans, id, true);
			PS1->Signal();

			// Now we are the consumer - waiting for GSC's go-ahead on when
			// to start the pump
			PS1->Wait();
			Consume(&trans);
			printTransaction(&trans, id, false);
			CS1->Signal();
			
			// Now ready to pump the gas

		}

		return 0;
	}

};