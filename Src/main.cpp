#include "rt/rt.h"
#include "customer.h"


int main(int argc, char* argv[])
{
	printf("\n\n\n");

	for (int i = 0; i < 15; i++)
	{
		printf("Creating new customer\n");
		Customer c;
		c.Resume();
		c.WaitForThread();

		getchar();
	}

	getchar();
	return 0;
}