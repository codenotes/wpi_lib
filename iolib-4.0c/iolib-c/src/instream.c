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

#ifndef __IOLIB_INSTREAM_
#define __IOLIB_INSTREAM_

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

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

void InStream_Init_real(InStream *stream)
{
	stream->iostream = malloc(sizeof(IOLibStream));

	IOLibStream_Init(stream->iostream);
}

void InStream_Free_real(InStream *stream)
{
	if (stream->iostream->crosslinked)
	{
		((OutStream *) stream->outstream)->iostream->crosslinked	= false;
		((OutStream *) stream->outstream)->instream			= NULL;
	}

	InStream_Close(stream);	// Datei schließen (return egal)

	IOLibStream_Free(stream->iostream);

	free(stream->iostream);
}

void InStream_Connect(InStream *stream, int type, ...)
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
	OutStream		*out;
	InStream		*in;
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

			stream->iostream->streamtype	= STREAM_FILE;
			stream->iostream->currentpos	= DEFAULT_PACKAGE_SIZE;
			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->stream	= open(filename, O_RDWR | O_BINARY);

			if (stream->iostream->stream <= 0)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				return;
			}

			lseek(stream->iostream->stream, 0, SEEK_END);

			stream->iostream->size		= tell(stream->iostream->stream);
			stream->iostream->origsize	= stream->iostream->size;

			lseek(stream->iostream->stream, 0, SEEK_SET);

			stream->iostream->isopen	= true;

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
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

			stream->iostream->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->socket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host = gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
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
				InStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
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
				InStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host		= gethostbyname(proxy);
			server_hostent	= gethostbyname(server);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
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
				InStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
					InStream_CloseSocket(stream, stream->iostream->socket);
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
					InStream_CloseSocket(stream, stream->iostream->socket);
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
						InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
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
				InStream_CloseSocket(stream, stream->iostream->socket);
				stream->iostream->isopen = false;
				return;
			}

			host = gethostbyname(proxy);

			if (host == NULL)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
					InStream_CloseSocket(stream, stream->iostream->socket);
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
					InStream_CloseSocket(stream, stream->iostream->socket);
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
						InStream_CloseSocket(stream, stream->iostream->socket);
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
				InStream_CloseSocket(stream, stream->iostream->socket);
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
				neededbytes	 = 0;

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
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;

			lseek(stream->iostream->stream, 0, SEEK_END);

			stream->iostream->size			= tell(stream->iostream->stream);
			stream->iostream->origsize	= stream->iostream->size;

			lseek(stream->iostream->stream, stream->iostream->currentfilepos, SEEK_SET);

			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

			return;
#ifndef MSDOS
		case STREAM_SOCKET:
			va_start(ap, type);

			opensocket = va_arg(ap, SOCKET);

			va_end(ap);

			stream->iostream->streamtype		= STREAM_SOCKET;
			stream->iostream->socket		= opensocket;
			stream->iostream->closefile		= false;
			stream->iostream->socketstream		= true;
			stream->iostream->currentfilepos	= -1;
			stream->iostream->allowpackset		= false;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;
			stream->iostream->connected		= true;

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
					stream->iostream->streamtype = STREAM_NONE;
					return;
				}
			}

			initialized++;
