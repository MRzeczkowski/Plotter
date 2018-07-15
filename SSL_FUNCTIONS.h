#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ProjectLib.h"

/* SERVER **/
void SSLF_InitServer(SSL_METHOD **server_ssl_method, SSL_CTX **server_ssl_ctx);
void SSLF_AcceptConnection(SSL **server_ssl, SSL_CTX *server_ssl_ctx, int nsockfd);

/* CLIENT **/
void SSLF_InitClient(SSL_METHOD **client_ssl_method, SSL_CTX **client_ssl_ctx, SSL **client_ssl);

/* SERVER **/

/** Set up SSL for server */
void SSLF_InitServer(SSL_METHOD **server_ssl_method, SSL_CTX **server_ssl_ctx)
{
    OpenSSL_add_all_algorithms();
    SSL_library_init();
    SSL_load_error_strings();
    *server_ssl_method = TLSv1_server_method();

    if((*server_ssl_ctx = SSL_CTX_new(*server_ssl_method)) == NULL)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_InitServer] SSL_CTX_new failed! (errno = %d)\n",errno);
		error();
    }

    SSL_CTX_use_certificate_file(*server_ssl_ctx,"cert.pem",SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(*server_ssl_ctx,"key.pem",SSL_FILETYPE_PEM);

    if(!SSL_CTX_check_private_key(*server_ssl_ctx))
    {
        snprintf(errBuf, sizeof(errBuf), "[SSLF_InitServer] Private key does not match certificate\n");
		error();
    }
}

/** Connects server with a socket file descriptor */
void SSLF_AcceptConnection(SSL **server_ssl, SSL_CTX *server_ssl_ctx, int nsockfd)
{
    if((*server_ssl = SSL_new(server_ssl_ctx)) == NULL)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_AcceptConnection] SSL_new failed! (errno = %d)\n",errno);
		error();
    }

    SSL_set_fd(*server_ssl,nsockfd);


    if(SSL_accept(*server_ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_AcceptConnection] SSL_accept failed! (errno = %d)\n",errno);
		error();
    }
}

/* CLIENT **/

/** Sets up SSL for client */
void SSLF_InitClient(SSL_METHOD **client_ssl_method, SSL_CTX **client_ssl_ctx, SSL **client_ssl)
{
    OpenSSL_add_all_algorithms();
    SSL_library_init();
    SSL_load_error_strings();
    *client_ssl_method = TLSv1_client_method();

    if((*client_ssl_ctx = SSL_CTX_new(*client_ssl_method)) == NULL)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_InitClient] SSL_CTX_new failed! (errno = %d)\n",errno);
		error();
    }

    if((*client_ssl = SSL_new(*client_ssl_ctx)) == NULL)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_InitClient] SSL_new failed! (errno = %d)\n",errno);
		error();
    }

}

/** Connects client to server */
void SSLF_ConnectWithServer(SSL *client_ssl, int sockfd)
{
    SSL_set_fd(client_ssl,sockfd);
    if(SSL_connect(client_ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        snprintf(errBuf, sizeof(errBuf), "[SSLF_ConnectWithServer] SSL_connect failed! (errno = %d)\n",errno);
		error();
    }
}

/* GENERAL **/

/** Reads recived bytes (larger files) and save them */
void SSLF_RecieveBigAndSave(SSL *source, char *SaveAs)
{
    FILE *sink = xfopen(SaveAs, "wb");

    char revbuf[LENGTH];
    bzero(revbuf, LENGTH);
    int block_sz = 0;

    do
    {
        block_sz = SSL_read(source, revbuf, sizeof(revbuf));

        int write_sz = fwrite(revbuf, sizeof(char), block_sz, sink);


    }while(block_sz > 0);

    fclose(sink);
}

/** Reads recived bytes and save them */
void SSLF_RecieveSmallAndSave(SSL *source, char *SaveAs)
{
    FILE *sink = xfopen(SaveAs, "wb");

    char revbuf[LENGTH];
    bzero(revbuf, LENGTH);
    int block_sz = 0;

    do
    {
        if(block_sz > 0)
            break;

        block_sz = SSL_read(source, revbuf, sizeof(revbuf));

        int write_sz = fwrite(revbuf, sizeof(char), block_sz, sink);


    }while(block_sz > 0);

    fclose(sink);
}

/** Sends file */
void SSLF_OpenAndSend(SSL *sink, char *FromFile)
{
    FILE *source = xfopen(FromFile, "r");

    char revbuf[LENGTH];
    bzero(revbuf, LENGTH);
    int block_sz = 0;

    do
	{
	    block_sz = fread(revbuf, sizeof(char), LENGTH, source);

        SSL_write(sink,revbuf,block_sz);

	}while(block_sz > 0);

    fclose(source);
}
