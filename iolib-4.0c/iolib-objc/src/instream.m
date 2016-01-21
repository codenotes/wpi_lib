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

#ifndef __IOLIB_INSTREAM_
#define __IOLIB_INSTREAM_

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

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

@implementation InStream

- free
{
	if (crosslinked)
	{
		outstream->crosslinked	= false;
		outstream->instream	= NULL;
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
	unsigned short		 port;
	unsigned short		 socksport;
	int			 file;
	int			 subtype;
	FILE			*openfile;
	void			*inbuffer;
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

			filename = va_arg(ap, char *);

			va_end(ap);

			streamtype	= STREAM_FILE;
			currentpos	= DEFAULT_PACKAGE_SIZE;
			data		= malloc(packagesize);
			stream		= open(filename, O_RDWR | O_BINARY);

			if (stream <= 0)
			{
				free(data);
				streamtype = STREAM_NONE;
				return;
			}

			lseek(stream, 0, SEEK_END);

			size		= tell(stream);
			origsize	= size;

			lseek(stream, 0, SEEK_SET);

			isopen		= true;

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
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
				value = 0;
				counter = 0;

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
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
			currentpos	= DEFAULT_PACKAGE_SIZE;

			lseek(stream, 0, SEEK_END);

			size		= tell(stream);
			origsize	= size;

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
			mysocket	= opensocket;
			closefile	= false;
			socketstream	= true;
			currentfilepos	= -1;
			allowpackset	= false;
			currentpos	= DEFAULT_PACKAGE_SIZE;
			data		= malloc(packagesize);
			isopen		= true;
			connected	= true;

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
			allowpackset	= false;
			currentfilepos	= -1;
			currentpos	= DEFAULT_PACKAGE_SIZE;
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
			currentpos	= DEFAULT_PACKAGE_SIZE;

			fseek(hlstream, 0, SEEK_END);

			size		= ftell(hlstream);
			origsize	= size;

			fseek(hlstream, currentfilepos, SEEK_SET);

			packagesize	= 1;
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
			currentpos	= DEFAULT_PACKAGE_SIZE;

			_llseek(stream, 0, FILE_END);

			size		= _llseek(stream, 0, FILE_END);
			origsize	= size;

			_llseek(stream, currentfilepos, FILE_BEGIN);

			data		= malloc(packagesize);
			isopen		= true;

			return;
#endif
		case STREAM_BUFFER:
			va_start(ap, type);

			inbuffer	= va_arg(ap, void *);
			bufsize		= va_arg(ap, long);

			va_end(ap);

			streamtype	= STREAM_BUFFER;
			buffer		= inbuffer;
			buffersize	= bufsize;
			packagesize	= 1;
			stdpacksize	= packagesize;
			origpacksize	= packagesize;
			currentpos	= packagesize;
			size		= buffersize;
			origsize	= size;
			data		= malloc(packagesize);
			closefile	= false;
			isopen		= true;

			return;
		case STREAM_STREAM:
			va_start(ap, type);

			outstr = va_arg(ap, OutStream *);

			va_end(ap);

			if (outstr->streamtype == STREAM_SERVER || outstr->streamtype == STREAM_SOCKS5_BIND || outstr->streamtype == STREAM_SOCKS4_BIND)							return;
			if ((!outstr->isopen && !(outstr->streamtype == STREAM_SERVER || outstr->streamtype == STREAM_SOCKS5_BIND || outstr->streamtype == STREAM_SOCKS4_BIND)) || outstr->crosslinked)	return;

			streamtype = STREAM_STREAM;

			crosslinked = true;
			outstream = outstr;
			outstream->instream = self;
			outstream->crosslinked = true;

			if (outstream->streamtype == STREAM_ANSI) // outstream got an ANSI file handle
			{
				streamtype	= STREAM_ANSI;
				closefile	= false;
				isopen		= true;
				hlstream	= outstream->hlstream;
				currentfilepos	= outstream->currentfilepos;
				packagesize	= 1;
				stdpacksize	= packagesize;
				origpacksize	= packagesize;
				data		= malloc(packagesize);
				currentpos	= packagesize;
				size		= outstream->size;
				origsize	= size;

				return;
			}
#if defined __WIN32__
			else if (outstream->streamtype == STREAM_WINDOWS)
			{
				streamtype	= STREAM_WINDOWS;
				closefile	= false;
				isopen		= true;
				stream		= outstream->stream;
				currentfilepos	= outstream->currentfilepos;
				data		= malloc(packagesize);
				currentpos	= packagesize;
				size		= outstream->size;
				origsize	= size;

				return;
			}
#endif
			else if (outstream->socketstream) // outstream is a socket
			{
				initialized++;

				streamtype	= outstream->streamtype;
				socketstream	= true;
				faraddr		= outstream->faraddr;
				closefile	= false;
				isopen		= outstream->isopen;
				connected	= outstream->connected;
				mysocket	= outstream->mysocket;
				localSocket	= outstream->localSocket;
				allowpackset	= false;
				currentfilepos	= -1;
				data		= malloc(packagesize);
				currentpos	= packagesize;

				return;
			}
			else if (outstream->buffersize != -1) // outstream is a mem buffer
			{
				streamtype	= STREAM_BUFFER;
				buffersize	= outstream->buffersize;
				closefile	= false;
				isopen		= true;
				buffer		= outstream->buffer;
				packagesize	= 1;
				stdpacksize	= packagesize;
				origpacksize	= packagesize;
				data		= malloc(packagesize);
				currentpos	= packagesize;
				size		= buffersize;
				origsize	= size;

				return;
			}
			else // outstream is a posix file handle
			{
				streamtype	= STREAM_POSIX;
				closefile	= false;
				isopen		= true;
				stream		= outstream->stream;
				currentfilepos	= outstream->currentfilepos;
				data		= malloc(packagesize);
				currentpos	= packagesize;
				size		= outstream->size;
				origsize	= size;

				return;
			}

			return;
		case STREAM_CHILD:
			va_start(ap, type);

			instr = va_arg(ap, InStream *);

			va_end(ap);

			initialized++;

			streamtype	= STREAM_CHILD;
			socketstream	= true;
			isopen		= true;
			connected	= true;
			mysocket	= instr->mysocket;
			allowpackset	= false;
			currentfilepos	= -1;
			data		= malloc(packagesize);
			currentpos	= packagesize;

			return;
		default:
			return;
	}
}

