#include "station.h"
#include "tank.h"

FuelTank::FuelTank()
{
	//printf("FuelTank default constructor\n");
	mRemain = new CMutex("remaining", 1);

	theDataPool = new CDataPool(string("OCTANE") + to_string(0), sizeof(struct Tank));
	tank = (struct Tank *)(theDataPool->LinkDataPool());

	mRemain->Wait();
	tank->remaining = MAX_CAPACITY;
	tank->type = OCTANE87;
	mRemain->Signal();
}

FuelTank::FuelTank(FuelType type, float remaining)
{
	//printf("FuelTank constructor %d %1.1f\n", type, remaining);
	mRemain = new CMutex("remaining", 1);

	theDataPool = new CDataPool(string("OCTANE") + to_string(type), sizeof(struct Tank));
	tank = (struct Tank *)(theDataPool->LinkDataPool());

	mRemain->Wait();
	tank->remaining = remaining;
	tank->type = OCTANE87;
	mRemain->Signal();
}

FuelTank::FuelTank(const FuelTank &obj)
{
	tank->remaining = obj.tank->remaining;
}

FuelTank::~FuelTank()
{
	delete theDataPool;
	delete mRemain;
}

float FuelTank::decrement()
{
	//while (isEmpty())
	if (isEmpty())
	{
		for (int i = 0; i < 2; i++)
		{
			// Wait for tank to be filled by operator
			Sleep(500);
		}
		fill();
	}
	float newVal;

	mRemain->Wait();
	if (tank->remaining <= 0)
	{
		tank->remaining = 0;
		return 0;
	}
	tank->remaining = tank->remaining - (float)0.5;
	newVal = tank->remaining;
	mRemain->Signal();

	return newVal;
}


float FuelTank::fill()
{
	float newVal;

	mRemain->Wait();
	tank->remaining = MAX_CAPACITY;
	newVal = tank->remaining;
	mRemain->Signal();

	return newVal;
}

float FuelTank::getRemaining()
{
	float retVal;

	mRemain->Wait();
	retVal = tank->remaining;
	mRemain->Signal();

	//printf(">>> remaining = %1.1f\n", retVal);
	return retVal;
}

FuelType FuelTank::getType()
{
	return tank->type;
}

bool FuelTank::isEmpty()
{
	if (tank->remaining <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}