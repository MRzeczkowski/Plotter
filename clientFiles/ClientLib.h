#include "../SSL_FUNCTIONS.h"
#include <time.h>


void SendFileToServer(char* fs_name);
void ReceiveFileFromServer(char* fr_name);
void ConnectWithPort(int PORT);
static int GetClientSocketFD();
static void CloseClientConnection();

static SSL_METHOD *client_ssl_method; //!< Describes the ssl library functions which implement the protocol version
static SSL_CTX *client_ssl_ctx; //!< The global context structure
static SSL *client_ssl; //!< Core structure in the SSL API 

static int sockfd; //!< Socket file descriptor

/** Return a socket file descriptor */
int GetClientSocketFD()
{
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
	    snprintf(errBuf, sizeof(errBuf), "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
		error();
	}

	return sockfd;
}

/** Connects with server */
void ConnectWithPort(int PORT)
{
    SSLF_InitClient(&client_ssl_method, &client_ssl_ctx, &client_ssl);

    sockfd = GetClientSocketFD();

    struct sockaddr_in remote_addr;

    bzero(&(remote_addr), sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(PORT);
	inet_aton("127.0.0.1",&remote_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == -1)
	{
	    snprintf(errBuf, sizeof(errBuf), "Failed to connect to the host! (errno = %d)\n",errno);
		error();
	}
	else
    {
        printf("Connected to server at port %d...ok!\n", PORT);

        SSLF_ConnectWithServer(client_ssl, sockfd);
    }
}

/** Sends data to the server that will be plotted */
void SendFileToServer(char* fs_name)
{
    printf("Sending %s to the Server... \n", fs_name);

	SSLF_OpenAndSend(client_ssl, fs_name);

	printf("\nOk file %s was sent!\n", fs_name);
}

/** Gets plot image form the server */
void ReceiveFileFromServer(char* fr_name)
{
    printf("Receiveing file from Server and saving it as %s... \n", fr_name);

    SSLF_RecieveAndSave(client_ssl, fr_name);

    CloseClientConnection();
}

/** Closes a connection with Server */
static void CloseClientConnection()
{
    close (sockfd);

    SSL_shutdown(client_ssl);
    SSL_free(client_ssl);
    SSL_CTX_free(client_ssl_ctx);

	printf("Connection lost.\n");
}