#endif

			stream->iostream->streamtype		= STREAM_SERVER;
			stream->iostream->socketstream		= true;
			stream->iostream->allowpackset		= false;
			stream->iostream->currentfilepos	= -1;
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;
			stream->iostream->data			= malloc(stream->iostream->packagesize);

			stream->iostream->localSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (stream->iostream->localSocket == (SOCKET)(~0))
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->localSocket);
				return;
			}

			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = INADDR_ANY;
			saddr.sin_port = htons((short) port);

			if (bind(stream->iostream->localSocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->localSocket);
				return;
			}

			if (listen(stream->iostream->localSocket, SOMAXCONN) == -1)
			{
				free(stream->iostream->data);
				stream->iostream->streamtype = STREAM_NONE;
				InStream_CloseSocket(stream, stream->iostream->localSocket);
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
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;

			fseek(stream->iostream->hlstream, 0, SEEK_END);

			stream->iostream->size			= ftell(stream->iostream->hlstream);
			stream->iostream->origsize		= stream->iostream->size;

			fseek(stream->iostream->hlstream, stream->iostream->currentfilepos, SEEK_SET);

			stream->iostream->packagesize		= 1;
			stream->iostream->stdpacksize		= stream->iostream->packagesize;
			stream->iostream->origpacksize		= stream->iostream->packagesize;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

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
			stream->iostream->currentpos		= DEFAULT_PACKAGE_SIZE;

			_llseek(stream->iostream->stream, 0, FILE_END);

			stream->iostream->size			= _llseek(stream->iostream->stream, 0, FILE_END);
			stream->iostream->origsize		= stream->iostream->size;

			_llseek(stream->iostream->stream, stream->iostream->currentfilepos, FILE_BEGIN);

			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->isopen		= true;

			return;
#endif
		case STREAM_BUFFER:
			va_start(ap, type);

			inbuffer	= va_arg(ap, void *);
			bufsize		= va_arg(ap, long);

			va_end(ap);

			stream->iostream->streamtype	= STREAM_BUFFER;
			stream->iostream->buffer	= inbuffer;
			stream->iostream->buffersize	= bufsize;
			stream->iostream->packagesize	= 1;
			stream->iostream->stdpacksize	= stream->iostream->packagesize;
			stream->iostream->origpacksize	= stream->iostream->packagesize;
			stream->iostream->currentpos	= stream->iostream->packagesize;
			stream->iostream->size		= stream->iostream->buffersize;
			stream->iostream->origsize	= stream->iostream->size;
			stream->iostream->data		= malloc(stream->iostream->packagesize);
			stream->iostream->closefile	= false;
			stream->iostream->isopen	= true;

			return;
		case STREAM_STREAM:
			va_start(ap, type);

			out = va_arg(ap, OutStream *);

			va_end(ap);

			if (out->iostream->streamtype == STREAM_SERVER || out->iostream->streamtype == STREAM_SOCKS5_BIND || out->iostream->streamtype == STREAM_SOCKS4_BIND)									return;
			if ((!out->iostream->isopen && !(out->iostream->streamtype == STREAM_SERVER || out->iostream->streamtype == STREAM_SOCKS5_BIND || out->iostream->streamtype == STREAM_SOCKS4_BIND)) || out->iostream->crosslinked)	return;

			stream->iostream->streamtype = STREAM_STREAM;

			stream->iostream->crosslinked = true;
			stream->outstream = (void *) out;
			((OutStream *) stream->outstream)->instream = (void *) stream;
			((OutStream *) stream->outstream)->iostream->crosslinked = true;

			if (((OutStream *) stream->outstream)->iostream->streamtype == STREAM_ANSI) // outstream got an ANSI file handle
			{
				stream->iostream->streamtype		= STREAM_ANSI;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->hlstream		= ((OutStream *) stream->outstream)->iostream->hlstream;
				stream->iostream->currentfilepos	= ((OutStream *) stream->outstream)->iostream->currentfilepos;
				stream->iostream->packagesize		= 1;
				stream->iostream->stdpacksize		= stream->iostream->packagesize;
				stream->iostream->origpacksize		= stream->iostream->packagesize;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->currentpos		= stream->iostream->packagesize;
				stream->iostream->size			= ((OutStream *) stream->outstream)->iostream->size;
				stream->iostream->origsize		= stream->iostream->size;

				return;
			}
#if defined __WIN32__
			else if (((OutStream *) stream->outstream)->iostream->streamtype == STREAM_WINDOWS)
			{
				stream->iostream->streamtype		= STREAM_WINDOWS;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->stream		= ((OutStream *) stream->outstream)->iostream->stream;
				stream->iostream->currentfilepos	= ((OutStream *) stream->outstream)->iostream->currentfilepos;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->currentpos		= stream->iostream->packagesize;
				stream->iostream->size			= ((OutStream *) stream->outstream)->iostream->size;
				stream->iostream->origsize		= stream->iostream->size;

				return;
			}
#endif
			else if (((OutStream *) stream->outstream)->iostream->socketstream) // outstream is a socket
			{
				initialized++;

				stream->iostream->streamtype		= ((OutStream *) stream->outstream)->iostream->streamtype;
				stream->iostream->socketstream		= true;
				stream->iostream->faraddr		= ((OutStream *) stream->outstream)->iostream->faraddr;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= ((OutStream *) stream->outstream)->iostream->isopen;
				stream->iostream->connected		= ((OutStream *) stream->outstream)->iostream->connected;
				stream->iostream->socket		= ((OutStream *) stream->outstream)->iostream->socket;
				stream->iostream->localSocket		= ((OutStream *) stream->outstream)->iostream->localSocket;
				stream->iostream->allowpackset		= false;
				stream->iostream->currentfilepos	= -1;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->currentpos		= stream->iostream->packagesize;

				return;
			}
			else if (((OutStream *) stream->outstream)->iostream->buffersize != -1) // outstream is a mem buffer
			{
				stream->iostream->streamtype	= STREAM_BUFFER;
				stream->iostream->buffersize	= ((OutStream *) stream->outstream)->iostream->buffersize;
				stream->iostream->closefile	= false;
				stream->iostream->isopen	= true;
				stream->iostream->buffer	= ((OutStream *) stream->outstream)->iostream->buffer;
				stream->iostream->packagesize	= 1;
				stream->iostream->stdpacksize	= stream->iostream->packagesize;
				stream->iostream->origpacksize	= stream->iostream->packagesize;
				stream->iostream->data		= malloc(stream->iostream->packagesize);
				stream->iostream->currentpos	= stream->iostream->packagesize;
				stream->iostream->size		= stream->iostream->buffersize;
				stream->iostream->origsize	= stream->iostream->size;

				return;
			}
			else // outstream is a posix file handle
			{
				stream->iostream->streamtype		= STREAM_POSIX;
				stream->iostream->closefile		= false;
				stream->iostream->isopen		= true;
				stream->iostream->stream		= ((OutStream *) stream->outstream)->iostream->stream;
				stream->iostream->currentfilepos	= ((OutStream *) stream->outstream)->iostream->currentfilepos;
				stream->iostream->data			= malloc(stream->iostream->packagesize);
				stream->iostream->currentpos		= stream->iostream->packagesize;
				stream->iostream->size			= ((OutStream *) stream->outstream)->iostream->size;
				stream->iostream->origsize		= stream->iostream->size;

				return;
			}

			return;
		case STREAM_CHILD:
			va_start(ap, type);

			in = va_arg(ap, InStream *);

			va_end(ap);

			initialized++;

			stream->iostream->streamtype		= STREAM_CHILD;
			stream->iostream->socketstream		= true;
			stream->iostream->isopen		= true;
			stream->iostream->connected		= true;
			stream->iostream->socket		= in->iostream->socket;
			stream->iostream->allowpackset		= false;
			stream->iostream->currentfilepos	= -1;
			stream->iostream->data			= malloc(stream->iostream->packagesize);
			stream->iostream->currentpos		= stream->iostream->packagesize;

			return;
		default:
			return;
	}
}

