#include <stdio.h>
#include <vector>
#include "rt\rt.h"
#include "station.h"
#include "tank.h"

const char *GAS_STATION_PATH = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\GasStation\\Debug\\CustomerDisplay.exe";


int main(int argc, char *argv[])
{
	// Start the CustomerDisplay process
	CProcess(GAS_STATION_PATH, NORMAL_PRIORITY_CLASS, OWN_WINDOW, ACTIVE);
	
	// Wait until process is up and running yo
	Sleep(1000);

	printf("Ready to go!\n");

	//vector<FuelTank *> fuelTanks;
	//for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	//{
	//	FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
	//	fuelTanks.push_back(tank_i);
	//}


	return 0;
}