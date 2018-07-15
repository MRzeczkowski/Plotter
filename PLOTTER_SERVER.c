#include "serverFiles/ServerLib.h"

#define PORT 5353
#define BACKLOG 5

int main (int argc, char * argv[])
{
	char * opts = "d";
    int c;

	if (argc == 2)
    {
        while ((c = getopt(argc, argv, opts)) != -1)
        {
            switch (c)
            {
                case 'd':
                    createDaemon();
                    break;
                case '?':
                default:
                    printf("Ignore passed options\n");
            }
        }
    }

	BindWithPortAndRun(BACKLOG, PORT);
}