InStream *InStream_WaitForClient(InStream *stream)
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
			InStream_New(in);
			InStream_Connect(in, STREAM_CHILD, stream);

			in->iostream->faraddr = stream->iostream->faraddr;

			return in;
		}
	}
	else if (stream->iostream->streamtype == STREAM_SOCKS4_BIND)
	{
		socksdata = malloc(8);

		while (recbytes != 8)
		{
			recbytes += recv(stream->iostream->socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			free(stream->iostream->data);
			free(socksdata);
			stream->iostream->streamtype = STREAM_NONE;
			InStream_CloseSocket(stream, stream->iostream->socket);
			stream->iostream->isopen = false;
			return NULL;
		}

		free(socksdata);

		InStream_New(in);
		InStream_Connect(in, STREAM_CHILD, stream);

		return in;
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
			InStream_CloseSocket(stream, stream->iostream->socket);
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

			while (recbytes != 5)
			{
				recbytes += recv(stream->iostream->socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}

		free(socksdata);

		InStream_New(in);
		InStream_Connect(in, STREAM_CHILD, stream);

		return in;
	}

	return NULL;
#endif
}

void InStream_ReadData(InStream *stream)
{
	int decsize = 0;

	if (!stream->iostream->isopen) return;

	if (stream->iostream->streamtype == STREAM_ANSI)
	{
		if (stream->iostream->filter != stream->iostream->NULLFILTER)
		{
			if (stream->iostream->filter->packsize != 0)	stream->iostream->packagesize = stream->iostream->filter->packsize;
			else						stream->iostream->packagesize = stream->iostream->stdpacksize;
		}
		else	stream->iostream->packagesize = stream->iostream->stdpacksize;

		stream->iostream->size			= stream->iostream->origsize;
		stream->iostream->currentfilepos	= stream->iostream->origfilepos;

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		fseek(stream->iostream->hlstream, stream->iostream->currentfilepos, SEEK_SET); // Do this because a crosslinked OutStream might have changed the file pointer

		fread((void *) stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)), 1, stream->iostream->hlstream);

		if (stream->iostream->packagesize <= stream->iostream->size-stream->iostream->currentfilepos || (stream->iostream->filter != stream->iostream->NULLFILTER && stream->iostream->filter->packsize == 0))
		{
			stream->iostream->origfilepos = stream->iostream->currentfilepos + stream->iostream->packagesize;

			stream->iostream->filter->decodeproc((void *) stream->iostream->filter, &stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)), &decsize, stream->iostream->filter->pointer);

			stream->iostream->packagesize	= decsize;
			stream->iostream->origsize	= stream->iostream->size;

			if (stream->iostream->packagesize + stream->iostream->currentfilepos > stream->iostream->size) stream->iostream->size = stream->iostream->packagesize + stream->iostream->currentfilepos;
		}

		stream->iostream->currentpos = 0;
	}
