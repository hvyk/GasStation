#ifndef __STATION_H__
#define __STATION_H__

#include <string>

/*
 * This header file contains all the data structure to be used
 * for things like pipelines, datapools, etc.  Simply include this file
 * for homogeneity across all classes 
 */



/***********************************************************
 *						Customers
 ***********************************************************/

// The number of digits in a credit card
#define CC_LENGTH 16	


/***********************************************************
 *						Fuel Tanks	
 ***********************************************************/

// Constants for the fuel type
enum FuelType { OCTANE87, OCTANE89, OCTANE91, OCTANE94 };

// Capacity of the fuel tank
#define MAX_CAPACITY 500



/***********************************************************
 *						Pumps
 ***********************************************************/
struct Transaction
{
	std::string name;
	std::string ccNum;
	std::string time; // time_t to c_time to _std::string
	FuelType type;
	float quantity;
};

struct PumpStatus
{
	// The details about the transaction
	Transaction trans;
	// A flag to indicate that the transaction approved and pump is ready
	bool pumpReady;
};


#endif // __STATION_H__
