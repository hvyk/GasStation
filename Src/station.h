#ifndef __STATION_H__
#define __STATION_H__

#include <string>
#include <time.h>
#include "rt\rt.h"

/*
 * This header file contains all the data structure to be used
 * for things like pipelines, datapools, etc.  Simply include this file
 * for homogeneity across all classes 
 */


#define NUM_PUMPS	4

// This is a problem

/***********************************************************
 *						Customers
 ***********************************************************/

// The number of digits in a credit card
#define CC_LENGTH 16	


/***********************************************************
 *						Fuel Tanks	
 ***********************************************************/

// Constants for the fuel type
#define NUM_FUELTYPES	4
enum FuelType { OCTANE87, OCTANE89, OCTANE91, OCTANE94 };
//char* FuelTypeNamesArray[NUM_FUELTYPES] = { "Octane 87", "Octane 89", "Octane 91", "Octane 94" };

// Capacity of the fuel tank
#define MAX_CAPACITY 500



/***********************************************************
 *						Pumps
 ***********************************************************/
//struct Transaction
//{
//	bool ready;
//	std::string name;
//	FuelType type;
//	float quantity;
//};

struct custInfo
{
	std::string firstName;
	std::string lastName;
	std::string ccNum;
	FuelType type;
	float quantity;
};

struct Transaction
{
	bool ready;
	bool pumping;
	std::string firstName;
	std::string lastName;
	std::string ccNum;
	time_t time;
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
