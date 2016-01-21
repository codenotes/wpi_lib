 /* IOLib-ObjC, Universal IO Library
  * Copyright (C) 1998-2002 Robert Kausch <robert.kausch@gmx.net>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public
  * License along with this library; if not, write to the Free
  * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  * MA 02111-1307, USA */

#ifndef __IOLIB_OUTSTREAM_
#define __IOLIB_OUTSTREAM_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <iolib-objc.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef O_RANDOM
	#define O_RANDOM 0
#endif

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

@implementation OutStream

- free
{
	if (crosslinked)
	{
		instream->crosslinked	= false;
		instream->outstream	= NULL;
	}

	[self Close];	// Datei schlieﬂen (return egal)

	return [super free];
}

- (void) Connect: (int) type, ...
{
#if defined __WIN32__ && !defined __CYGWIN32__
	WORD			 wVersionRequested = MAKEWORD(1,1);
	WSADATA			 wsaData;
#endif

#ifndef MSDOS
	struct hostent		*host;
	struct hostent		*server_hostent;
	struct sockaddr_in	 saddr;
#endif

	char			*filename;
	char			*server;
	char			*proxy;
	char			*uname = NULL;
	char			*passwd = NULL;
	char			*remote_host = NULL;
	unsigned char		*socksdata;
	unsigned short		*remote_port = NULL;
	int			 mode;
	unsigned short		 port;
	unsigned short		 socksport;
	int			 file;
	int			 subtype;
	FILE			*openfile;
	void			*outbuffer;
	long			 bufsize;
	OutStream		*outstr;
	InStream		*instr;
	va_list			 ap;
	int			 i;
	int			 value;
	int			 counter;
	int			 recbytes = 0;
	int			 neededbytes;
	SOCKET			 opensocket;

	switch (type)
	{
		case STREAM_FILE:
			va_start(ap, type);

			filename	= va_arg(ap, char *);
			mode		= va_arg(ap, int);

			va_end(ap);

			streamtype	= STREAM_FILE;
			lastfilepos	= 0;
			data		= malloc(packagesize);

			if (mode == OS_APPEND)
			{
				stream = open(filename, O_RDWR | O_BINARY | O_RANDOM | O_CREAT, 0600);

				if (stream <= 0)
				{
					free(data);
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}

				lseek(stream, 0, SEEK_END);

				size		= tell(stream);
				currentfilepos	= size;
				lastfilepos	= size;
			}
			else
			{
				stream = open(filename, O_RDWR | O_BINARY | O_RANDOM | O_CREAT | O_TRUNC, 0600);

				if (stream <= 0)
				{
					free(data);
					streamtype = STREAM_NONE;
					return;
				}
			}

			isopen = true;

			return;
#ifndef MSDOS
		case STREAM_CLIENT:
			va_start(ap, type);

			server	= va_arg(ap, char *);
			port	= va_arg(ap, int);

			va_end(ap);

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}
			}

			initialized++;
#endif

			streamtype	= STREAM_CLIENT;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;

			mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (mysocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			host = gethostbyname(server);

			if (host == NULL)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) port);

			if (connect(mysocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			connected = true;

			return;
#endif
#ifndef MSDOS
		case STREAM_SOCKS4_CLIENT:
			va_start(ap, type);

			subtype		= va_arg(ap, int);
			proxy		= va_arg(ap, char *);
			socksport	= va_arg(ap, int);
			server		= va_arg(ap, char *);
			port		= va_arg(ap, int);

			va_end(ap);

			if (subtype != STREAM_SOCKS4_NOAUTH) return;

			streamtype	= STREAM_SOCKS4_CLIENT;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(data);
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}
			}

			initialized++;