- (InStream *) WaitForClient
{
#ifdef MSDOS
	return NULL;
#else

	InStream	*in = NULL;
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
			in = [InStream new];

			[in Connect: STREAM_CHILD, self];

			in->faraddr = faraddr;

			return in;
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

		in = [InStream new];

		[in Connect: STREAM_CHILD, self];

		return in;
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

		in = [InStream new];

		[in Connect: STREAM_CHILD, self];

		return in;
	}

	return NULL;
#endif
}

- (void) ReadData
{
	int decsize = 0;

	if (!isopen) return;

	if (streamtype == STREAM_ANSI)
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize != 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		free(data);

		data = malloc(packagesize);

		fseek(hlstream, currentfilepos, SEEK_SET); // Do this because a crosslinked OutStream might have changed the file pointer

		fread((void *) data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), 1, hlstream);

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			[filter DecodeData: &data: ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)): &decsize];

			packagesize	= decsize;
			origsize	= size;

			if (packagesize + currentfilepos > size) size = packagesize + currentfilepos;
		}

		currentpos = 0;
	}
#if defined __WIN32__
	else if (streamtype == STREAM_WINDOWS)
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize != 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		free(data);

		data = malloc(packagesize);

		_llseek(stream, currentfilepos, FILE_BEGIN); // Do this because a crosslinked OutStream might have changed the file pointer

		_hread(stream, (void *) data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)));

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			[filter DecodeData: &data: ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)): &decsize];

			packagesize	= decsize;
			origsize	= size;

			if (packagesize + currentfilepos > size) size = packagesize + currentfilepos;
		}

		currentpos = 0;
	}
#endif
#ifndef MSDOS
	else if (socketstream)
	{
		packagesize = DEFAULT_PACKAGE_SIZE;
		packagesize = recv(mysocket, (char *) data, packagesize, 0);

		if (packagesize == -1)
		{
			free(data);
			connected	= false;
			isopen		= false;
			streamtype	= STREAM_NONE;

			[self CloseSocket: mysocket];
		}

		currentpos = 0;
	}