#if defined __WIN32__
	else if (stream->iostream->streamtype == STREAM_WINDOWS)
	{
		if (stream->iostream->filter != stream->iostream->NULLFILTER)
		{
			if (stream->iostream->filter->packsize != 0)	stream->iostream->packagesize = stream->iostream->filter->packsize;
			else						stream->iostream->packagesize = stream->iostream->stdpacksize;
		}
		else	stream->iostream->packagesize = stream->iostream->stdpacksize;

		stream->iostream->size			= stream->iostream->origsize;
		stream->iostream->currentfilepos	= stream->iostream->origfilepos;

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		_llseek(stream->iostream->stream, stream->iostream->currentfilepos, FILE_BEGIN); // Do this because a crosslinked OutStream might have changed the file pointer

		_hread(stream->iostream->stream, (void *) stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)));

		if (stream->iostream->packagesize <= stream->iostream->size-stream->iostream->currentfilepos || (stream->iostream->filter != stream->iostream->NULLFILTER && stream->iostream->filter->packsize == 0))
		{
			stream->iostream->origfilepos = stream->iostream->currentfilepos + stream->iostream->packagesize;

			stream->iostream->filter->decodeproc((void *) stream->iostream->filter, &stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)), &decsize, stream->iostream->filter->pointer);

			stream->iostream->packagesize	= decsize;
			stream->iostream->origsize	= stream->iostream->size;

			if (stream->iostream->packagesize + stream->iostream->currentfilepos > stream->iostream->size) stream->iostream->size = stream->iostream->packagesize + stream->iostream->currentfilepos;
		}

		stream->iostream->currentpos = 0;
	}
#endif
#ifndef MSDOS
	else if (stream->iostream->socketstream)
	{
		stream->iostream->packagesize = DEFAULT_PACKAGE_SIZE;
		stream->iostream->packagesize = recv(stream->iostream->socket, (char *) stream->iostream->data, stream->iostream->packagesize, 0);

		if (stream->iostream->packagesize == -1)
		{
			free(stream->iostream->data);
			stream->iostream->connected	= false;
			stream->iostream->isopen	= false;
			stream->iostream->streamtype	= STREAM_NONE;

			InStream_CloseSocket(stream, stream->iostream->socket);
		}

		stream->iostream->currentpos = 0;
	}
