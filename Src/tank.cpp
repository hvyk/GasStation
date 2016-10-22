#include "station.h"
#include "tank.h"

Tank::Tank()
{
	this->remaining = MAX_CAPACITY;
	this->type = OCTANE87;
	mRemain = new CMutex("remaining", 1);
}

//Tank::Tank(FuelType type, float remaining)
//{
//	this->remaining = remaining;
//	this->type = type;
//	mRemain = new CMutex("remaining", 1);
}

Tank::Tank(const Tank &obj)
{
	this->remaining = obj.remaining;
	this->type = obj.type;
}

Tank::~Tank()
{
	// no pointers to delete
}

float Tank::decrement()
{
	float newVal;
	
	mRemain->Wait();
	this->remaining = this->remaining - 1;
	newVal = this->remaining;
	mRemain->Signal();

	return newVal;
}


float Tank::fill()
{
	float newVal;

	mRemain->Wait();
	this->remaining = MAX_CAPACITY;
	newVal = this->remaining;
	mRemain->Signal();

	return newVal;
}

float Tank::getRemaining()
{
	float retVal;

	mRemain->Wait();
	retVal = this->remaining;
	mRemain->Signal();

	return retVal;
}

//FuelType Tank::getType()
//{
//	return type;
//}
