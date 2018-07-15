#include "../SSL_FUNCTIONS.h"

char* fr_name = "tmp/PlotterServerFiles/receive.dat"; //!< Defines the localization of 'receive.dat'
char* fs_name = "tmp/PlotterServerFiles/output.plt"; //!< Defines the localization of 'output.plt'
char* plotName = "tmp/PlotterServerFiles/output.png"; //!< Defines the localization of 'output.png'

static char noticeBuff[256]; //!< The buffer of server notices
static int isDaemon = 0; //!< Flag specifying whether server is a deamon
static int sockfd;  //!< Socket file descriptor
static int nsockfd; //!< New socket file descriptor
static int backlog; //!< Defines the maximum length to which the queue of pending connections for sockfd
static int port; //!< Server port number

static SSL_METHOD *server_ssl_method; //!< Describes the ssl library functions which implement the protocol version
static SSL_CTX *server_ssl_ctx; //!< The global context structure
static SSL *server_ssl; //!< Core structure in the SSL API 

void createDaemon(); // <---There was 'void createDemon();'---> KK.
static void demonize();
static void logServerNotice();

void BindWithPortAndRun(int BACKLOG, int PORT);

static int GetServerSocketFD();

static void ListenToRemote();
static void ObtainConnection();
static void ReceiveFileFromClient();
static void CreatePlot();
static void SendPlotToClient();
static void CloseConnection();

/** Return a socket file descriptor */
int GetServerSocketFD()
{
    int sockfd;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
	    snprintf(errBuf, sizeof(errBuf), "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
		error();
	}

	return sockfd;
}

