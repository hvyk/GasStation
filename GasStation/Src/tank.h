#ifndef __TANK_H__
#define __TANK_H__

//#include "rt/rt.h"
//#include "station.h"


class FuelTank
{
private:
	// The Tank datapool
	struct Tank {
		float remaining;
		FuelType type;
	};

	CDataPool *theDataPool;
	struct Tank *tank;

	// Mutex to project the remaining amount of fuel
	CMutex *mRemain;

public:
	FuelTank();
	FuelTank(FuelType type, float remaining);
	FuelTank(const FuelTank &obj);
	~FuelTank();

	// decrements the tank by 1 litre and returns the number of litres left
	float decrement();
	// Fills the tank to full
	float fill();
	// returns the amount of fuel remaining
	float getRemaining();
	// returns the type of fuel in the tank
	FuelType getType();

	bool isEmpty();
};

#endif //__TANK_H__