#endif
	else if (buffersize == -1)
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize != 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		free(data);

		data = malloc(packagesize);

		lseek(stream, currentfilepos, SEEK_SET);

		read(stream, data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)));

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			[filter DecodeData: &data: ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)): &decsize];

			packagesize	= decsize;
			origsize	= size;

			if (packagesize + currentfilepos > size) size = packagesize + currentfilepos;
		}

		currentpos = 0;
	}
	else
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize != 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		free(data);

		data = malloc(packagesize);

		if (packagesize <= (buffersize - bufferpos))
		{
			memcpy((void *) data, (void *) ((unsigned char *) buffer + bufferpos), packagesize);

			bufferpos += packagesize;
		}
		else
		{
			memcpy((void *) data, (void *) ((unsigned char *) buffer + bufferpos), (buffersize - 1) - bufferpos);

			bufferpos = buffersize - 1;
		}

		if (packagesize <= size - currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			[filter DecodeData: &data: ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)): &decsize];

			packagesize	= decsize;
			origsize	= size;

			if (packagesize + currentfilepos > size) size = packagesize + currentfilepos;
		}

		currentpos = 0;
	}
}

- (long) InputNumber: (int) bytes	// Intel byte order DCBA
{
	long	 rval = 0;
	int	 i;

	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	for (i = 0; i < bytes; i++)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) [self ReadData];
		if (packagesize == 0) return -1;

		rval += data[currentpos] * (1 << (i * 8));
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	return rval;
}

- (long) InputNumberRaw: (int) bytes	// Raw byte order ABCD
{
	long	 rval = 0;
	int	 i;

	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	for (i = bytes - 1; i >= 0; i--)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) [self ReadData];
		if (packagesize == 0) return -1;

		rval += data[currentpos] * (1 << (i * 8));
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	return rval;
}

- (long) InputNumberVAX: (int) bytes	// VAX byte order BADC
{
	long	 rval = 0;
	int	 i;

	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	for (i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (currentfilepos >= size) return -1;

			if (currentpos >= packagesize) [self ReadData];
			if (packagesize == 0) return -1;

			rval += (data[currentpos] << (((3 - i) ^ 1) * 8)) >> (8 * (4 - bytes));
			currentpos++;
			if (!socketstream) currentfilepos++;
		}
	}

	return rval;
}

- (long) InputNumberPBD: (int) bits
{
	unsigned char	 inp;
	long		 rval = 0;
	int		 i;
	int		 j;

	if (!isopen)	return -1;
	if (bits > 32)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (!pbd) [self InitPBD];

	if (crosslinked && socketstream) [outstream Flush];

	while (pbdlen < bits)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) [self ReadData];
		if (packagesize == 0) return -1;

		inp = data[currentpos];
		currentpos++;
		if (!socketstream) currentfilepos++;

		for (i = 0; i < 8; i++)
		{
			pbdbuffer[pbdlen] = GetBit(inp, i);
			pbdlen++;
		}
	}

	for (i = 0; i < bits; i++)
	{
		rval = rval | (pbdbuffer[i] << i);
	}

	pbdlen = pbdlen - bits;

	for (j = 0; j < pbdlen; j++)
	{
		pbdbuffer[j] = pbdbuffer[j + bits];
	}

	return rval;
}

- (char *) InputString: (int) bytes
{
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;
	char	*rval;

	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) [self CompletePBD];

	if (bytes == 0) return NULL;

	if (crosslinked && socketstream) [outstream Flush];

	rval = malloc(bytes + 1);

	while (bytesleft)
	{
		if (currentfilepos >= size)
		{
			free(rval);
			return NULL;
		}

		if (currentpos >= packagesize) [self ReadData];

		if (packagesize == 0)
		{
			free(rval);
			return NULL;
		}

		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) ((unsigned char *) rval + databufferpos), (void *) (data + currentpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;
	}

	rval[bytes] = 0;

	return rval;
}