/** Creates daemon process */
static void demonize ()
{
    pid_t pid, sid;
    int chdir_status, fd;

    if ( getppid() == 1 )
    {
        snprintf(errBuf, sizeof(errBuf), "Unable to daemonize, becouse is already daemon\n");
        error();
    }

    pid = fork();

    if (pid < 0)
    {
        snprintf(errBuf, sizeof(errBuf), "Unable to daemonize: %s - fork\n", strerror(errno));
        error();
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    sid = setsid();

    signal(SIGHUP,SIG_IGN);

    pid = fork();

    if (pid != 0)
    {
        exit(EXIT_SUCCESS);
    }

    if (sid < 0)
    {
        snprintf(errBuf, sizeof(errBuf), "Unable to create new SID: %s\n", strerror(errno));
        error();
    }

    chdir_status = chdir("/");

    if (chdir_status < 0)
    {
        snprintf(errBuf, sizeof(errBuf), "Unable to change directory to /: %s\n", strerror(errno));
        error();
    }

    fd = open("/dev/null", O_RDWR, 0);

    if (fd != -1)
    {
        dup2 (fd, STDIN_FILENO);
        dup2 (fd, STDOUT_FILENO);
        dup2 (fd, STDERR_FILENO);

        if (fd > 2)
        {
            close (fd);
        }
    }

    umask(027);
}

/** Runs server as a daemon */
void createDaemon()
{
    isDaemon = 1;
    demonize();
    mkdir("/tmp/PlotterServerFiles", 0700);
    openlog("PLOTTER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_NOTICE, "Successfully started daemon!");
    closelog();
}

/** Logs servers notices */
static void logServerNotice()
{
    printf("%s", noticeBuff);
    openlog("PLOTTER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_NOTICE, "%s%s", "[SERVER NOTICE]: ", noticeBuff);
    closelog();
}

/** Handles SIGINT, shuts down demon */
static void sigintHandler()
{
    if (isDaemon)
    {
        syslog(LOG_NOTICE, "Successfully shutdown demon");
        closelog();
    }
    else
    {
        printf("Shutting down\n");
        fflush(stdout);
    }
    exit(1);
}

/** Binds server with port */
void BindWithPortAndRun(int BACKLOG, int PORT)
{
    backlog = BACKLOG;
    port = PORT;

    signal(SIGINT, sigintHandler);

    sockfd = GetServerSocketFD();

    SSLF_InitServer(&server_ssl_method, &server_ssl_ctx);

    struct sockaddr_in addr_local;
    bzero(&(addr_local), sizeof(addr_local));

	addr_local.sin_family = AF_INET; // Protocol Family
	addr_local.sin_port = htons(PORT); // Port number
	addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
	//bzero(&(addr_local), sizeof(addr_local)); // Flush the rest of struct

	/* Bind a special Port */
	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(addr_local)) == -1 )
	{
	    snprintf(errBuf, sizeof(errBuf), "Failed to bind Port. (errno = %d)\n", errno);
		error();
	}
	else
    {
        snprintf(noticeBuff, sizeof(noticeBuff), "Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);
        logServerNotice();
        ListenToRemote();
    }
}

/** Listens to remote port */
static void ListenToRemote()
{
    if(listen(sockfd,backlog) == -1)
	{
	    snprintf(errBuf, sizeof(errBuf), "Failed to listen Port. (errno = %d)\n", errno);
		error();
	}
	else
    {
        snprintf(noticeBuff, sizeof(noticeBuff), "Listening the port %d successfully.\n", port);
        logServerNotice();

        for(;;)
        {
            ObtainConnection();

            ReceiveFileFromClient();

            CreatePlot();

            SendPlotToClient();

            CloseConnection();
        }

        SSL_CTX_free(server_ssl_ctx);
    }
}


/** Awaits connection and when one is established returns socket descriptor */
static void ObtainConnection()
{
    unsigned int sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr_remote;
    bzero(&(addr_remote), sizeof(addr_remote));

    /* Wait a connection, and obtain a new socket file despriptor for single connection */
	if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1)
	{
	    snprintf(errBuf, sizeof(errBuf), "Obtaining new Socket Despcritor. (errno = %d)\n", errno);
	    error();
	}
	else
    {
        snprintf(noticeBuff, sizeof(noticeBuff),"Got connected with: %s.\n", inet_ntoa(addr_remote.sin_addr));
        logServerNotice();

        SSLF_AcceptConnection(&server_ssl, server_ssl_ctx, nsockfd);

        logServerNotice();
    }
}

/** Receives file sent from client */
static void ReceiveFileFromClient()
{
    printf("Receving file!\n");

    SSLF_RecieveSmallAndSave(server_ssl, fr_name);

	printf("Ok received from client!\n");
}

/** Creates plot from data that the client has sent */
static void CreatePlot()
{
    int pid, status;
	FILE *fs = NULL;

    fs = xfopen (fs_name, "wb");
    fprintf (fs, "set xlabel 'x'\n"
        "set ylabel 'y'\n"
        "set grid\n"
        "set style data lines\n"
        "unset key\n"
        "set terminal png enhanced font \'Verdana, 20\'\n"
        "set output \"tmp/PlotterServerFiles/output.png\"\n"
        "plot \"%s\" lw 3 linecolor rgb \"red\"\n"
        "quit\n", fr_name);

    xfclose (fs);


    /* fill arguments array for execvp */
    char *args[] = { "gnuplot", "-p", fs_name, NULL };

    if ((pid = (fork())) < 0)
    { /* fork plot process */
        snprintf(errBuf, sizeof(errBuf), "fork() error: fork failed.\n");
        error();
    }
    else if (pid == 0)
    {    /* plot from child process */
        if (execvp (*args, args) == -1)
        {
            snprintf(errBuf, sizeof(errBuf), "execvp() error: returned error.\n");
            error();
        }
    }

    waitpid (pid, &status, 0);  /* wait for plot completion */
}

/** Sends back a gnuplot in png format to the user */
static void SendPlotToClient()
{
	printf("[Server] Sending %s to the Client...", plotName);

	SSLF_OpenAndSend(server_ssl, plotName);

	snprintf(noticeBuff, sizeof(noticeBuff), "Ok sent plot to client!\n");
	logServerNotice();
}

/** Closes a connection with Client */
static void CloseConnection()
{
    SSL_shutdown(server_ssl);

    SSL_free(server_ssl);

    close(nsockfd);

	snprintf(noticeBuff, sizeof(noticeBuff), "Connection with Client closed. Server will wait now...\n");
	logServerNotice();
}