#endif

			mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (mysocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(mysocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (server_hostent != NULL)
			{
				socksdata = malloc(9);

				socksdata[0] = 4;
				socksdata[1] = 1;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = server_hostent->h_addr_list[0][0];
				socksdata[5] = server_hostent->h_addr_list[0][1];
				socksdata[6] = server_hostent->h_addr_list[0][2];
				socksdata[7] = server_hostent->h_addr_list[0][3];
				socksdata[8] = 0;

				send(mysocket, (char *) socksdata, 9, 0);
			}
			else if (GetIPAddress(server) != 0)
			{
				socksdata = malloc(9);

				socksdata[0] = 4;
				socksdata[1] = 1;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = GetByte(htonl(GetIPAddress(server)), 0);
				socksdata[5] = GetByte(htonl(GetIPAddress(server)), 1);
				socksdata[6] = GetByte(htonl(GetIPAddress(server)), 2);
				socksdata[7] = GetByte(htonl(GetIPAddress(server)), 3);
				socksdata[8] = 0;

				send(mysocket, (char *) socksdata, 9, 0);
			}
			else
			{
				socksdata = malloc(10 + strlen(server));

				socksdata[0] = 4;
				socksdata[1] = 1;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = 0;
				socksdata[5] = 0;
				socksdata[6] = 0;
				socksdata[7] = 1;
				socksdata[8] = 0;

				for (i = 0; i < (int) strlen(server); i++) socksdata[9 + i] = server[i];

				socksdata[9 + strlen(server)] = 0;

				send(mysocket, (char *) socksdata, 10 + strlen(server), 0);
			}

			while (recbytes != 8)
			{
				recbytes += recv(mysocket, (char *) socksdata + recbytes, 8 - recbytes, 0);
			}

			if (socksdata[1] != 90)	// proxy rejected request
			{
				free(data);
				free(socksdata);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			free(socksdata);

			connected = true;

			return;
#endif
#ifndef MSDOS
		case STREAM_SOCKS4_BIND:
			va_start(ap, type);

			subtype		= va_arg(ap, int);
			proxy		= va_arg(ap, char *);
			socksport	= va_arg(ap, int);
			server		= va_arg(ap, char *);
			port		= va_arg(ap, int);
			remote_host	= va_arg(ap, char *);
			remote_port	= va_arg(ap, unsigned short *);

			va_end(ap);

			if (subtype != STREAM_SOCKS4_NOAUTH) return;

			streamtype	= STREAM_SOCKS4_BIND;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(data);
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}
			}

			initialized++;
#endif

			mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (mysocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(mysocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (server_hostent != NULL)
			{
				socksdata = malloc(9);

				socksdata[0] = 4;
				socksdata[1] = 2;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = server_hostent->h_addr_list[0][0];
				socksdata[5] = server_hostent->h_addr_list[0][1];
				socksdata[6] = server_hostent->h_addr_list[0][2];
				socksdata[7] = server_hostent->h_addr_list[0][3];
				socksdata[8] = 0;

				send(mysocket, (char *) socksdata, 9, 0);
			}
			else if (GetIPAddress(server) != 0)
			{
				socksdata = malloc(9);

				socksdata[0] = 4;
				socksdata[1] = 2;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = GetByte(htonl(GetIPAddress(server)), 0);
				socksdata[5] = GetByte(htonl(GetIPAddress(server)), 1);
				socksdata[6] = GetByte(htonl(GetIPAddress(server)), 2);
				socksdata[7] = GetByte(htonl(GetIPAddress(server)), 3);
				socksdata[8] = 0;

				send(mysocket, (char *) socksdata, 9, 0);
			}
			else
			{
				socksdata = malloc(10 + strlen(server));

				socksdata[0] = 4;
				socksdata[1] = 2;
				socksdata[2] = htons((short) port) % 256;
				socksdata[3] = htons((short) port) / 256;
				socksdata[4] = 0;
				socksdata[5] = 0;
				socksdata[6] = 0;
				socksdata[7] = 1;
				socksdata[8] = 0;

				for (i = 0; i < (int) strlen(server); i++) socksdata[9 + i] = server[i];

				socksdata[9 + strlen(server)] = 0;

				send(mysocket, (char *) socksdata, 10 + strlen(server), 0);
			}

			while (recbytes != 8)
			{
				recbytes += recv(mysocket, (char *) socksdata + recbytes, 8 - recbytes, 0);
			}

			if (socksdata[1] != 90)	// proxy rejected request
			{
				free(data);
				free(socksdata);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			*remote_port = (unsigned short) ntohs((short) (socksdata[2] + 256 * socksdata[3]));

			if (socksdata[4] == 0 && socksdata[5] == 0 && socksdata[6] == 0 && socksdata[7] == 0)
			{
				strcpy(remote_host, proxy);
			}
			else
			{
				value	= 0;
				counter	= 0;

				for (i = 0; i < 15; i++)
				{
					if (i == 3 || i == 7 || i == 11)		value = '.';
					if (i == 0 || i == 1 || i == 2)			value = socksdata[4];
					if (i == 4 || i == 5 || i == 6)			value = socksdata[5];
					if (i == 8 || i == 9 || i == 10)		value = socksdata[6];
					if (i == 12 || i == 13 || i == 14)		value = socksdata[7];
					if (i == 0 || i == 4 || i == 8 || i == 12)	value = value / 100 + 48;
					if (i == 1 || i == 5 || i == 9 || i == 13)	value = (value % 100) / 10 + 48;
					if (i == 2 || i == 6 || i == 10 || i == 14)	value = (value % 100) % 10 + 48;

					if (value != 48)
					{
						remote_host[counter] = value;

						counter++;
					}
				}

				remote_host[counter] = 0;
			}

			free(socksdata);

			return;
#endif
#ifndef MSDOS
		case STREAM_SOCKS5_CLIENT:
			va_start(ap, type);

			subtype		= va_arg(ap, int);
			proxy		= va_arg(ap, char *);
			socksport	= va_arg(ap, int);
			server		= va_arg(ap, char *);
			port		= va_arg(ap, int);

			if (subtype == STREAM_SOCKS5_AUTH)
			{
				uname	= va_arg(ap, char *);
				passwd	= va_arg(ap, char *);
			}

			va_end(ap);

			streamtype	= STREAM_SOCKS5_CLIENT;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(data);
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}
			}

			initialized++;
#endif

			mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (mysocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(mysocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (subtype == STREAM_SOCKS5_NOAUTH)
			{
				socksdata = malloc(3);

				socksdata[0] = 5;
				socksdata[1] = 1;
				socksdata[2] = 0;

				send(mysocket, (char *) socksdata, 3, 0);

				while (recbytes != 2)
				{
					recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication
				{
					free(data);
					free(socksdata);
					streamtype = STREAM_NONE;
					[self CloseSocket: mysocket];
					isopen = false;
					return;
				}

				free(socksdata);
			}
			else
			{
				socksdata = malloc(4);

				socksdata[0] = 5;
				socksdata[1] = 2;
				socksdata[2] = 0;
				socksdata[3] = 2;

				send(mysocket, (char *) socksdata, 4, 0);

				while (recbytes != 2)
				{
					recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication, but doesn't support username/password
				{
					free(data);
					free(socksdata);
					streamtype = STREAM_NONE;
					[self CloseSocket: mysocket];
					isopen = false;
					return;
				}

				if (socksdata[1] == 2)
				{
					free(socksdata);

					socksdata = malloc(3 + strlen(uname) + strlen(passwd));

					socksdata[0] = 1;
					socksdata[1] = strlen(uname);

					for (i = 0; i < (int) strlen(uname); i++) socksdata[2 + i] = uname[i];

					socksdata[2 + strlen(uname)] = strlen(passwd);

					for (i = 0; i < (int) strlen(passwd); i++) socksdata[3 + strlen(uname) + i] = passwd[i];

					send(mysocket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

					recbytes = 0;

					while (recbytes != 2)
					{
						recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
					}

					if (socksdata[1] != 0)	// proxy rejected username/password
					{
						free(data);
						free(socksdata);
						streamtype = STREAM_NONE;
						[self CloseSocket: mysocket];
						isopen = false;
						return;
					}
				}

				free(socksdata);
			}

			socksdata = malloc(7 + strlen(server));

			socksdata[0] = 5;
			socksdata[1] = 1;
			socksdata[2] = 0;
			socksdata[3] = 3;
			socksdata[4] = strlen(server);

			for (i = 0; i < (int) strlen(server); i++) socksdata[5 + i] = server[i];

			socksdata[5 + strlen(server)] = htons((short) port) % 256;
			socksdata[6 + strlen(server)] = htons((short) port) / 256;

			send(mysocket, (char *) socksdata, 7 + strlen(server), 0);

			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(mysocket, (char *) socksdata + recbytes, 5 - recbytes, 0);
			}

			if (socksdata[1] != 0)	// an error occurred
			{
				free(data);
				free(socksdata);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (socksdata[3] == 1)
			{
				recbytes = 0;

				while (recbytes != 5)
				{
					recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
				}
			}
			else if (socksdata[3] == 3)
			{
				recbytes	= 0;
				neededbytes	= socksdata[4] + 2;

				while (recbytes != neededbytes)
				{
					recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
				}
			}

			free(socksdata);

			connected = true;

			return;
#endif
#ifndef MSDOS
		case STREAM_SOCKS5_BIND:
			va_start(ap, type);

			subtype		= va_arg(ap, int);
			proxy		= va_arg(ap, char *);
			socksport	= va_arg(ap, int);
			server		= va_arg(ap, char *);
			port		= va_arg(ap, int);

			if (subtype == STREAM_SOCKS5_AUTH)
			{
				uname	= va_arg(ap, char *);
				passwd	= va_arg(ap, char *);
			}

			remote_host = va_arg(ap, char *);
			remote_port = va_arg(ap, unsigned short *);

			va_end(ap);

			streamtype	= STREAM_SOCKS5_BIND;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(data);
					streamtype	= STREAM_NONE;
					isopen		= false;
					return;
				}
			}

			initialized++;
#endif

			mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (mysocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(mysocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (subtype == STREAM_SOCKS5_NOAUTH)
			{
				socksdata = malloc(3);

				socksdata[0] = 5;
				socksdata[1] = 1;
				socksdata[2] = 0;

				send(mysocket, (char *) socksdata, 3, 0);

				while (recbytes != 2)
				{
					recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication
				{
					free(data);
					free(socksdata);
					streamtype = STREAM_NONE;
					[self CloseSocket: mysocket];
					isopen = false;
					return;
				}

				free(socksdata);
			}
			else
			{
				socksdata = malloc(4);

				socksdata[0] = 5;
				socksdata[1] = 2;
				socksdata[2] = 0;
				socksdata[3] = 2;

				send(mysocket, (char *) socksdata, 4, 0);

				while (recbytes != 2)
				{
					recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy needs authentication, but doesn't support username/password
				{
					free(data);
					free(socksdata);
					streamtype = STREAM_NONE;
					[self CloseSocket: mysocket];
					isopen = false;
					return;
				}

				if (socksdata[1] == 2)
				{
					free(socksdata);

					socksdata = malloc(3 + strlen(uname) + strlen(passwd));

					socksdata[0] = 1;
					socksdata[1] = strlen(uname);

					for (i = 0; i < (int) strlen(uname); i++) socksdata[2 + i] = uname[i];

					socksdata[2 + strlen(uname)] = strlen(passwd);

					for (i = 0; i < (int) strlen(passwd); i++) socksdata[3 + strlen(uname) + i] = passwd[i];

					send(mysocket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

					recbytes = 0;

					while (recbytes != 2)
					{
						recbytes += recv(mysocket, (char *) socksdata + recbytes, 2 - recbytes, 0);
					}

					if (socksdata[1] != 0)	// proxy rejected username/password
					{
						free(data);
						free(socksdata);
						streamtype = STREAM_NONE;
						[self CloseSocket: mysocket];
						isopen = false;
						return;
					}
				}

				free(socksdata);
			}

			socksdata = malloc(7 + strlen(server));

			socksdata[0] = 5;
			socksdata[1] = 2;
			socksdata[2] = 0;
			socksdata[3] = 3;
			socksdata[4] = strlen(server);

			for (i = 0; i < (int) strlen(server); i++) socksdata[5 + i] = server[i];

			socksdata[5 + strlen(server)] = htons((short) port) % 256;
			socksdata[6 + strlen(server)] = htons((short) port) / 256;

			send(mysocket, (char *) socksdata, 7 + strlen(server), 0);

			free(socksdata);

			socksdata = malloc(300);

			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(mysocket, (char *) socksdata + recbytes, 5 - recbytes, 0);
			}

			if (socksdata[1] != 0)	// an error occurred
			{
				free(data);
				free(socksdata);
				streamtype = STREAM_NONE;
				[self CloseSocket: mysocket];
				isopen = false;
				return;
			}

			if (socksdata[3] == 1)
			{
				recbytes = 0;

				while (recbytes != 5)
				{
					recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
				}
			}
			else if (socksdata[3] == 3)
			{
				recbytes	= 0;
				neededbytes	= socksdata[4] + 2;

				while (recbytes != neededbytes)
				{
					recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
				}
			}

			switch (socksdata[3])
			{
				case 1:
					*remote_port = (unsigned short) ntohs((short) (socksdata[8] + 256 * socksdata[9]));

					value	= 0;
					counter	= 0;

					for (i = 0; i < 15; i++)
					{
						if (i == 3 || i == 7 || i == 11)		value = '.';
						if (i == 0 || i == 1 || i == 2)			value = socksdata[4];
						if (i == 4 || i == 5 || i == 6)			value = socksdata[5];
						if (i == 8 || i == 9 || i == 10)		value = socksdata[6];
						if (i == 12 || i == 13 || i == 14)		value = socksdata[7];
						if (i == 0 || i == 4 || i == 8 || i == 12)	value = value / 100 + 48;
						if (i == 1 || i == 5 || i == 9 || i == 13)	value = (value % 100) / 10 + 48;
						if (i == 2 || i == 6 || i == 10 || i == 14)	value = (value % 100) % 10 + 48;

						if (value != 48)
						{
							remote_host[counter] = value;

							counter++;
						}
					}

					remote_host[counter] = 0;

					break;
				case 3:
					*remote_port = (unsigned short) ntohs((short) (socksdata[5 + socksdata[4]] + 256 * socksdata[6 + socksdata[4]]));

					for (i = 0; i < socksdata[4]; i++) remote_host[i] = socksdata[5 + i];

					remote_host[socksdata[4]] = 0;

					break;
			}

			free(socksdata);

			return;
#endif
		case STREAM_POSIX:
			va_start(ap, type);

			file = va_arg(ap, int);

			va_end(ap);

			streamtype	= STREAM_POSIX;
			stream		= file;
			closefile	= false;
			currentfilepos	= tell(stream);
			lastfilepos	= 0;

			lseek(stream, 0, SEEK_END);

			size		= tell(stream);

			lseek(stream, currentfilepos, SEEK_SET);

			data		= malloc(packagesize);
			isopen		= true;

			return;
#ifndef MSDOS
		case STREAM_SOCKET:
			va_start(ap, type);

			opensocket = va_arg(ap, SOCKET);

			va_end(ap);

			streamtype	= STREAM_SOCKET;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);
			isopen		= true;
			mysocket	= opensocket;
			connected	= true;
			closefile	= false;

			return;
#endif
#ifndef MSDOS
		case STREAM_SERVER:
			va_start(ap, type);

			port = va_arg(ap, int);

			va_end(ap);

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					streamtype = STREAM_NONE;
					return;
				}
			}

			initialized++;
#endif

			streamtype	= STREAM_SERVER;
			socketstream	= true;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);

			localSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (localSocket == (SOCKET)(~0))
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: localSocket];
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr.s_addr	= INADDR_ANY;
			saddr.sin_port		= htons((short) port);

			if (bind(localSocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: localSocket];
				return;
			}

			if (listen(localSocket, SOMAXCONN) == -1)
			{
				free(data);
				streamtype = STREAM_NONE;
				[self CloseSocket: localSocket];
				return;
			}

			return;
#endif
		case STREAM_ANSI:
			va_start(ap, type);

			openfile = va_arg(ap, FILE *);

			va_end(ap);

			streamtype	= STREAM_ANSI;
			hlstream	= openfile;
			closefile	= false;
			currentfilepos	= ftell(hlstream);
			lastfilepos	= 0;

			fseek(hlstream, 0, SEEK_END);

			size		= ftell(hlstream);

			fseek(hlstream, currentfilepos, SEEK_SET);

			packagesize	= 1; // low package size, 'cause openfile could point at the console or so
			stdpacksize	= packagesize;
			origpacksize	= packagesize;
			data		= malloc(packagesize);
			isopen		= true;

			return;
#if defined __WIN32__
		case STREAM_WINDOWS:
			va_start(ap, type);

			file = va_arg(ap, int);

			va_end(ap);

			streamtype	= STREAM_WINDOWS;
			stream		= file;
			closefile	= false;
			currentfilepos	= _llseek(stream, 0, FILE_CURRENT);
			lastfilepos	= 0;

			_llseek(stream, 0, FILE_END);

			size		= _llseek(stream, 0, FILE_END);

			_llseek(stream, currentfilepos, FILE_BEGIN);

			data		= malloc(packagesize);
			isopen		= true;

			return;
#endif
		case STREAM_BUFFER:
			va_start(ap, type);

			outbuffer	= va_arg(ap, void *);
			bufsize		= va_arg(ap, long);

			va_end(ap);

			streamtype	= STREAM_BUFFER;
			lastfilepos	= 0;
			packagesize	= 1;
			stdpacksize	= packagesize;
			origpacksize	= packagesize;
			data		= malloc(packagesize);
			closefile	= false;
			isopen		= true;
			buffersize	= bufsize;
			buffer		= outbuffer;

			return;
		case STREAM_STREAM:
			va_start(ap, type);

			instr = va_arg(ap, InStream *);

			va_end(ap);

			if (instr->streamtype == STREAM_SERVER || instr->streamtype == STREAM_SOCKS5_BIND || instr->streamtype == STREAM_SOCKS4_BIND)							return;
			if ((!instr->isopen && !(instr->streamtype == STREAM_SERVER || instr->streamtype == STREAM_SOCKS5_BIND || instr->streamtype == STREAM_SOCKS4_BIND)) || instr->crosslinked)	return;

			streamtype = STREAM_STREAM;

			crosslinked = true;
			instream = instr;
			instream->outstream = self;
			instream->crosslinked = true;

			if (instream->streamtype == STREAM_ANSI) // instream is an ANSI file handle
			{
				streamtype	= STREAM_ANSI;
				closefile	= false;
				isopen		= true;
				hlstream	= instream->hlstream;
				currentfilepos	= instream->currentfilepos;
				lastfilepos	= currentfilepos;
				packagesize	= 1;
				stdpacksize	= packagesize;
				origpacksize	= packagesize;
				data		= malloc(packagesize);
				size		= instream->origsize;

				return;
			}
#if defined __WIN32__
			else if (instream->streamtype == STREAM_WINDOWS)
			{
				streamtype	= STREAM_WINDOWS;
				closefile	= false;
				isopen		= true;
				stream		= instream->stream;
				currentfilepos	= instream->currentfilepos;
				lastfilepos	= currentfilepos;
				data		= malloc(packagesize);
				size		= instream->origsize;

				return;
			}
#endif
			else if (instream->socketstream) // instream is a socket
			{
				initialized++;

				streamtype	= instream->streamtype;
				socketstream	= true;
				faraddr		= instream->faraddr;
				closefile	= false;
				isopen		= instr->isopen;
				connected	= instream->connected;
				mysocket	= instream->mysocket;
				localSocket	= instream->localSocket;
				currentfilepos	= -1;
				lastfilepos	= 0;
				data		= malloc(packagesize);

				return;
			}
			else if (instream->buffersize != -1) // instream is a mem buffer
			{
				streamtype	= STREAM_BUFFER;
				buffersize	= instream->buffersize;
				closefile	= false;
				isopen		= true;
				buffer		= instream->buffer;
				lastfilepos	= 0;
				packagesize	= 1;
				stdpacksize	 = packagesize;
				origpacksize	= packagesize;
				data		= malloc(packagesize);
				size		= buffersize;

				return;
			}
			else // instream is a posix file handle
			{
				streamtype	= STREAM_POSIX;
				closefile	= false;
				isopen		= true;
				stream		= instream->stream;
				currentfilepos	= instream->currentfilepos;
				lastfilepos	= currentfilepos;
				data		= malloc(packagesize);
				size		= instream->origsize;

				return;
			}

			return;
		case STREAM_CHILD:
			va_start(ap, type);

			outstr = va_arg(ap, OutStream *);

			va_end(ap);

			initialized++;

			streamtype	= STREAM_CHILD;
			socketstream	= true;
			isopen		= true;
			connected	= true;
			mysocket	= outstr->mysocket;
			currentfilepos	= -1;
			lastfilepos	= 0;
			data		= malloc(packagesize);

			return;
		default:
			return;
	}
}

- (OutStream *) WaitForClient
{
#ifdef MSDOS
	return NULL;
#else

	OutStream	*out = NULL;
	unsigned char	*socksdata;
	int		 recbytes = 0;
	int		 neededbytes;

#if defined __WIN32__ || defined __CYGWIN32__ || defined __BEOS__ || defined sgi
	int		 faraddrsize = sizeof(struct sockaddr_in);
#else
	unsigned int	 faraddrsize = sizeof(struct sockaddr_in);
#endif

	if (streamtype != STREAM_SERVER && streamtype != STREAM_SOCKS5_BIND && streamtype != STREAM_SOCKS4_BIND) return NULL;

	if (streamtype == STREAM_SERVER)
	{
		mysocket = accept(localSocket, (struct sockaddr *) &faraddr, &faraddrsize);

		if (mysocket == (SOCKET)(~0))
		{
			return NULL;
		}
		else
		{
			out = [OutStream new];

			[out Connect: STREAM_CHILD, self];

			out->faraddr = faraddr;

			return out;
		}
	}
	else if (streamtype == STREAM_SOCKS4_BIND)
	{
		socksdata = malloc(8);

		while (recbytes != 8)
		{
			recbytes += recv(mysocket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 90)	// an error occurred
		{
			free(data);
			free(socksdata);
			streamtype = STREAM_NONE;
			[self CloseSocket: mysocket];
			isopen = false;
			return NULL;
		}

		free(socksdata);

		out = [OutStream new];

		[out Connect: STREAM_CHILD, self];

		return out;
	}
	else if (streamtype == STREAM_SOCKS5_BIND)
	{
		socksdata = malloc(300);

		while (recbytes != 5)
		{
			recbytes += recv(mysocket, (char *) socksdata + recbytes, 5 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			free(data);
			free(socksdata);
			streamtype = STREAM_NONE;
			[self CloseSocket: mysocket];
			isopen = false;
			return NULL;
		}

		if (socksdata[3] == 1)
		{
			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}
		else if (socksdata[3] == 3)
		{
			recbytes	= 0;
			neededbytes	= socksdata[4] + 2;

			while (recbytes != neededbytes)
			{
				recbytes += recv(mysocket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
			}
		}

		free(socksdata);

		out = [OutStream new];

		[out Connect: STREAM_CHILD, self];

		return out;
	}

	return NULL;
#endif
}

- (void) Flush
{
	int	 ps		= packagesize;
	int	 oldcpos	= currentpos;
	int	 i;

	if (!isopen)		return;
	if (currentpos <= 0)	return;

	if (pbd) [self CompletePBD];

	if (filter != NULLFILTER && filter->packsize > 0)
	{
		for (i = 0; i < (packagesize - oldcpos); i++)
		{
			[self OutputNumber: 0: 1];
		}
	}
	else
	{
		packagesize = currentpos;

		[self WriteData];

		packagesize = ps;
	}
}

- (void) WriteData
{
	unsigned char	*databuffer;
	int		 encsize = 0;

	if (!isopen)			return;
	if (currentpos < packagesize)	return;

	if (filter->packsize == -1)
	{
		databuffer = malloc(packagesize);

		memcpy((void *) databuffer, (void *) data, packagesize);

		free(data);

		data = malloc(packagesize + DEFAULT_PACKAGE_SIZE);

		memcpy((void *) data, (void *) databuffer, packagesize);

		free(databuffer);

		packagesize += DEFAULT_PACKAGE_SIZE;
		stdpacksize = packagesize;

		return;
	}

	if (streamtype == STREAM_ANSI)
	{
		fseek(hlstream, lastfilepos, SEEK_SET);

		[filter EncodeData: &data: packagesize: &encsize];

		fwrite((void *) data, encsize, 1, hlstream);
		fflush(hlstream);

		free(data);

		data = malloc(packagesize);

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
#if defined __WIN32__
	else if (streamtype == STREAM_WINDOWS)
	{
		_llseek(stream, lastfilepos, FILE_BEGIN);

		[filter EncodeData: &data: packagesize: &encsize];

		_hwrite(stream, (char *) data, encsize);

		free(data);

		data = malloc(packagesize);

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
#endif
#ifndef MSDOS
	else if (socketstream)
	{
		if (send(mysocket, (char *) data, packagesize, 0) == -1)
		{							// looks like other end has been closed
			free(data);
			connected	= false;
			isopen		= false;
			streamtype	= STREAM_NONE;

			[self CloseSocket: mysocket];
		}

		currentpos -= packagesize;
	}
#endif
	else if (buffersize == -1)
	{
		lseek(stream, lastfilepos, SEEK_SET);

		[filter EncodeData: &data: packagesize: &encsize];

		write(stream, data, encsize);

		free(data);

		data = malloc(packagesize);

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
	else
	{
		[filter EncodeData: &data: packagesize: &encsize];

		if (encsize <= (buffersize - bufferpos))
		{
			memcpy((void *) ((unsigned char *) buffer + bufferpos), (void *) data, encsize);

			bufferpos += encsize;
		}
		else
		{
			memcpy((void *) ((unsigned char *) buffer + bufferpos), (void *) data, (buffersize - 1) - bufferpos);

			bufferpos = buffersize - 1;
		}

		free(data);

		data = malloc(packagesize);

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
	}
}

- (bool) OutputNumber: (long) number: (int) bytes
{
	int	 i;

	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	for (i = 0; i < bytes; i++)
	{
		if (currentpos >= packagesize) [self WriteData];

		data[currentpos] = GetByte(number, i);
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	if (currentpos >= packagesize) [self WriteData];

	if (socketstream && !holdpbd) [self Flush];

	return true;
}

- (bool) OutputNumberRaw: (long) number: (int) bytes
{
	int	 i;

	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	for (i = bytes - 1; i >= 0; i--)
	{
		if (currentpos >= packagesize) [self WriteData];

		data[currentpos] = GetByte(number, i);
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	if (currentpos >= packagesize) [self WriteData];

	if (socketstream && !holdpbd) [self Flush];

	return true;
}

- (bool) OutputNumberVAX: (long) number: (int) bytes
{
	int	 i;

	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	number <<= 8 * (4 - bytes);

	for (i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (currentpos >= packagesize) [self WriteData];

			data[currentpos] = GetByte(number, (3 - i) ^ 1);
			if (currentfilepos == size) size++;
			currentpos++;
			if (!socketstream) currentfilepos++;
		}
	}

	if (currentpos >= packagesize) [self WriteData];

	if (socketstream && !holdpbd) [self Flush];

	return true;
}

- (bool) OutputNumberPBD: (long) number: (int) bits
{
	unsigned char	 out;
	int		 i;
	int		 j;

	if (!isopen)	return false;
	if (bits > 32)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (!pbd) [self InitPBD];

	for (j = 0; j < bits; j++)
	{
		pbdbuffer[pbdlen] = GetBit(number, j);
		pbdlen++;
	}

	while (pbdlen >= 8)
	{
		out = 0;

		for (i = 0; i < 8; i++)
		{
			out = out | (pbdbuffer[i] << i);
		}

		pbdlen = pbdlen - 8;

		for (j = 0; j < pbdlen; j++)
		{
			pbdbuffer[j] = pbdbuffer[j+8];
		}

		data[currentpos] = out;
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
		if (currentpos >= packagesize) [self WriteData];
	}

	return true;
}

- (bool) OutputString: (const char *) string
{
	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	if (string == NULL) return false;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		[self WriteData];
	}

	if (socketstream) [self Flush];

	return true;
}

- (bool) OutputLine: (const char *) string
{
	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	if (string == NULL) return false;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		[self WriteData];
	}

#if (defined __WIN32__ || defined MSDOS) && !defined __CYGWIN32__
	[self OutputNumber: 13: 1];
#endif

	[self OutputNumber: 10: 1];

	if (socketstream) [self Flush];

	return true;
}

- (bool) OutputData: (const void *) pointer: (int) bytes
{
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) [self CompletePBD];

	if (pointer == NULL) return false;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) pointer + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		[self WriteData];
	}

	if (socketstream) [self Flush];

	return true;
}

- (void) InitPBD
{
	pbdlen	= 0;
	pbd	= 1;
}

- (void) CompletePBD
{
	int	 out = 0;
	int	 i;

	if (pbdlen > 0)
	{
		for (i = 0; i < 8; i++)
		{
			if (i < pbdlen) out = out | (pbdbuffer[i] << i);
		}
		holdpbd = true;
		[self OutputNumber: out: 1];
		holdpbd = false;
	}

	if (socketstream) [self Flush];

	pbd = 0;
}

- (bool) SetPackageSize: (int) newPackagesize
{
	if (!isopen)			return false;
	if (!allowpackset)		return false;
	if (newPackagesize <= 0)	return false;

	[self Flush];

	lastfilepos = currentfilepos;

	free(data);

	data = malloc(newPackagesize);

	packagesize = newPackagesize;
	stdpacksize = packagesize;

	return true;
}

- (bool) SetFilter: (IOLibFilter *) newFilter
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	if (pbd && !holdpbd) [self CompletePBD];

	allowpackset = true;

	if (newFilter->packsize > 0)
	{
		[self SetPackageSize: newFilter->packsize];	// package size must be eqv filter size

		allowpackset = false;
	}
	else if (newFilter->packsize == -1)
	{
		[self SetPackageSize: DEFAULT_PACKAGE_SIZE];

		allowpackset = false;
	}

	filter = newFilter;

	return true;
}

- (bool) RemoveFilter
{
	int	 oldcpos;
	int	 i;

	if (!isopen)		return false;
	if (socketstream)	return false;

	if (pbd && !holdpbd) [self CompletePBD];

	oldcpos = currentpos;

	if (filter->packsize > 0 && currentpos != 0)
	{
		for (i = 0; i < (packagesize - oldcpos); i++)
		{
			[self OutputNumber: 0: 1];
		}
	}
	else if (filter->packsize == -1)
	{
		filter->packsize = 0;

		allowpackset = true;

		[self Flush];

		filter->packsize = -1;
	}

	allowpackset = true;

	filter = NULLFILTER;

	[self SetPackageSize: origpacksize];

	return true;
}

- (bool) Close
{
	if (!isopen && streamtype == STREAM_NONE) return false;

	if (filter != NULLFILTER) [self RemoveFilter];

	[self Flush];

	if (crosslinked)
	{
		if (closefile)
		{
			instream->closefile = true;
			if (streamtype == STREAM_FILE) instream->streamtype = STREAM_FILE;
		}

		closefile = false;
	}

	if (!socketstream)
	{
		if (closefile) close(stream);
	}
#ifndef MSDOS
	else
	{
		if (closefile)
		{
			if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND)
			{
#ifdef __WIN32__
				closesocket(localSocket);
				if (connected) closesocket(mysocket);
#else
				close(localSocket);
				if (connected) close(mysocket);
#endif
			}
			else
			{
#ifdef __WIN32__
				closesocket(mysocket);
#else
				close(mysocket);
#endif
			}
		}
	}
#endif

	if (socketstream) initialized--;

	if (socketstream && initialized == 0)
	{
#if defined __WIN32__ && !defined __CYGWIN32__
		WSACleanup();
#endif
	}

	free(data);
	data = NULL;

	isopen		= false;
	streamtype	= STREAM_NONE;

	return true;
}

- (bool) Seek: (long) position
{
	if(!isopen)		return false;
	if (socketstream)	return false;

	[self Flush];

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, position, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, position, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, position, FILE_BEGIN);
#endif

	currentfilepos	= position;
	bufferpos	= position;
	lastfilepos	= position;
	currentpos	= 0;

	return true;
}

- (bool) RelSeek: (long) offset
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	[self Flush];

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, currentfilepos + offset, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, currentfilepos + offset, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, currentfilepos + offset, FILE_BEGIN);
#endif

	currentfilepos	+= offset;
	bufferpos	+= offset;
	lastfilepos	= currentfilepos;
	currentpos	= 0;

	return true;
}

@end

#endif
