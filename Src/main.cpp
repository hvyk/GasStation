#include "rt/rt.h"
#include "customer.h"
#include "pump.h"


UINT __stdcall spawnCustomerThreads(void *args);

int main(int argc, char* argv[])
{
	//for (int i = 0; i < 25; i++)
	//{
		printf("Creating new customers\n");
		CThread spawnThread(spawnCustomerThreads, ACTIVE, NULL);
		spawnThread.Resume();

		Sleep(2000);


		//printf("Creating a pump\n");
		//Pump pump1;
		//pump1.Resume();

		//pump1.WaitForThread();
		//getchar();
	//}


	getchar();
	return 0;
}


UINT __stdcall spawnCustomerThreads(void *args)
{

	Pump pump1;
	pump1.Resume();

	for (int i = 0; i < 20; i++)
	{
		printf("spawning new customer...\n");
		Customer c;
		int x = c.Resume();
	}

	pump1.WaitForThread();
	return 0;
}