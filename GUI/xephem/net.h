#ifndef _NET_H
#define _NET_H

#ifdef VMS
#include <types.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#if !defined(FD_ZERO) || defined(_AIX)
/* one last place to look for select() stuff */
#include <sys/select.h>
#endif

#include <openssl/ssl.h>

typedef struct {
	int fd;		//file desciptor for the underlying connection socket
	SSL *ssl;	//ssl connection for use with SSL_read( )and SSL_write()
} XE_SSL_FD;

/* support functions */

extern int httpGET (char *host, char *GETcmd, char msg[]);
extern int mkconnection (char *host, int port, char msg[]);
extern int recvbytes (int fd, unsigned char buf[], int n);
extern int readbytes (int fd, unsigned char buf[], int n);
extern int recvline (int fd, char buf[], int max);
extern int recvlineb (int sock, char *buf, int size);
extern int sendbytes (int fd, unsigned char buf[], int n);
extern int httpsGET (char *host, char *GETcmd, char msg[], XE_SSL_FD *ssl_fd);
extern int ssl_recvbytes (XE_SSL_FD *ssl_fd, unsigned char buf[], int n);
extern int ssl_readbytes (XE_SSL_FD *ssl_fd, unsigned char buf[], int n);
extern int ssl_recvline (XE_SSL_FD *ssl_fd, char buf[], int max);
extern int ssl_recvlineb (XE_SSL_FD *ssl_fd, char *buf, int size);

#endif /* _NET_H */
