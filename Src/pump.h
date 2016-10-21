#include "rt/rt.h"
#include "station.h"


class Pump : public ActiveClass
{
private:

public:
	int main(void)
	{
			CPipe pipe("testPipe", 1024);
		for (int i = 0; i < 20; i++)
		{
			int x;
			printf("starting pump\n");
			//for (int i = 0; i < 24; i++)
			//while (true)
			//{
				//CPipe pipe1("transactionPipe", 1024);
				//Transaction trans;
				//pipe1.Read(&trans, sizeof(trans));

				//printf("\n\nTransaction: %s, %s, %s, %d, %1.1f\n", trans.name.c_str(), trans.ccNum.c_str(), trans.time.c_str(), trans.type, trans.quantity);
			//}

			pipe.Read(&x, sizeof(x));

			printf("the value for x was %d\n", x);
		}
		getchar();

		return 0;
	}

};