#endif
	else if (stream->iostream->buffersize == -1)
	{
		if (stream->iostream->filter != stream->iostream->NULLFILTER)
		{
			if (stream->iostream->filter->packsize != 0)	stream->iostream->packagesize = stream->iostream->filter->packsize;
			else						stream->iostream->packagesize = stream->iostream->stdpacksize;
		}
		else	stream->iostream->packagesize = stream->iostream->stdpacksize;

		stream->iostream->size			= stream->iostream->origsize;
		stream->iostream->currentfilepos	= stream->iostream->origfilepos;

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		lseek(stream->iostream->stream, stream->iostream->currentfilepos, SEEK_SET);

		read(stream->iostream->stream, stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)));

		if (stream->iostream->packagesize <= stream->iostream->size-stream->iostream->currentfilepos || (stream->iostream->filter != stream->iostream->NULLFILTER && stream->iostream->filter->packsize == 0))
		{
			stream->iostream->origfilepos = stream->iostream->currentfilepos + stream->iostream->packagesize;

			stream->iostream->filter->decodeproc((void *) stream->iostream->filter, &stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)), &decsize, stream->iostream->filter->pointer);

			stream->iostream->packagesize	= decsize;
			stream->iostream->origsize	= stream->iostream->size;

			if (stream->iostream->packagesize + stream->iostream->currentfilepos > stream->iostream->size) stream->iostream->size = stream->iostream->packagesize + stream->iostream->currentfilepos;
		}

		stream->iostream->currentpos = 0;
	}
	else
	{
		if (stream->iostream->filter != stream->iostream->NULLFILTER)
		{
			if (stream->iostream->filter->packsize != 0)	stream->iostream->packagesize = stream->iostream->filter->packsize;
			else						stream->iostream->packagesize = stream->iostream->stdpacksize;
		}
		else	stream->iostream->packagesize = stream->iostream->stdpacksize;

		stream->iostream->size			= stream->iostream->origsize;
		stream->iostream->currentfilepos	= stream->iostream->origfilepos;

		free(stream->iostream->data);

		stream->iostream->data = malloc(stream->iostream->packagesize);

		if (stream->iostream->packagesize <= (stream->iostream->buffersize - stream->iostream->bufferpos))
		{
			memcpy((void *) stream->iostream->data, (void *) ((unsigned char *) stream->iostream->buffer + stream->iostream->bufferpos), stream->iostream->packagesize);

			stream->iostream->bufferpos += stream->iostream->packagesize;
		}
		else
		{
			memcpy((void *) stream->iostream->data, (void *) ((unsigned char *) stream->iostream->buffer + stream->iostream->bufferpos), (stream->iostream->buffersize - 1) - stream->iostream->bufferpos);

			stream->iostream->bufferpos = stream->iostream->buffersize - 1;
		}

		if (stream->iostream->packagesize <= stream->iostream->size-stream->iostream->currentfilepos || (stream->iostream->filter != stream->iostream->NULLFILTER && stream->iostream->filter->packsize == 0))
		{
			stream->iostream->origfilepos = stream->iostream->currentfilepos + stream->iostream->packagesize;

			stream->iostream->filter->decodeproc((void *) stream->iostream->filter, &stream->iostream->data, ((stream->iostream->packagesize)<(stream->iostream->size-stream->iostream->currentfilepos)?(stream->iostream->packagesize):(stream->iostream->size-stream->iostream->currentfilepos)), &decsize, stream->iostream->filter->pointer);

			stream->iostream->packagesize	= decsize;
			stream->iostream->origsize	= stream->iostream->size;

			if (stream->iostream->packagesize + stream->iostream->currentfilepos > stream->iostream->size) stream->iostream->size = stream->iostream->packagesize + stream->iostream->currentfilepos;
		}

		stream->iostream->currentpos = 0;
	}
}

