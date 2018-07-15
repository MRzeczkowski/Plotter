#include "clientFiles/ClientLib.h"

#define PORT 5353

int main(int argc, char *argv[])
{
    /* Variable Definition */
	char* fs_name = "clientFiles/data.dat";
	char commandBuff[256];
	char plotName[256];

    if( argc == 2)
    {
        if(xfexists(argv[1]) == 0)
            error("Submitted file doesn't exist!");
        else
            fs_name = argv[1];
    }

	ConnectWithPort(PORT);

	/* Send File to Server */
    SendFileToServer(fs_name);

    char timeNow[128];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timeNow, sizeof(timeNow)-1, "[%d-%m-%Y-%H:%M]", t);

	/* Receive File from Server */
	char* fr_name = "clientFiles/PLOT";
	snprintf(plotName, sizeof(plotName), "%s%s%s", fr_name, timeNow, ".png");

    ReceiveFileFromServer(plotName);

    snprintf(commandBuff, sizeof(commandBuff), "eog %s", plotName);

	system(commandBuff);

	return (0);
}
