#include "pump.h"

// Baaad, don't use this constructor for more than one pump
Pump::Pump()
{
	//CDataPool pumpDP("PumpDP", sizeof(Transaction));
	//transDP = (struct Transaction *)(pumpDP.LinkDataPool());
	id = 0;

	PS1 = new CSemaphore("Producer0", 0, 1);
	CS1 = new CSemaphore("Consumer0", 1, 1);
}


Pump::Pump(int id)
{
	//CDataPool pumpDP("PumpDP", sizeof(Transaction));
	//transDP = (struct Transaction *)(pumpDP.LinkDataPool());
	this->id = id;

	std::string producerName = "Producer" + this->id;
	std::string consumerName = "Consumer" + this->id;

	PS1 = new CSemaphore(producerName, 0, 1);
	CS1 = new CSemaphore(consumerName, 1, 1);
}


Pump::~Pump()
{
	delete PS1;
	delete CS1;
}


void Pump::Produce(Transaction *trans)
{
	transDP->ready = trans->ready;
	transDP->name = trans->name;
	transDP->type = trans->type;
	transDP->quantity = trans->quantity;
}


void Pump::Consume(Transaction *trans)
{
	trans->ready = transDP->ready;
	trans->name = transDP->name;
	trans->type	= transDP->type;
	trans->quantity = transDP->quantity;
}