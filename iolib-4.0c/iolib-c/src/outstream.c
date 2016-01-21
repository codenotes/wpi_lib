 /* IOLib-C, Universal IO Library
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
#include <iolib-c.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#if !defined __WIN32__
	#include <sys/ioctl.h>
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

void OutStream_Init_real(OutStream *stream)
{
	stream->iostream = malloc(sizeof(IOLibStream));

	IOLibStream_Init(stream->iostream);
}

void OutStream_Free_real(OutStream *stream)
{
	if (stream->iostream->crosslinked)
	{
		((InStream *) stream->instream)->iostream->crosslinked	= false;
		((InStream *) stream->instream)->outstream		= NULL;
	}

	OutStream_Close(stream);	// Datei schließen (return egal)

	IOLibStream_Free(stream->iostream);

	free(stream->iostream);
}

void OutStream_Connect(OutStream *stream, int type, ...)
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
	va_list			 ap;
	InStream		*in;
	OutStream		*out;
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

			stream->iostream->streamtype	= STREAM_FILE;
			stream->lastfilepos		= 0;
			stream->iostream->data		= malloc(stream->iostream->packagesize);

			if (mode == OS_APPEND)
			{
				stream->iostream->stream = open(filename, O_RDWR | O_BINARY | O_RANDOM | O_CREAT, 0600);

				if (stream->iostream->stream <= 0)
				{
					free(stream->iostream->data);
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;

					return;
				}

				lseek(stream->iostream->stream, 0, SEEK_END);

				stream->iostream->size			= tell(stream->iostream->stream);
				stream->iostream->currentfilepos	= stream->iostream->size;
				stream->lastfilepos			= stream->iostream->size;
			}
			else
			{
				stream->iostream->stream = open(filename, O_RDWR | O_BINARY | O_RANDOM | O_CREAT | O_TRUNC, 0600);

				if (stream->iostream->stream <= 0)
				{
					free(stream->iostream->data);
					stream->iostream->streamtype = STREAM_NONE;

					return;
				}
			}

			stream->iostream->isopen = true;

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
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;

					return;
				}
			}

			initialized++;
#endif

			stream->iostream->streamtype		= STREAM_CLIENT;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;

				return;
			}

			host = gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) port);

			if (connect(stream->iostream->socket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			stream->iostream->connected = true;

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

			stream->iostream->streamtype		= STREAM_SOCKS4_CLIENT;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(stream->iostream->data);
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(stream->iostream->socket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
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

				send(stream->iostream->socket, (char *) socksdata, 9, 0);
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

				send(stream->iostream->socket, (char *) socksdata, 9, 0);
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

				send(stream->iostream->socket, (char *) socksdata, 10 + strlen(server), 0);
			}

			while (recbytes != 8)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
			}

			if (socksdata[1] != 90)	// proxy rejected request
			{
				free(stream->iostream->data);
				free(socksdata);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			free(socksdata);

			stream->iostream->connected = true;

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

			stream->iostream->streamtype		= STREAM_SOCKS4_BIND;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(stream->iostream->data);
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(stream->iostream->socket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
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

				send(stream->iostream->socket, (char *) socksdata, 9, 0);
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

				send(stream->iostream->socket, (char *) socksdata, 9, 0);
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

				send(stream->iostream->socket, (char *) socksdata, 10 + strlen(server), 0);
			}

			while (recbytes != 8)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
			}

			if (socksdata[1] != 90)	// proxy rejected request
			{
				free(stream->iostream->data);
				free(socksdata);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
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

			stream->iostream->streamtype		= STREAM_SOCKS5_CLIENT;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(stream->iostream->data);
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(stream->iostream->socket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			if (subtype == STREAM_SOCKS5_NOAUTH)
			{
				socksdata = malloc(3);

				socksdata[0] = 5;
				socksdata[1] = 1;
				socksdata[2] = 0;

				send(stream->iostream->socket, (char *) socksdata, 3, 0);

				while (recbytes != 2)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication
				{
					free(stream->iostream->data);
					free(socksdata);
					stream->iostream->streamtype = STREAM_NONE;
					OutStream_CloseSocket(stream, stream->iostream->socket);
					stream->iostream->isopen = false;
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

				send(stream->iostream->socket, (char *) socksdata, 4, 0);

				while (recbytes != 2)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication, but doesn't support username/password
				{
					free(stream->iostream->data);
					free(socksdata);
					stream->iostream->streamtype = STREAM_NONE;
					OutStream_CloseSocket(stream, stream->iostream->socket);
					stream->iostream->isopen = false;
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

					send(stream->iostream->socket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

					recbytes = 0;

					while (recbytes != 2)
					{
						recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
					}

					if (socksdata[1] != 0)	// proxy rejected username/password
					{
						free(stream->iostream->data);
						free(socksdata);
						stream->iostream->streamtype = STREAM_NONE;
						OutStream_CloseSocket(stream, stream->iostream->socket);
						stream->iostream->isopen = false;
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

			send(stream->iostream->socket, (char *) socksdata, 7 + strlen(server), 0);

			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
			}

			if (socksdata[1] != 0)	// an error occurred
			{
				free(stream->iostream->data);
				free(socksdata);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			if (socksdata[3] == 1)
			{
				recbytes = 0;

				while (recbytes != 5)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
				}
			}
			else if (socksdata[3] == 3)
			{
				recbytes	= 0;
				neededbytes	= socksdata[4] + 2;

				while (recbytes != neededbytes)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
				}
			}

			free(socksdata);

			stream->iostream->connected = true;

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

			stream->iostream->streamtype		= STREAM_SOCKS5_BIND;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
			if (initialized == 0)
			{
				WSAStartup(wVersionRequested, &wsaData);

				if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
				{
					free(stream->iostream->data);
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr		= *((struct in_addr *) *host->h_addr_list);
			saddr.sin_port		= htons((short) socksport);

			if (connect(stream->iostream->socket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			if (subtype == STREAM_SOCKS5_NOAUTH)
			{
				socksdata = malloc(3);

				socksdata[0] = 5;
				socksdata[1] = 1;
				socksdata[2] = 0;

				send(stream->iostream->socket, (char *) socksdata, 3, 0);

				while (recbytes != 2)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy requires authentication
				{
					free(stream->iostream->data);
					free(socksdata);
					stream->iostream->streamtype = STREAM_NONE;
					OutStream_CloseSocket(stream, stream->iostream->socket);
					stream->iostream->isopen = false;
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

				send(stream->iostream->socket, (char *) socksdata, 4, 0);

				while (recbytes != 2)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] == 255)	// proxy needs authentication, but doesn't support username/password
				{
					free(stream->iostream->data);
					free(socksdata);
					stream->iostream->streamtype = STREAM_NONE;
					OutStream_CloseSocket(stream, stream->iostream->socket);
					stream->iostream->isopen = false;
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

					send(stream->iostream->socket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

					recbytes = 0;

					while (recbytes != 2)
					{
						recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
					}

					if (socksdata[1] != 0)	// proxy rejected username/password
					{
						free(stream->iostream->data);
						free(socksdata);
						stream->iostream->streamtype = STREAM_NONE;
						OutStream_CloseSocket(stream, stream->iostream->socket);
						stream->iostream->isopen = false;
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

			send(stream->iostream->socket, (char *) socksdata, 7 + strlen(server), 0);

			free(socksdata);

			socksdata = malloc(300);

			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
			}

			if (socksdata[1] != 0)	// an error occurred
			{
				free(stream->iostream->data);
				free(socksdata);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			if (socksdata[3] == 1)
			{
				recbytes = 0;

				while (recbytes != 5)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
				}
			}
			else if (socksdata[3] == 3)
			{
				recbytes	= 0;
				neededbytes	= socksdata[4] + 2;

				while (recbytes != neededbytes)
				{
					recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
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

			stream->iostream->streamtype		= STREAM_POSIX;
			stream->iostream->stream		= file;
			stream->iostream->closefile		= false;
			stream->iostream->currentfilepos	= tell(stream->iostream->stream);
			stream->lastfilepos			= 0;

			lseek(stream->iostream->stream, 0, SEEK_END);

			stream->iostream->size		= tell(stream->iostream->stream);

			lseek(stream->iostream->stream, stream->iostream->currentfilepos, SEEK_SET);

			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->isopen	= true;

			return;
#ifndef MSDOS
		case STREAM_SOCKET:
			va_start(ap, type);

			opensocket = va_arg(ap, SOCKET);

			va_end(ap);

			stream->iostream->streamtype		= STREAM_SOCKET;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;
			stream->iostream->socket		= opensocket;
			stream->iostream->connected		= true;
			stream->iostream->closefile		= false;

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
					stream->iostream->streamtype	= STREAM_NONE;
					stream->iostream->isopen	= false;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->streamtype		= STREAM_SERVER;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);

			stream->iostream->localSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->localSocket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->localSocket);
				return;
			}

			saddr.sin_family	= AF_INET;
			saddr.sin_addr.s_addr	= INADDR_ANY;
			saddr.sin_port		= htons((short) port);

			if (bind(stream->iostream->localSocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->localSocket);
				return;
			}

			if (listen(stream->iostream->localSocket, SOMAXCONN) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				OutStream_CloseSocket(stream, stream->iostream->localSocket);
				return;
			}

			return;
#endif
		case STREAM_ANSI:
			va_start(ap, type);

			openfile = va_arg(ap, FILE *);

			va_end(ap);

			stream->iostream->streamtype		= STREAM_ANSI;
			stream->iostream->hlstream		= openfile;
			stream->iostream->closefile		= false;
			stream->iostream->currentfilepos	= ftell(stream->iostream->hlstream);
			stream->lastfilepos			= 0;

			fseek(stream->iostream->hlstream, 0, SEEK_END);

			stream->iostream->size		= ftell(stream->iostream->hlstream);

			fseek(stream->iostream->hlstream, stream->iostream->currentfilepos, SEEK_SET);

			stream->iostream->packagesize	= 1; // low package size, 'cause openfile could point at the console or so
			stream->iostream->stdpacksize	= stream->iostream->packagesize;
			stream->iostream->origpacksize	= stream->iostream->packagesize;
			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->isopen	= true;

			return;
#if defined __WIN32__
		case STREAM_WINDOWS:
			va_start(ap, type);

			file = va_arg(ap, int);

			va_end(ap);

			stream->iostream->streamtype		= STREAM_WINDOWS;
			stream->iostream->stream		= file;
			stream->iostream->closefile		= false;
			stream->iostream->currentfilepos	= _llseek(stream->iostream->stream, 0, FILE_CURRENT);
			stream->lastfilepos			= 0;

			_llseek(stream->iostream->stream, 0, FILE_END);

			stream->iostream->size		= _llseek(stream->iostream->stream, 0, FILE_END);

			_llseek(stream->iostream->stream, stream->iostream->currentfilepos, FILE_BEGIN);

			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->isopen	= true;

			return;
#endif
		case STREAM_BUFFER:
			va_start(ap, type);

			outbuffer	= va_arg(ap, void *);
			bufsize		= va_arg(ap, long);

			va_end(ap);

			stream->iostream->streamtype	= STREAM_BUFFER;
			stream->lastfilepos		= 0;
			stream->iostream->packagesize	= 1;
			stream->iostream->stdpacksize	= stream->iostream->packagesize;
			stream->iostream->origpacksize	= stream->iostream->packagesize;
			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->closefile	= false;
			stream->iostream->isopen	= true;
			stream->iostream->buffersize	= bufsize;
			stream->iostream->buffer	= outbuffer;

			return;
		case STREAM_STREAM:
			va_start(ap, type);

			in = va_arg(ap, InStream *);

			va_end(ap);

			if (in->iostream->streamtype == STREAM_SERVER || in->iostream->streamtype == STREAM_SOCKS5_BIND || in->iostream->streamtype == STREAM_SOCKS4_BIND)								return;
			if ((!in->iostream->isopen && !(in->iostream->streamtype == STREAM_SERVER || in->iostream->streamtype == STREAM_SOCKS5_BIND || in->iostream->streamtype == STREAM_SOCKS4_BIND)) || in->iostream->crosslinked)	return;

			stream->iostream->streamtype = STREAM_STREAM;

			stream->iostream->crosslinked = true;
			stream->instream = (void *) in;
			((InStream *) stream->instream)->outstream = (void *) stream;
			((InStream *) stream->instream)->iostream->crosslinked = true;

			if (((InStream *) stream->instream)->iostream->streamtype == STREAM_ANSI) // instream is an ANSI file handle
			{
				stream->iostream->streamtype		= STREAM_ANSI;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->hlstream		= ((InStream *) stream->instream)->iostream->hlstream;
				stream->iostream->currentfilepos	= ((InStream *) stream->instream)->iostream->currentfilepos;
				stream->lastfilepos			= stream->iostream->currentfilepos;
				stream->iostream->packagesize		= 1;
				stream->iostream->stdpacksize		= stream->iostream->packagesize;
				stream->iostream->origpacksize		= stream->iostream->packagesize;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->size			= ((InStream *) stream->instream)->iostream->origsize;

				return;
			}
#if defined __WIN32__
			else if (((InStream *) stream->instream)->iostream->streamtype == STREAM_WINDOWS)
			{
				stream->iostream->streamtype		= STREAM_WINDOWS;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->stream		= ((InStream *) stream->instream)->iostream->stream;
				stream->iostream->currentfilepos	= ((InStream *) stream->instream)->iostream->currentfilepos;
				stream->lastfilepos			= stream->iostream->currentfilepos;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->size			= ((InStream *) stream->instream)->iostream->origsize;

				return;
			}
#endif
			else if (((InStream *) stream->instream)->iostream->socketstream) // instream is a socket
			{
				initialized++;

				stream->iostream->streamtype		= ((InStream *) stream->instream)->iostream->streamtype;
				stream->iostream->socketstream		= true;
				stream->iostream->faraddr		= ((InStream *) stream->instream)->iostream->faraddr;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= in->iostream->isopen;
				stream->iostream->connected		= ((InStream *) stream->instream)->iostream->connected;
				stream->iostream->socket		= ((InStream *) stream->instream)->iostream->socket;
				stream->iostream->localSocket		= ((InStream *) stream->instream)->iostream->localSocket;
				stream->iostream->currentfilepos	= -1;
				stream->lastfilepos			= 0;
				stream->iostream->data			= malloc(stream->iostream->packagesize);

				return;
			}
			else if (((InStream *) stream->instream)->iostream->buffersize != -1) // instream is a mem buffer
			{
				stream->iostream->streamtype	= STREAM_BUFFER;
				stream->iostream->buffersize	= ((InStream *) stream->instream)->iostream->buffersize;
				stream->iostream->closefile	= false;
				stream->iostream->isopen	= true;
				stream->iostream->buffer	= ((InStream *) stream->instream)->iostream->buffer;
				stream->lastfilepos		= 0;
				stream->iostream->packagesize	= 1;
				stream->iostream->stdpacksize	= stream->iostream->packagesize;
				stream->iostream->origpacksize	= stream->iostream->packagesize;
				stream->iostream->data		= malloc(stream->iostream->packagesize);
				stream->iostream->size		= stream->iostream->buffersize;

				return;
			}
			else // instream is a posix file handle
			{
				stream->iostream->streamtype		= STREAM_POSIX;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->stream		= ((InStream *) stream->instream)->iostream->stream;
				stream->iostream->currentfilepos	= ((InStream *) stream->instream)->iostream->currentfilepos;
				stream->lastfilepos			= stream->iostream->currentfilepos;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->size			= ((InStream *) stream->instream)->iostream->origsize;

				return;
			}

			return;
		case STREAM_CHILD:
			va_start(ap, type);

			out = va_arg(ap, OutStream *);

			va_end(ap);

			initialized++;

			stream->iostream->streamtype		= STREAM_CHILD;
			stream->iostream->socketstream		= true;
			stream->iostream->isopen		= true;
			stream->iostream->connected		= true;
			stream->iostream->socket		= out->iostream->socket;
			stream->iostream->currentfilepos	= -1;
			stream->lastfilepos			= 0;
			stream->iostream->data			= malloc(stream->iostream->packagesize);

			return;
		default:
			return;
	}
}

OutStream *OutStream_WaitForClient(OutStream *stream)
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

	if (stream->iostream->streamtype != STREAM_SERVER && stream->iostream->streamtype != STREAM_SOCKS5_BIND && stream->iostream->streamtype != STREAM_SOCKS4_BIND) return NULL;

	if (stream->iostream->streamtype == STREAM_SERVER)
	{
		stream->iostream->socket = accept(stream->iostream->localSocket, (struct sockaddr *) &stream->iostream->faraddr, &faraddrsize);

		if (stream->iostream->socket == (SOCKET)(~0))
		{
			return NULL;
		}
		else
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_CHILD, stream);

			out->iostream->faraddr = stream->iostream->faraddr;

			return out;
		}
	}
	else if (stream->iostream->streamtype == STREAM_SOCKS4_BIND)
	{
		socksdata = malloc(8);

		while (recbytes != 8)
		{
			recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 90)	// an error occurred
		{
			free(stream->iostream->data);
			free(socksdata);
			stream->iostream->streamtype = STREAM_NONE;
			OutStream_CloseSocket(stream, stream->iostream->socket);
			stream->iostream->isopen = false;
			return NULL;
		}

		free(socksdata);

		OutStream_New(out);
		OutStream_Connect(out, STREAM_CHILD, stream);

		return out;
	}
	else if (stream->iostream->streamtype == STREAM_SOCKS5_BIND)
	{
		socksdata = malloc(300);

		while (recbytes != 5)
		{
			recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			free(stream->iostream->data);
			free(socksdata);
			stream->iostream->streamtype = STREAM_NONE;
			OutStream_CloseSocket(stream, stream->iostream->socket);
			stream->iostream->isopen = false;
			return NULL;
		}

		if (socksdata[3] == 1)
		{
			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}
		else if (socksdata[3] == 3)
		{
			recbytes	= 0;
			neededbytes	= socksdata[4] + 2;

			while (recbytes != neededbytes)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
			}
		}

		free(socksdata);

		OutStream_New(out);
		OutStream_Connect(out, STREAM_CHILD, stream);

		return out;
	}

	return NULL;
#endif
}

void OutStream_Flush(OutStream *stream)
{
	int	 ps		= stream->iostream->packagesize;
	int	 oldcpos	= stream->iostream->currentpos;
	int	 i;

	if (!stream->iostream->isopen)		return;
	if (stream->iostream->currentpos <= 0)	return;

	if (stream->iostream->pbd) OutStream_CompletePBD(stream);

	if (stream->iostream->filter != stream->iostream->NULLFILTER && stream->iostream->filter->packsize > 0)
	{
		for (i = 0; i < (stream->iostream->packagesize - oldcpos); i++)
		{
			OutStream_OutputNumber(stream, 0, 1);
		}
	}
	else
	{
		stream->iostream->packagesize = stream->iostream->currentpos;

		OutStream_WriteData(stream);

		stream->iostream->packagesize = ps;
	}
}

void OutStream_WriteData(OutStream *stream)
{
	unsigned char	*databuffer;
	int		 encsize = 0;

	if (!stream->iostream->isopen)						return;
	if (stream->iostream->currentpos < stream->iostream->packagesize)	return;

	if (stream->iostream->filter->packsize == -1)
	{
		databuffer = malloc(stream->iostream->packagesize);

		memcpy((void *) databuffer, (void *) stream->iostream->data, stream->iostream->packagesize);

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize + DEFAULT_PACKAGE_SIZE);

		memcpy((void *) stream->iostream->data, (void *) databuffer, stream->iostream->packagesize);

		free(databuffer);

		stream->iostream->packagesize += DEFAULT_PACKAGE_SIZE;
		stream->iostream->stdpacksize = stream->iostream->packagesize;

		return;
	}

	if (stream->iostream->streamtype == STREAM_ANSI)
	{
		fseek(stream->iostream->hlstream, stream->lastfilepos, SEEK_SET);

		stream->iostream->filter->encodeproc((void *) stream->iostream->filter, &stream->iostream->data, stream->iostream->packagesize, &encsize, stream->iostream->filter->pointer);

		fwrite((void *) stream->iostream->data, encsize, 1, stream->iostream->hlstream);
		fflush(stream->iostream->hlstream);

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		stream->iostream->currentpos -= stream->iostream->packagesize;
		if (stream->iostream->size == stream->iostream->currentfilepos) stream->iostream->size -= (stream->iostream->packagesize - encsize);
		stream->iostream->currentfilepos -= (stream->iostream->packagesize - encsize);
		stream->lastfilepos += encsize;
	}
#if defined __WIN32__
	else if (stream->iostream->streamtype == STREAM_WINDOWS)
	{
		_llseek(stream->iostream->stream, stream->lastfilepos, FILE_BEGIN);

		stream->iostream->filter->encodeproc((void *) stream->iostream->filter, &stream->iostream->data, stream->iostream->packagesize, &encsize, stream->iostream->filter->pointer);

		_hwrite(stream->iostream->stream, (char *) stream->iostream->data, encsize);

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		stream->iostream->currentpos -= stream->iostream->packagesize;
		if (stream->iostream->size == stream->iostream->currentfilepos) stream->iostream->size -= (stream->iostream->packagesize - encsize);
		stream->iostream->currentfilepos -= (stream->iostream->packagesize - encsize);
		stream->lastfilepos += encsize;
	}
#endif
#ifndef MSDOS
	else if (stream->iostream->socketstream)
	{
		if (send(stream->iostream->socket, (char *) stream->iostream->data, stream->iostream->packagesize, 0) == -1)
		{							// looks like other end has been closed
			free(stream->iostream->data);
			stream->iostream->connected	= false;
			stream->iostream->isopen	= false;
			stream->iostream->streamtype	= STREAM_NONE;

			OutStream_CloseSocket(stream, stream->iostream->socket);
		}

		stream->iostream->currentpos -= stream->iostream->packagesize;
	}
#endif
	else if (stream->iostream->buffersize == -1)
	{
		lseek(stream->iostream->stream, stream->lastfilepos, SEEK_SET);

		stream->iostream->filter->encodeproc((void *) stream->iostream->filter, &stream->iostream->data, stream->iostream->packagesize, &encsize, stream->iostream->filter->pointer);

		write(stream->iostream->stream, stream->iostream->data, encsize);

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		stream->iostream->currentpos -= stream->iostream->packagesize;
		if (stream->iostream->size == stream->iostream->currentfilepos) stream->iostream->size -= (stream->iostream->packagesize - encsize);
		stream->iostream->currentfilepos -= (stream->iostream->packagesize - encsize);
		stream->lastfilepos += encsize;
	}
	else
	{
		stream->iostream->filter->encodeproc((void *) stream->iostream->filter, &stream->iostream->data, stream->iostream->packagesize, &encsize, stream->iostream->filter->pointer);

		if (encsize <= (stream->iostream->buffersize - stream->iostream->bufferpos))
		{
			memcpy((void *) ((unsigned char *) stream->iostream->buffer + stream->iostream->bufferpos), (void *) stream->iostream->data, encsize);

			stream->iostream->bufferpos += encsize;
		}
		else
		{
			memcpy((void *) ((unsigned char *) stream->iostream->buffer + stream->iostream->bufferpos), (void *) stream->iostream->data, (stream->iostream->buffersize - 1) - stream->iostream->bufferpos);

			stream->iostream->bufferpos = stream->iostream->buffersize - 1;
		}

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		stream->iostream->currentpos -= stream->iostream->packagesize;
		if (stream->iostream->size == stream->iostream->currentfilepos) stream->iostream->size -= (stream->iostream->packagesize - encsize);
		stream->iostream->currentfilepos -= (stream->iostream->packagesize - encsize);
	}
}

bool OutStream_OutputNumber(OutStream *stream, long number, int bytes)
{
	int	 i;

	if (!stream->iostream->isopen)	return false;
	if (bytes > 4)			return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	for (i = 0; i < bytes; i++)
	{
		if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

		stream->iostream->data[stream->iostream->currentpos] = GetByte(number, i);
		if (stream->iostream->currentfilepos == stream->iostream->size) stream->iostream->size++;
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
	}

	if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

	if (stream->iostream->socketstream && !stream->iostream->holdpbd) OutStream_Flush(stream);

	return true;
}

bool OutStream_OutputNumberRaw(OutStream *stream, long number, int bytes)
{
	int	 i;

	if (!stream->iostream->isopen)	return false;
	if (bytes > 4)			return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	for (i = bytes - 1; i >= 0; i--)
	{
		if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

		stream->iostream->data[stream->iostream->currentpos] = GetByte(number, i);
		if (stream->iostream->currentfilepos == stream->iostream->size) stream->iostream->size++;
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
	}

	if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

	if (stream->iostream->socketstream && !stream->iostream->holdpbd) OutStream_Flush(stream);

	return true;
}

bool OutStream_OutputNumberVAX(OutStream *stream, long number, int bytes)
{
	int	 i;

	if (!stream->iostream->isopen)	return false;
	if (bytes > 4)			return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	number <<= 8 * (4 - bytes);

	for (i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

			stream->iostream->data[stream->iostream->currentpos] = GetByte(number, (3 - i) ^ 1);
			if (stream->iostream->currentfilepos == stream->iostream->size) stream->iostream->size++;
			stream->iostream->currentpos++;
			if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
		}
	}

	if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);

	if (stream->iostream->socketstream && !stream->iostream->holdpbd) OutStream_Flush(stream);

	return true;
}

bool OutStream_OutputNumberPBD(OutStream *stream, long number, int bits)
{
	unsigned char	 out;
	int		 i;
	int		 j;

	if (!stream->iostream->isopen)	return false;
	if (bits > 32)			return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (!stream->iostream->pbd) OutStream_InitPBD(stream);

	for (j = 0; j < bits; j++)
	{
		stream->iostream->pbdbuffer[stream->iostream->pbdlen] = GetBit(number, j);
		stream->iostream->pbdlen++;
	}

	while (stream->iostream->pbdlen >= 8)
	{
		out = 0;

		for (i = 0; i < 8; i++)
		{
			out = out | (stream->iostream->pbdbuffer[i] << i);
		}

		stream->iostream->pbdlen = stream->iostream->pbdlen - 8;

		for (j = 0; j < stream->iostream->pbdlen; j++)
		{
			stream->iostream->pbdbuffer[j] = stream->iostream->pbdbuffer[j+8];
		}

		stream->iostream->data[stream->iostream->currentpos] = out;
		if (stream->iostream->currentfilepos == stream->iostream->size) stream->iostream->size++;
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
		if (stream->iostream->currentpos >= stream->iostream->packagesize) OutStream_WriteData(stream);
	}

	return true;
}

bool OutStream_OutputString(OutStream *stream, const char *string)
{
	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!stream->iostream->isopen) return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	if (string == NULL) return false;

	while (bytesleft)
	{
		amount = ((stream->iostream->packagesize - stream->iostream->currentpos)<(bytesleft))?(stream->iostream->packagesize - stream->iostream->currentpos):(bytesleft);

		memcpy((void *) (stream->iostream->data + stream->iostream->currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		stream->iostream->currentpos += amount;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos += amount;

		if (stream->iostream->size < stream->iostream->currentfilepos) stream->iostream->size = stream->iostream->currentfilepos;

		OutStream_WriteData(stream);
	}

	if (stream->iostream->socketstream) OutStream_Flush(stream);

	return true;
}

bool OutStream_OutputLine(OutStream *stream, const char *string)
{
	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!stream->iostream->isopen) return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	if (string == NULL) return false;

	while (bytesleft)
	{
		amount = ((stream->iostream->packagesize - stream->iostream->currentpos)<(bytesleft))?(stream->iostream->packagesize - stream->iostream->currentpos):(bytesleft);

		memcpy((void *) (stream->iostream->data + stream->iostream->currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		stream->iostream->currentpos += amount;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos += amount;

		if (stream->iostream->size < stream->iostream->currentfilepos) stream->iostream->size = stream->iostream->currentfilepos;

		OutStream_WriteData(stream);
	}


#if (defined __WIN32__ || defined MSDOS) && !defined __CYGWIN32__
	OutStream_OutputNumber(stream, 13, 1);
#endif

	OutStream_OutputNumber(stream, 10, 1);

	if (stream->iostream->socketstream) OutStream_Flush(stream);

	return true;
}

bool OutStream_OutputData(OutStream *stream, const void *pointer, int bytes)
{
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!stream->iostream->isopen) return false;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	if (pointer == NULL) return false;

	while (bytesleft)
	{
		amount = ((stream->iostream->packagesize - stream->iostream->currentpos)<(bytesleft))?(stream->iostream->packagesize - stream->iostream->currentpos):(bytesleft);

		memcpy((void *) (stream->iostream->data + stream->iostream->currentpos), (void *) ((unsigned char *) pointer + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		stream->iostream->currentpos += amount;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos += amount;

		if (stream->iostream->size < stream->iostream->currentfilepos) stream->iostream->size = stream->iostream->currentfilepos;

		OutStream_WriteData(stream);
	}

	if (stream->iostream->socketstream) OutStream_Flush(stream);

	return true;
}

void OutStream_InitPBD(OutStream *stream)
{
	stream->iostream->pbdlen	= 0;
	stream->iostream->pbd		= 1;
}

void OutStream_CompletePBD(OutStream *stream)
{
	int	 out = 0;
	int	 i;

	if (stream->iostream->pbdlen > 0)
	{
		for (i = 0; i < 8; i++)
		{
			if (i < stream->iostream->pbdlen) out = out | (stream->iostream->pbdbuffer[i] << i);
		}
		stream->iostream->holdpbd = true;
		OutStream_OutputNumber(stream, out, 1);
		stream->iostream->holdpbd = false;
	}

	if (stream->iostream->socketstream) OutStream_Flush(stream);

	stream->iostream->pbd = 0;
}

bool OutStream_SetPackageSize(OutStream *stream, int newPackagesize)
{
	if (!stream->iostream->isopen)		return false;
	if (!stream->iostream->allowpackset)	return false;
	if (newPackagesize <= 0)		return false;

	OutStream_Flush(stream);

	stream->lastfilepos = stream->iostream->currentfilepos;

	free(stream->iostream->data);

	stream->iostream->data = malloc(newPackagesize);

	stream->iostream->packagesize = newPackagesize;
	stream->iostream->stdpacksize = stream->iostream->packagesize;

	return true;
}

bool OutStream_SetFilter(OutStream *stream, IOLibFilter *newFilter)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	stream->iostream->allowpackset = true;

	if (newFilter->packsize > 0)
	{
		OutStream_SetPackageSize(stream, newFilter->packsize);	// package size must be eqv filter size

		stream->iostream->allowpackset = false;
	}
	else if (newFilter->packsize == -1)
	{
		OutStream_SetPackageSize(stream, DEFAULT_PACKAGE_SIZE);

		stream->iostream->allowpackset = false;
	}

	stream->iostream->filter = newFilter;

	return true;
}

bool OutStream_RemoveFilter(OutStream *stream)
{
	int	 oldcpos;
	int	 i;

	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) OutStream_CompletePBD(stream);

	oldcpos = stream->iostream->currentpos;

	if (stream->iostream->filter->packsize > 0 && stream->iostream->currentpos != 0)
	{
		for (i = 0; i < (stream->iostream->packagesize - oldcpos); i++)
		{
			OutStream_OutputNumber(stream, 0, 1);
		}
	}
	else if (stream->iostream->filter->packsize == -1)
	{
		stream->iostream->filter->packsize = 0;

		stream->iostream->allowpackset = true;

		OutStream_Flush(stream);

		stream->iostream->filter->packsize = -1;
	}

	stream->iostream->allowpackset = true;

	stream->iostream->filter = stream->iostream->NULLFILTER;

	OutStream_SetPackageSize(stream, stream->iostream->origpacksize);

	return true;
}

bool OutStream_Close(OutStream *stream)
{
	if (!stream->iostream->isopen && stream->iostream->streamtype == STREAM_NONE) return false;

	if (stream->iostream->filter != stream->iostream->NULLFILTER) OutStream_RemoveFilter(stream);

	OutStream_Flush(stream);

	if (stream->iostream->crosslinked)
	{
		if (stream->iostream->closefile)
		{
			((InStream *) stream->instream)->iostream->closefile = true;
			if (stream->iostream->streamtype == STREAM_FILE) ((InStream *) stream->instream)->iostream->streamtype = STREAM_FILE;
		}

		stream->iostream->closefile = false;
	}

	if (!stream->iostream->socketstream)
	{
		if (stream->iostream->closefile) close(stream->iostream->stream);
	}
#ifndef MSDOS
	else
	{
		if (stream->iostream->closefile)
		{
			if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND)
			{
#ifdef __WIN32__
				closesocket(stream->iostream->localSocket);
				if (stream->iostream->connected) closesocket(stream->iostream->socket);
#else
				close(stream->iostream->localSocket);
				if (stream->iostream->connected) close(stream->iostream->socket);
#endif
			}
			else
			{
#ifdef __WIN32__
				closesocket(stream->iostream->socket);
#else
				close(stream->iostream->socket);
#endif
			}
		}
	}
#endif

	if (stream->iostream->socketstream) initialized--;

	if (stream->iostream->socketstream && initialized == 0)
	{
#if defined __WIN32__ && !defined __CYGWIN32__
		WSACleanup();
#endif
	}

	free(stream->iostream->data);
	stream->iostream->data = NULL;

	stream->iostream->isopen 	= false;
	stream->iostream->streamtype	= STREAM_NONE;

	return true;
}

long OutStream_GetPos(OutStream *stream)
{
	if (!stream->iostream->isopen)		return -1;
	if (stream->iostream->socketstream)	return -1;

	return stream->iostream->currentfilepos;
}

bool OutStream_Seek(OutStream *stream, long position)
{
	if(!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	OutStream_Flush(stream);

	if (stream->iostream->buffersize == -1 && !(stream->iostream->streamtype == STREAM_ANSI) && !(stream->iostream->streamtype == STREAM_WINDOWS))	lseek(stream->iostream->stream, position, SEEK_SET);
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_ANSI)							fseek(stream->iostream->hlstream, position, SEEK_SET);
#ifdef __WIN32__
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_WINDOWS)							_llseek(stream->iostream->stream, position, FILE_BEGIN);
#endif

	stream->iostream->currentfilepos	= position;
	stream->iostream->bufferpos		= position;
	stream->lastfilepos			= position;
	stream->iostream->currentpos		= 0;

	return true;
}

bool OutStream_RelSeek(OutStream *stream, long offset)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	OutStream_Flush(stream);

	if (stream->iostream->buffersize == -1 && !(stream->iostream->streamtype == STREAM_ANSI) && !(stream->iostream->streamtype == STREAM_WINDOWS))	lseek(stream->iostream->stream, stream->iostream->currentfilepos + offset, SEEK_SET);
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_ANSI)							fseek(stream->iostream->hlstream, stream->iostream->currentfilepos + offset, SEEK_SET);
#ifdef __WIN32__
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_WINDOWS)							_llseek(stream->iostream->stream, stream->iostream->currentfilepos + offset, FILE_BEGIN);
#endif

	stream->iostream->currentfilepos	+= offset;
	stream->iostream->bufferpos		+= offset;
	stream->lastfilepos			= stream->iostream->currentfilepos;
	stream->iostream->currentpos		= 0;

	return true;
}

int OutStream_GetStreamType(OutStream *stream)
{
	return stream->iostream->streamtype;
}

long OutStream_Size(OutStream *stream)
{
	if (!stream->iostream->isopen)		return -1;
	if (stream->iostream->socketstream)	return -1;

	return stream->iostream->size;
}

bool OutStream_SetMode(OutStream *stream, long mode)
{
	unsigned long	 var;

	if (!stream->iostream->isopen)		return false;
	if (!stream->iostream->socketstream)	return false;

#if defined MSDOS | defined __BEOS__ | defined __svr4__
	return false;
#else
	switch (mode)
	{
		case MODE_SOCKET_BLOCKING:
			var = 0;

#if defined __WIN32__
			if (ioctlsocket(stream->iostream->socket, FIONBIO, &var) != 0) return false;
#else
			if (ioctl(stream->iostream->socket, FIONBIO, &var) != 0) return false;
#endif
			break;
		case MODE_SOCKET_NONBLOCKING:
			var = 1;

#if defined __WIN32__
			if (ioctlsocket(stream->iostream->socket, FIONBIO, &var) != 0) return false;
#else
			if (ioctl(stream->iostream->socket, FIONBIO, &var) != 0) return false;
#endif

			break;
		default:
			return false;
	}

	return true;
#endif
}

const char *OutStream_GetRemoteHostname(OutStream *stream)
{
#ifdef MSDOS
	return NULL;
#else
	if (stream->iostream->faraddr.sin_addr.s_addr == 0) return NULL;

	return gethostbyaddr((char *) &stream->iostream->faraddr.sin_addr.s_addr, 4, AF_INET)->h_name;
#endif
}

unsigned long OutStream_GetRemoteIP(OutStream *stream)
{
#ifdef MSDOS
	return 0;
#else
	if (stream->iostream->faraddr.sin_addr.s_addr == 0) return 0;

	return stream->iostream->faraddr.sin_addr.s_addr;
#endif
}

void OutStream_CloseSocket(OutStream *stream, SOCKET sock)
{
	if (!stream->iostream->socketstream) return;

#ifdef __WIN32__
	closesocket(sock);
#else
	close(sock);
#endif

	initialized--;

	if (initialized == 0)
	{
#if defined __WIN32__ && !defined __CYGWIN32__
		WSACleanup();
#endif
	}
}

#endif
