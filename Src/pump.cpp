#include "pump.h"

Pump::Pump()
{
	//CDataPool pumpDP("PumpDP", sizeof(Transaction));
	//transDP = (struct Transaction *)(pumpDP.LinkDataPool());

	PS1 = new CSemaphore("Producer", 0, 1);
	CS1 = new CSemaphore("Consumer", 1, 1);
}

Pump::~Pump()
{
	delete PS1;
	delete CS1;
}