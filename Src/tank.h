#ifndef __TANK_H__
#define __TANK_H__

#include "rt/rt.h"


class Tank
{
private:
	// litres remaining in tank
	float remaining;
	// Type of fuel
	FuelType type;

	// Mutex to project the remaining amount of fuel
	CMutex *mRemain;

public:
	Tank();
	Tank(float remaining, FuelType type);
	Tank(const Tank &obj);
	~Tank();

	// decrements the tank by 1 litre and returns the number of litres left
	float decrement();
	// Fills the tank to full
	float fill();
	// returns the amount of fuel remaining
	float getRemaining();
	// returns the type of fuel in the tank
	FuelType getType();
};


#endif //__TANK_H__
