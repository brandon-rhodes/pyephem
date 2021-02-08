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

/* support functions */

extern int httpGET (char *host, char *GETcmd, char msg[]);
extern int mkconnection (char *host, int port, char msg[]);
extern int recvbytes (int fd, unsigned char buf[], int n);
extern int readbytes (int fd, unsigned char buf[], int n);
extern int recvline (int fd, char buf[], int max);
extern int recvlineb (int sock, char *buf, int size);
extern int sendbytes (int fd, unsigned char buf[], int n);



/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: net.h,v $ $Date: 2003/03/17 07:26:21 $ $Revision: 1.3 $ $Name:  $
 */

#endif /* _NET_H */