- (char *) InputLine
{
	long	 inpval;
	int	 bytes = 0;
	char	*rval;
	char	*linebuffer1;
	char	*linebuffer2;
	int	 i;
	int	 j;
	int	 k;

	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	linebuffer1 = malloc(1024);

	for (j = 0; j >= 0; j++)
	{
		for (i = 0; i < 1024; i++)
		{
			inpval = [self InputNumber: 1];

			if (inpval == -1)
			{
				linebuffer1[bytes] = 0;

				rval = malloc(bytes + 1);
				for (k = 0; k <= bytes; k++) rval[k] = linebuffer1[k];
				free(linebuffer1);

				return rval;
			}

			if (inpval != 13 && inpval != 10)
			{
				linebuffer1[bytes] = (char) inpval;
				bytes++;
			}
			else
			{
				if (inpval == 13) [self InputNumber: 1];

				linebuffer1[bytes] = 0;

				rval = malloc(bytes + 1);
				for (k = 0; k <= bytes; k++) rval[k] = linebuffer1[k];
				free(linebuffer1);

				return rval;
			}
		}

		linebuffer2 = malloc(bytes);

		for (i = 0; i < bytes; i++) linebuffer2[i] = linebuffer1[i];

		free(linebuffer1);

		linebuffer1 = malloc(bytes + 1024);

		for (i = 0; i < bytes; i++) linebuffer1[i] = linebuffer2[i];

		free(linebuffer2);
	}

	return NULL;
}

- (char *) InputSocketString
{
	int	 datasize;
	char	*rval;
	int	 i;

	if (!isopen)		return NULL;
	if (!socketstream)	return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	if (currentpos >= packagesize)
	{
		[self ReadData];

		if (packagesize == 0) return NULL;

		datasize = packagesize;
	}
	else
	{
		datasize = packagesize - currentpos;
	}

	if (datasize == 0) return NULL;

	rval = malloc(datasize + 1);

	for (i = 0; i < datasize; i++)
	{
		rval[i] = (char) [self InputNumber: 1];
	}
	rval[datasize] = 0;

	return rval;
}

- (void *) InputData: (void *) pointer: (int) bytes
{
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) [self CompletePBD];

	if (crosslinked && socketstream) [outstream Flush];

	if (bytes == 0) return NULL;

	while (bytesleft)
	{
		if (currentfilepos >= size) return NULL;

		if (currentpos >= packagesize) [self ReadData];
		if (packagesize == 0) return NULL;

		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) ((unsigned char *) pointer + databufferpos), (void *) (data + currentpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;
	}

	return pointer;
}

- (void) InitPBD
{
	pbdlen	= 0;
	pbd	= 1;
}

- (void) CompletePBD
{
	pbdlen	= 0;
	pbd	= 0;
}

- (bool) SetPackageSize: (int) newPackagesize
{
	if (!isopen)			return false;
	if (!allowpackset)		return false;
	if (newPackagesize <= 0)	return false;

	if (pbd) [self CompletePBD];

	free(data);

	data = malloc(newPackagesize);

	packagesize = newPackagesize;
	stdpacksize = packagesize;

	[self Seek: currentfilepos];

	return true;
}

- (bool) SetFilter: (IOLibFilter *) newFilter
{
	if (!isopen)		return false;
	if (socketstream)	return false; // Filters are not allowed on socket streams currently

	filter = newFilter;

	allowpackset = true;

	if (filter->packsize > 0)
	{
		[self SetPackageSize: filter->packsize];

		allowpackset = false;
	}
	else if (filter->packsize == -1)
	{
		[self SetPackageSize: size - currentfilepos];

		allowpackset = false;
	}

	return true;
}

- (bool) RemoveFilter
{
	if (!isopen)		return false;
	if (socketstream)	return false; // Filters are not allowed on socket streams currently

	filter = NULLFILTER;

	allowpackset = true;

	[self SetPackageSize: origpacksize];

	return true;
}

- (bool) Close
{
	if (!isopen && streamtype == STREAM_NONE) return false;

	if (crosslinked)
	{
		if (closefile)
		{
			outstream->closefile = true;
			if (streamtype == STREAM_FILE) outstream->streamtype = STREAM_FILE;
		}

		closefile = false;
	}

	if (pbd) [self CompletePBD];

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
	if (!isopen)		return false;
	if (socketstream)	return false; // You cannot seek in a socket stream

	if (pbd) [self CompletePBD];

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, position, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, position, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, position, FILE_BEGIN);
#endif

	currentfilepos	= position;
	bufferpos	= position;
	origfilepos	= position;

	[self ReadData];

	return true;
}

- (bool) RelSeek: (long) offset
{
	if (!isopen)		return false;
	if (socketstream)	return false; // You cannot seek in a socket stream

	if (pbd) [self CompletePBD];

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, currentfilepos + offset, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, currentfilepos + offset, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, currentfilepos + offset, FILE_BEGIN);
#endif

	currentfilepos	+= offset;
	bufferpos	+= offset;
	origfilepos	= currentfilepos;

	[self ReadData];

	return true;
}

@end

#endif