long InStream_InputNumber(InStream *stream, int bytes)	// Intel byte order DCBA
{
	long	 rval = 0;
	int	 i;

	if (!stream->iostream->isopen)	return -1;
	if (bytes > 4)			return -1;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return -1;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	for (i = 0; i < bytes; i++)
	{
		if (stream->iostream->currentfilepos >= stream->iostream->size) return -1;

		if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);
		if (stream->iostream->packagesize == 0) return -1;

		rval += stream->iostream->data[stream->iostream->currentpos] * (1 << (i * 8));
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
	}

	return rval;
}

long InStream_InputNumberRaw(InStream *stream, int bytes)	// Raw byte order ABCD
{
	long	 rval = 0;
	int	 i;

	if (!stream->iostream->isopen)	return -1;
	if (bytes > 4)			return -1;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return -1;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	for (i = bytes - 1; i >= 0; i--)
	{
		if (stream->iostream->currentfilepos >= stream->iostream->size) return -1;

		if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);
		if (stream->iostream->packagesize == 0) return -1;

		rval += stream->iostream->data[stream->iostream->currentpos] * (1 << (i * 8));
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
	}

	return rval;
}

long InStream_InputNumberVAX(InStream *stream, int bytes)	// VAX byte order BADC
{
	long	 rval = 0;
	int	 i;

	if (!stream->iostream->isopen)	return -1;
	if (bytes > 4)			return -1;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return -1;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	for (i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (stream->iostream->currentfilepos >= stream->iostream->size) return -1;

			if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);
			if (stream->iostream->packagesize == 0) return -1;

			rval += (stream->iostream->data[stream->iostream->currentpos] << (((3 - i) ^ 1) * 8)) >> (8 * (4 - bytes));
			stream->iostream->currentpos++;
			if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;
		}
	}

	return rval;
}

long InStream_InputNumberPBD(InStream *stream, int bits)
{
	unsigned char	 inp;
	long		 rval = 0;
	int		 i;
	int		 j;

	if (!stream->iostream->isopen)	return -1;
	if (bits > 32)			return -1;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return -1;

	if (!stream->iostream->pbd) InStream_InitPBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	while (stream->iostream->pbdlen < bits)
	{
		if (stream->iostream->currentfilepos >= stream->iostream->size) return -1;

		if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);
		if (stream->iostream->packagesize == 0) return -1;

		inp = stream->iostream->data[stream->iostream->currentpos];
		stream->iostream->currentpos++;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos++;

		for (i = 0; i < 8; i++)
		{
			stream->iostream->pbdbuffer[stream->iostream->pbdlen] = GetBit(inp, i);
			stream->iostream->pbdlen++;
		}
	}

	for (i = 0; i < bits; i++)
	{
		rval = rval | (stream->iostream->pbdbuffer[i] << i);
	}

	stream->iostream->pbdlen = stream->iostream->pbdlen - bits;

	for (j = 0; j < stream->iostream->pbdlen; j++)
	{
		stream->iostream->pbdbuffer[j] = stream->iostream->pbdbuffer[j + bits];
	}

	return rval;
}

char *InStream_InputString(InStream *stream, int bytes)
{
	char	*rval;
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!stream->iostream->isopen) return NULL;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (bytes == 0) return NULL;

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	rval = malloc(bytes+1);

	while (bytesleft)
	{
		if (stream->iostream->currentfilepos >= stream->iostream->size)
		{
			free(rval);
			return NULL;
		}

		if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);

		if (stream->iostream->packagesize == 0)
		{
			free(rval);
			return NULL;
		}

		amount = ((stream->iostream->packagesize - stream->iostream->currentpos)<(bytesleft))?(stream->iostream->packagesize - stream->iostream->currentpos):(bytesleft);

		memcpy((void *) ((unsigned char *) rval + databufferpos), (void *) (stream->iostream->data + stream->iostream->currentpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		stream->iostream->currentpos += amount;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos += amount;
	}

	rval[bytes] = 0;

	return rval;
}

