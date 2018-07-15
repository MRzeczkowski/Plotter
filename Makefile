all: PLOTTER_CLIENT.o PLOTTER_SERVER.o

PLOTTER_CLIENT.o: PLOTTER_CLIENT.c
	gcc -I/usr/include/openssl/ -o PLOTTER_CLIENT PLOTTER_CLIENT.c -lcrypto -lssl

PLOTTER_SERVER.o: PLOTTER_SERVER.c
	gcc -I/usr/include/openssl/ -o PLOTTER_SERVER PLOTTER_SERVER.c -lcrypto -lssl
