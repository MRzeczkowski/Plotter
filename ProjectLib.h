#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LENGTH 8192 //!< Maximum length of sent file
char errBuf[256]; //!< The buffer of server errors

void error();
FILE *xfopen (const char *fn, const char *mode);
int badmode (const char *s);
int xfclose (FILE *fp);
int xfexists (char *fn);
char *fnwoext (char *nm, char *fn);

/** Error reporting function, reports error message that is in errBuff */
void error()
{
    openlog("PLOTTER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "%s%s", "[ERROR]: ",errBuf);
    closelog();

	perror(errBuf);
	exit(1);
}

/** fopen with error checking - short version */
FILE *xfopen (const char *fn, const char *mode)
{
    if (!fn || !mode || badmode (mode)) {
        fprintf (stderr, "xfopen() error: invalid parameter.\n");
        exit (EXIT_FAILURE);
    }
    FILE *fp = fopen (fn, mode);

    if (!fp) {
        snprintf(errBuf, sizeof(errBuf), "xfopen() error: file open failed '%s'.\n", fn);
        error();
    }

    return fp;
}

/** Validates file mode 's' is "rwa+b" */
int badmode (const char *s)
{
    const char *modes = "rwa+b";

    for (; *s; s++) {
        const char *m = modes;
        int valid = 0;
        while (*m) if (*s == *m++) { valid = 1; break; }
        if (!valid) return *s;
    }
    return 0;
}

/** file close with error check */
int xfclose (FILE *fp)
{
    if (fclose (fp))
    {
        snprintf(errBuf, sizeof(errBuf), "xfclose() error: nonzero return on fclose.\n");
        error();
    }
    return 0;
}

/** Checks if file 'fn' already exists */
int xfexists (char *fn)
{
    /* if access return is not -1 file exists */
    if (access (fn, F_OK ) != -1 )
        return 1;

    return 0;
}

/** Isolates filename, without path or extension */
char *fnwoext (char *nm, char *fn)
{
    char *p  = NULL, *ep = NULL;
    char fnm[256] = "";

    if (!fn) return NULL;
    strcpy (fnm, fn);
    if ((p = strrchr (fnm, '/')))
        p++;
    else
        p = fnm;

    if ((ep = strrchr (p, '.'))) {
        *ep = 0;
        strcpy (nm, p);
        *ep = '.';
    } else
        strcpy (nm, p);

    return nm;
}
