#include "rt/rt.h"

struct Transaction
{
	std::string name;
	std::string ccNum;
	std::string time; // time_t to c_time to _std::string
	FuelType type;
	int quantity;
};

struct PumpStatus
{
	// The details about the transaction
	Transaction trans;
	// A flag to indicate that the transaction approved and pump is ready
	bool pumpReady;
};

//class Pump
//{
//private:
//
//public:
//
//};