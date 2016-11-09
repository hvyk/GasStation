#include <stdio.h>
#include <vector>
#include "rt\rt.h"
#include "station.h"
#include "tank.h"

#define WHITE		7
#define RED			12
#define GREEN		10
#define YELLOW		14
#define X_OFFSET	10

const char *GAS_STATION_PATH = "C:\\Users\\Sean\\School\\Cpen333\\Assignment1\\GasStation\\Debug\\CustomerDisplay.exe";

CSemaphore ConsoleMutex("GSCConsoleMutex", 1);

int main(int argc, char *argv[])
{
	// Start the CustomerDisplay process
	CProcess(GAS_STATION_PATH, NORMAL_PRIORITY_CLASS, OWN_WINDOW, ACTIVE);
	
	// Wait until process is up and running yo
	Sleep(1000);

	// Create the fuel tanks
	vector<FuelTank *> fuelTanks;
	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		FuelTank *tank_i = new FuelTank((FuelType)octane, (float)MAX_CAPACITY);
		fuelTanks.push_back(tank_i);
	}

	// Initializaion Rendezvous
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (1)
	{
		for (int octane = OCTANE87; octane <= OCTANE94; octane++)
		{
			float remain = fuelTanks[octane]->getRemaining();
			if (remain > 0.25 * (float)MAX_CAPACITY)
			{
				SetConsoleTextAttribute(hConsole, GREEN);
			}
			else
			{
				SetConsoleTextAttribute(hConsole, RED);
			}
			int numBars = remain / MAX_CAPACITY * 50;
			ConsoleMutex.Wait();
			MOVE_CURSOR(X_OFFSET, 4 * octane);
			for (int ii = 0; ii < 50; ii++)
			{
				if (remain > 0.5 * (float)MAX_CAPACITY)
				{
					SetConsoleTextAttribute(hConsole, GREEN);
				}
				else if (remain > 0.25 * (float)MAX_CAPACITY)
				{
					SetConsoleTextAttribute(hConsole, YELLOW);
				}
				else
				{
					SetConsoleTextAttribute(hConsole, RED);
				}

				if (ii < numBars)
				{
					printf("|");
				}
				else
				{
					printf(" ");
				}
			}
			SetConsoleTextAttribute(hConsole, WHITE);
			printf("\nFuel %d: %5.1f", octane, fuelTanks[octane]->getRemaining());
			ConsoleMutex.Signal();
		}
		Sleep(200);
	}

	for (int octane = OCTANE87; octane <= OCTANE94; octane++)
	{
		delete fuelTanks[octane];
	}


	return 0;
}