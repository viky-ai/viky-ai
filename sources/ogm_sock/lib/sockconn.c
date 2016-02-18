/*
 *  connect with a timeout, based on PHP4 encoding (php-4.3.5RC3/main/network.c)
 *  Copyright (c) 2004 Pertimm
 *  Dev : February 2004
 *  Version 1.0
*/
#include "ogm_sock.h"



#if (DPcSystem == DPcSystemUnix)


int php_connect_nonb(int sockfd,
						const struct sockaddr *addr,
						int addrlen,
						struct timeval *timeout,
                        int *timed_out,
                        struct og_call_socket_stat *st
                        )
{
	/* probably won't work on Win32, someone else might try it (read: fix it ;) */

#if (!defined(__BEOS__)) && (defined(O_NONBLOCK) || defined(O_NDELAY))

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

	int flags;
	int n;
	int error = 0;
	socklen_t len;
	int ret = 0;
	fd_set rset;
	fd_set wset;
	fd_set eset;
    int ogclock,clock_start=0,overall_clock_start;

    if (timed_out) *timed_out=0;
	if (timeout == NULL)	{
		/* blocking mode */
		return connect(sockfd, addr, addrlen);
	}

    IFx(st) {
      overall_clock_start = clock_start = OgClock();
      }
	
	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    IFx(st) {
      ogclock=OgClock(); st->connect_set_non_block = ogclock-clock_start; clock_start=ogclock;
      }

	if ((n = connect(sockfd, addr, addrlen)) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}

    IFx(st) {
      ogclock=OgClock(); st->connect_connect += ogclock-clock_start; clock_start=ogclock;
      st->nb_connect++;
      }
 
	if (n == 0) {
		goto ok;
	}

//#ifdef __linux__
#if (DPcArch == DPcArchlinux)
retry_again:
#endif
	
	FD_ZERO(&rset);
	FD_ZERO(&eset);
	FD_SET(sockfd, &rset);
	FD_SET(sockfd, &eset);

	wset = rset;

	if ((n = select(sockfd + 1, &rset, &wset, &eset, timeout)) == 0) {
        IFx(st) {
          ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
	      }
        if (timed_out) *timed_out=1;
    	error = ETIMEDOUT;

	} else if ((FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))) {
        IFx(st) {
          ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
          }
		len = sizeof(error);
		/*
		   BSD-derived systems set errno correctly
		   Solaris returns -1 from getsockopt in case of error
		   */
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			ret = -1;
		}
        IFx(st) {
          ogclock=OgClock(); st->connect_getsockopt = ogclock-clock_start; clock_start=ogclock;
          }

	} else {
        IFx(st) {
          ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
          }
		/* whoops: sockfd has disappeared */
		ret = -1;
		error = errno;
	}

//#ifdef __linux__
#if (DPcArch == DPcArchlinux)
	/* this is a linux specific hack that only works since linux updates
	 * the timeout struct to reflect the time remaining from the original
	 * timeout value.  One day, we should record the start time and calculate
	 * the remaining time ourselves for portability */
	if (ret == -1 && error == EINPROGRESS) {
		error = 0;
		goto retry_again;
	}
#endif
	
ok:
	fcntl(sockfd, F_SETFL, flags);
    IFx(st) {
      ogclock=OgClock(); st->connect_set_block = ogclock-clock_start; clock_start=ogclock;
      }

	if (error) {
		errno = error;
		ret = -1;
	}
	return ret;
#else 
	return connect(sockfd, addr, addrlen);
#endif
}
/* }}} */


#else
#if (DPcSystem == DPcSystemWin32)



int php_connect_nonb(int sockfd,
						const struct sockaddr *addr,
						int addrlen,
						struct timeval *timeout,
                        int *timed_out,
                        struct og_call_socket_stat *st
                        )
{
	int error = 0, error_len, ret;
	u_long non_block = TRUE, block = FALSE;
    int ogclock,clock_start,overall_clock_start;

	fd_set rset, wset;

    if (timed_out) *timed_out=0;
	if (timeout == NULL)	{
		/* blocking mode */
		return connect(sockfd, addr, addrlen);
	}

    IFx(st) {
      overall_clock_start = clock_start = OgClock();
      }
	
	/* Set the socket to be non-blocking */
	ioctlsocket(sockfd, FIONBIO, &non_block);

    IFx(st) {
      ogclock=OgClock(); st->connect_set_non_block = ogclock-clock_start; clock_start=ogclock;
      }

	if (connect(sockfd, addr, addrlen) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			return SOCKET_ERROR;
		}
	}

    IFx(st) {
      ogclock=OgClock(); st->connect_connect += ogclock-clock_start; clock_start=ogclock;
      st->nb_connect++;
      }

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);

	FD_ZERO(&wset);
	FD_SET(sockfd, &wset);

	if ((ret = select(sockfd + 1, &rset, &wset, NULL, timeout)) == 0) {
        IFx(st) {
          ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
          }
        if (timed_out) *timed_out=1;
		WSASetLastError(WSAETIMEDOUT);
		return SOCKET_ERROR;
	}

	if (ret == SOCKET_ERROR) {
        ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
		return SOCKET_ERROR;
	}

    IFx(st) {
      ogclock=OgClock(); st->connect_select = ogclock-clock_start; clock_start=ogclock;
      }

	if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		error_len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *) &error, &error_len) == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
        IFx(st) {
          ogclock=OgClock(); st->connect_getsockopt = ogclock-clock_start; clock_start=ogclock;
          }
	} else {
		/* whoops: sockfd has disappeared */
		return SOCKET_ERROR;
	}

	/* Set the socket back to blocking */
	ioctlsocket(sockfd, FIONBIO, &block);
    IFx(st) {
      ogclock=OgClock(); st->connect_set_block = ogclock-clock_start; clock_start=ogclock;
      }

	if (error) { 
		WSASetLastError(error);
		return SOCKET_ERROR;
	}

	return 0;
}

#endif
#endif








