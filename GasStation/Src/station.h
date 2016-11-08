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

#define MAX_NAME_LEN	50
#define CC_NUM_LEN		21
#define NUM_CUSTOMERS	100

#define NUM_PUMPS		4

#define MIN_QUANTITY	3
#define MAX_QUANTITY	8

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

typedef struct customer
{
	char firstName[MAX_NAME_LEN];
	char lastName[MAX_NAME_LEN];
	char ccNum[CC_NUM_LEN];
	//FuelType type;
	//float quantity;
} customer_t;

// 0 - arrival
// 1 - ready to start pumping
// 2 - pumping
// 3 - finished pumping, goodbye customer
// int state;

#define ACK			0
#define ARRIVAL		1
#define READY		2
#define PUMPING		3
#define	COMPLETE	4
typedef struct transaction
{
	int state;
	customer_t customer;
	//char firstName[MAX_NAME_LEN];
	//char lastName[MAX_NAME_LEN];
	//char ccNum[CC_NUM_LEN];
	time_t time;
	FuelType type;
	float quantity;
} transaction_t;

struct PumpStatus
{
	// The details about the transaction
	transaction trans;
	// A flag to indicate that the transaction approved and pump is ready
	bool pumpReady;
};



#endif // __STATION_H__