char *InStream_InputLine(InStream *stream)
{
	long	 inpval;
	int	 bytes = 0;
	char	*rval;
	char	*linebuffer1;
	char	*linebuffer2;
	int	 i;
	int	 j;
	int	 k;

	if (!stream->iostream->isopen) return NULL;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	linebuffer1 = malloc(1024);

	for (j = 0; j >= 0; j++)
	{
		for (i = 0; i < 1024; i++)
		{
			inpval = InStream_InputNumber(stream, 1);

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
				if (inpval == 13) InStream_InputNumber(stream, 1);

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

char *InStream_InputSocketString(InStream *stream)
{
	int	 datasize;
	char	*rval = NULL;
	int	 i;

	if (!stream->iostream->isopen)		return NULL;
	if (!stream->iostream->socketstream)	return NULL;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	if (stream->iostream->currentpos >= stream->iostream->packagesize)
	{
		InStream_ReadData(stream);

		if (stream->iostream->packagesize == 0) return NULL;

		datasize = stream->iostream->packagesize;
	}
	else
	{
		datasize = stream->iostream->packagesize - stream->iostream->currentpos;
	}

	if (datasize == 0) return NULL;

	rval = malloc(datasize + 1);

	for (i = 0; i < datasize; i++)
	{
		rval[i] = (char) InStream_InputNumber(stream, 1);
	}
	rval[datasize] = 0;

	return rval;
}

void *InStream_InputData(InStream *stream, void *pointer, int bytes)
{
	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	if (!stream->iostream->isopen) return NULL;
	if (stream->iostream->streamtype == STREAM_SERVER || stream->iostream->streamtype == STREAM_SOCKS5_BIND || stream->iostream->streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (stream->iostream->pbd && !stream->iostream->holdpbd) InStream_CompletePBD(stream);

	if (bytes == 0) return NULL;

	if (stream->iostream->crosslinked && stream->iostream->socketstream) OutStream_Flush((OutStream *) stream->outstream);

	while (bytesleft)
	{
		if (stream->iostream->currentfilepos >= stream->iostream->size) return NULL;

		if (stream->iostream->currentpos >= stream->iostream->packagesize) InStream_ReadData(stream);
		if (stream->iostream->packagesize == 0) return NULL;

		amount = ((stream->iostream->packagesize - stream->iostream->currentpos)<(bytesleft))?(stream->iostream->packagesize - stream->iostream->currentpos):(bytesleft);

		memcpy((void *) ((unsigned char *) pointer + databufferpos), (void *) (stream->iostream->data + stream->iostream->currentpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		stream->iostream->currentpos += amount;
		if (!stream->iostream->socketstream) stream->iostream->currentfilepos += amount;
	}

	return pointer;
}

void InStream_InitPBD(InStream *stream)
{
	stream->iostream->pbdlen	= 0;
	stream->iostream->pbd		= 1;
}

void InStream_CompletePBD(InStream *stream)
{
	stream->iostream->pbdlen	= 0;
	stream->iostream->pbd		= 0;
}

bool InStream_SetPackageSize(InStream *stream, int newPackagesize)
{
	if (!stream->iostream->isopen)		return false;
	if (!stream->iostream->allowpackset)	return false;
	if (newPackagesize <= 0)		return false;

	if (stream->iostream->pbd) InStream_CompletePBD(stream);

	free(stream->iostream->data);

	stream->iostream->data = malloc(newPackagesize);

	stream->iostream->packagesize = newPackagesize;
	stream->iostream->stdpacksize = stream->iostream->packagesize;

	InStream_Seek(stream, stream->iostream->currentfilepos);

	return true;
}

bool InStream_SetFilter(InStream *stream, IOLibFilter *newFilter)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	stream->iostream->filter = newFilter;

	stream->iostream->allowpackset = true;

	if (stream->iostream->filter->packsize > 0)
	{
		InStream_SetPackageSize(stream, stream->iostream->filter->packsize);

		stream->iostream->allowpackset = false;
	}
	else if (stream->iostream->filter->packsize == -1)
	{
		InStream_SetPackageSize(stream, stream->iostream->size - stream->iostream->currentfilepos);

		stream->iostream->allowpackset = false;
	}

	return true;
}

bool InStream_RemoveFilter(InStream *stream)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	stream->iostream->filter = stream->iostream->NULLFILTER;

	stream->iostream->allowpackset = true;

	InStream_SetPackageSize(stream, stream->iostream->origpacksize);

	return true;
}

bool InStream_Close(InStream *stream)
{
	if (!stream->iostream->isopen && stream->iostream->streamtype == STREAM_NONE) return false;

	if (stream->iostream->crosslinked)
	{
		if (stream->iostream->closefile)
		{
			((OutStream *) stream->outstream)->iostream->closefile = true;
			if (stream->iostream->streamtype == STREAM_FILE) ((OutStream *) stream->outstream)->iostream->streamtype = STREAM_FILE;
		}

		stream->iostream->closefile = false;
	}

	if (stream->iostream->pbd) InStream_CompletePBD(stream);

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

	stream->iostream->isopen	= false;
	stream->iostream->streamtype	= STREAM_NONE;

	return true;
}

long InStream_GetPos(InStream *stream)
{
	if (!stream->iostream->isopen)		return -1;
	if (stream->iostream->socketstream)	return -1;

	return stream->iostream->currentfilepos;
}

bool InStream_Seek(InStream *stream, long position)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	if (stream->iostream->pbd) InStream_CompletePBD(stream);

	if (stream->iostream->buffersize == -1 && !(stream->iostream->streamtype == STREAM_ANSI) && !(stream->iostream->streamtype == STREAM_WINDOWS))	lseek(stream->iostream->stream, position, SEEK_SET);
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_ANSI)							fseek(stream->iostream->hlstream, position, SEEK_SET);
#ifdef __WIN32__
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_WINDOWS)							_llseek(stream->iostream->stream, position, FILE_BEGIN);
#endif

	stream->iostream->currentfilepos	= position;
	stream->iostream->bufferpos		= position;
	stream->iostream->origfilepos		= position;

	InStream_ReadData(stream);

	return true;
}

bool InStream_RelSeek(InStream *stream, long offset)
{
	if (!stream->iostream->isopen)		return false;
	if (stream->iostream->socketstream)	return false;

	if (stream->iostream->pbd) InStream_CompletePBD(stream);

	if (stream->iostream->buffersize == -1 && !(stream->iostream->streamtype == STREAM_ANSI) && !(stream->iostream->streamtype == STREAM_WINDOWS))	lseek(stream->iostream->stream, stream->iostream->currentfilepos + offset, SEEK_SET);
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_ANSI)							fseek(stream->iostream->hlstream, stream->iostream->currentfilepos + offset, SEEK_SET);
#ifdef __WIN32__
	else if (stream->iostream->buffersize == -1 && stream->iostream->streamtype == STREAM_WINDOWS)							_llseek(stream->iostream->stream, stream->iostream->currentfilepos + offset, FILE_BEGIN);
#endif

	stream->iostream->currentfilepos	+= offset;
	stream->iostream->bufferpos		+= offset;
	stream->iostream->origfilepos		= stream->iostream->currentfilepos;

	InStream_ReadData(stream);

	return true;
}

int InStream_GetStreamType(InStream *stream)
{
	return stream->iostream->streamtype;
}

long InStream_Size(InStream *stream)
{
	if (!stream->iostream->isopen)		return -1;
	if (stream->iostream->socketstream)	return -1;

	return stream->iostream->size;
}

bool InStream_SetMode(InStream *stream, long mode)
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

const char *InStream_GetRemoteHostname(InStream *stream)
{
#ifdef MSDOS
	return NULL;
#else
	if (stream->iostream->faraddr.sin_addr.s_addr == 0) return NULL;

	return gethostbyaddr((char *) &stream->iostream->faraddr.sin_addr.s_addr, 4, AF_INET)->h_name;
#endif
}

unsigned long InStream_GetRemoteIP(InStream *stream)
{
#ifdef MSDOS
	return 0;
#else
	if (stream->iostream->faraddr.sin_addr.s_addr == 0) return 0;

	return stream->iostream->faraddr.sin_addr.s_addr;
#endif
}

void InStream_CloseSocket(InStream *stream, SOCKET sock)
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
