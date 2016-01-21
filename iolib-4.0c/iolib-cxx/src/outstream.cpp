 /* IOLib-C++, Universal IO Library
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
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <iolib-cxx.h>

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

OutStream::OutStream(int type, const char *server, int port)
{
	if (type == STREAM_FILE)
	{
		streamtype	= STREAM_FILE;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];

		if (port == OS_APPEND)
		{
			stream = open(server, O_RDWR | O_BINARY | O_RANDOM | O_CREAT, 0600);

			if (stream <= 0)
			{
				delete [] data;
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
			stream = open(server, O_RDWR | O_BINARY | O_RANDOM | O_CREAT | O_TRUNC, 0600);

			if (stream <= 0)
			{
				delete [] data;
				streamtype = STREAM_NONE;
				return;
			}
		}

		isopen		= true;

		return;
	}
	else if (type == STREAM_CLIENT)
	{
#ifdef MSDOS
		return;
#else
		streamtype	= STREAM_CLIENT;
		socketstream	= true;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];
		isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype	= STREAM_NONE;
				isopen		= false;
				return;
			}
		}

		initialized++;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (socket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		hostent		*host = gethostbyname(server);
		sockaddr_in	 saddr;

		if (host == NULL)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		saddr.sin_family	= AF_INET;
		saddr.sin_addr		= *((in_addr *) *host->h_addr_list);
		saddr.sin_port		= htons((short) port);

		if (connect(socket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		connected = true;

		return;
#endif
	}
	else
	{
		return;
	}
}

OutStream::OutStream(int type, int subtype, const char *proxy, unsigned short socksport, const char *server, unsigned short port, ...)
{
	if (type == STREAM_SOCKS4_CLIENT && subtype == STREAM_SOCKS4_NOAUTH)
	{
#ifdef MSDOS
		return;
#else
		unsigned char	*socksdata;

		streamtype	= STREAM_SOCKS4_CLIENT;
		socketstream	= true;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];
		isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype	= STREAM_NONE;
				isopen		= false;
				return;
			}
		}

		initialized++;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (socket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		hostent		*host		= gethostbyname(proxy);
		hostent		*server_hostent	= gethostbyname(server);
		sockaddr_in	 saddr;

		if (host == NULL)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		saddr.sin_family	= AF_INET;
		saddr.sin_addr		= *((in_addr *) *host->h_addr_list);
		saddr.sin_port		= htons((short) socksport);

		if (connect(socket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		if (server_hostent != NULL)
		{
			socksdata = new unsigned char [9];

			socksdata[0] = 4;
			socksdata[1] = 1;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = server_hostent->h_addr_list[0][0];
			socksdata[5] = server_hostent->h_addr_list[0][1];
			socksdata[6] = server_hostent->h_addr_list[0][2];
			socksdata[7] = server_hostent->h_addr_list[0][3];
			socksdata[8] = 0;

			send(socket, (char *) socksdata, 9, 0);

		}
		else if (GetIPAddress(server) != 0)
		{
			socksdata = new unsigned char [9];

			socksdata[0] = 4;
			socksdata[1] = 1;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = GetByte(htonl(GetIPAddress(server)), 0);
			socksdata[5] = GetByte(htonl(GetIPAddress(server)), 1);
			socksdata[6] = GetByte(htonl(GetIPAddress(server)), 2);
			socksdata[7] = GetByte(htonl(GetIPAddress(server)), 3);
			socksdata[8] = 0;

			send(socket, (char *) socksdata, 9, 0);
		}
		else
		{
			socksdata = new unsigned char [10 + strlen(server)];

			socksdata[0] = 4;
			socksdata[1] = 1;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = 0;
			socksdata[5] = 0;
			socksdata[6] = 0;
			socksdata[7] = 1;
			socksdata[8] = 0;

			for (int i = 0; i < (int) strlen(server); i++) socksdata[9 + i] = server[i];

			socksdata[9 + strlen(server)] = 0;

			send(socket, (char *) socksdata, 10 + strlen(server), 0);
		}

		int	 recbytes = 0;

		while (recbytes != 8)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 90)	// proxy rejected request
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		delete [] socksdata;

		connected = true;

		return;
#endif
	}
	else if (type == STREAM_SOCKS4_BIND && subtype == STREAM_SOCKS4_NOAUTH)
	{
#ifdef MSDOS
		return;
#else
		unsigned char	*socksdata;
		char		*remote_host = NULL;
		unsigned short	*remote_port = NULL;
		int		 counter;
		int		 value;
		va_list		 ap;

		va_start(ap, port);

		remote_host = va_arg(ap, char *);
		remote_port = va_arg(ap, unsigned short *);

		va_end(ap);

		streamtype	= STREAM_SOCKS4_BIND;
		socketstream	= true;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];
		isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype	= STREAM_NONE;
				isopen		= false;
				return;
			}
		}

		initialized++;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (socket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		hostent		*host = gethostbyname(proxy);
		hostent		*server_hostent = gethostbyname(server);
		sockaddr_in	 saddr;

		if (host == NULL)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		saddr.sin_family	= AF_INET;
		saddr.sin_addr		= *((in_addr *) *host->h_addr_list);
		saddr.sin_port		= htons((short) socksport);

		if (connect(socket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		if (server_hostent != NULL)
		{
			socksdata = new unsigned char [9];

			socksdata[0] = 4;
			socksdata[1] = 2;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = server_hostent->h_addr_list[0][0];
			socksdata[5] = server_hostent->h_addr_list[0][1];
			socksdata[6] = server_hostent->h_addr_list[0][2];
			socksdata[7] = server_hostent->h_addr_list[0][3];
			socksdata[8] = 0;

			send(socket, (char *) socksdata, 9, 0);

		}
		else if (GetIPAddress(server) != 0)
		{
			socksdata = new unsigned char [9];

			socksdata[0] = 4;
			socksdata[1] = 2;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = GetByte(htonl(GetIPAddress(server)), 0);
			socksdata[5] = GetByte(htonl(GetIPAddress(server)), 1);
			socksdata[6] = GetByte(htonl(GetIPAddress(server)), 2);
			socksdata[7] = GetByte(htonl(GetIPAddress(server)), 3);
			socksdata[8] = 0;

			send(socket, (char *) socksdata, 9, 0);
		}
		else
		{
			socksdata = new unsigned char [10 + strlen(server)];

			socksdata[0] = 4;
			socksdata[1] = 2;
			socksdata[2] = htons((short) port) % 256;
			socksdata[3] = htons((short) port) / 256;
			socksdata[4] = 0;
			socksdata[5] = 0;
			socksdata[6] = 0;
			socksdata[7] = 1;
			socksdata[8] = 0;

			for (int i = 0; i < (int) strlen(server); i++) socksdata[9 + i] = server[i];

			socksdata[9 + strlen(server)] = 0;

			send(socket, (char *) socksdata, 10 + strlen(server), 0);
		}

		int	 recbytes = 0;

		while (recbytes != 8)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 90)	// proxy rejected request
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
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

			for (int i = 0; i < 15; i++)
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

		delete [] socksdata;

		return;
#endif
	}
	else if (type == STREAM_SOCKS5_CLIENT)
	{
#ifdef MSDOS
		return;
#else
		unsigned char	*socksdata;
		char		*uname = NULL;
		char		*passwd = NULL;
		va_list		 ap;

		if (subtype == STREAM_SOCKS5_AUTH)
		{
			va_start(ap, port);

			uname	= va_arg(ap, char *);
			passwd	= va_arg(ap, char *);

			va_end(ap);
		}

		streamtype	= STREAM_SOCKS5_CLIENT;
		socketstream	= true;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];
		isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype	= STREAM_NONE;
				isopen		= false;
				return;
			}
		}

		initialized++;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (socket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		hostent		*host = gethostbyname(proxy);
		sockaddr_in	 saddr;

		if (host == NULL)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		saddr.sin_family	= AF_INET;
		saddr.sin_addr		= *((in_addr *) *host->h_addr_list);
		saddr.sin_port		= htons((short) socksport);

		if (connect(socket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		int	 recbytes = 0;
		int	 neededbytes;

		if (subtype == STREAM_SOCKS5_NOAUTH)
		{
			socksdata = new unsigned char [3];

			socksdata[0] = 5;
			socksdata[1] = 1;
			socksdata[2] = 0;

			send(socket, (char *) socksdata, 3, 0);

			while (recbytes != 2)
			{
				recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
			}

			if (socksdata[1] == 255)	// proxy requires authentication
			{
				delete [] data;
				delete [] socksdata;
				streamtype = STREAM_NONE;
				CloseSocket(socket);
				isopen = false;
				return;
			}

			delete [] socksdata;
		}
		else
		{
			socksdata = new unsigned char [4];

			socksdata[0] = 5;
			socksdata[1] = 2;
			socksdata[2] = 0;
			socksdata[3] = 2;

			send(socket, (char *) socksdata, 4, 0);

			while (recbytes != 2)
			{
				recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
			}

			if (socksdata[1] == 255)	// proxy needs authentication, but doesn't support username/password
			{
				delete [] data;
				delete [] socksdata;
				streamtype = STREAM_NONE;
				CloseSocket(socket);
				isopen = false;
				return;
			}

			if (socksdata[1] == 2)
			{
				delete [] socksdata;

				socksdata = new unsigned char [3 + strlen(uname) + strlen(passwd)];

				socksdata[0] = 1;
				socksdata[1] = strlen(uname);

				for (int i = 0; i < (int) strlen(uname); i++) socksdata[2 + i] = uname[i];

				socksdata[2 + strlen(uname)] = strlen(passwd);

				for (int j = 0; j < (int) strlen(passwd); j++) socksdata[3 + strlen(uname) + j] = passwd[j];

				send(socket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

				recbytes = 0;

				while (recbytes != 2)
				{
					recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] != 0)	// proxy rejected username/password
				{
					delete [] data;
					delete [] socksdata;
					streamtype = STREAM_NONE;
					CloseSocket(socket);
					isopen = false;
					return;
				}
			}

			delete [] socksdata;
		}

		socksdata = new unsigned char [7 + strlen(server)];

		socksdata[0] = 5;
		socksdata[1] = 1;
		socksdata[2] = 0;
		socksdata[3] = 3;
		socksdata[4] = strlen(server);

		for (int i = 0; i < (int) strlen(server); i++) socksdata[5 + i] = server[i];

		socksdata[5 + strlen(server)] = htons((short) port) % 256;
		socksdata[6 + strlen(server)] = htons((short) port) / 256;

		send(socket, (char *) socksdata, 7 + strlen(server), 0);

		recbytes = 0;

		while (recbytes != 5)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		if (socksdata[3] == 1)
		{
			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}
		else if (socksdata[3] == 3)
		{
			recbytes	= 0;
			neededbytes	= socksdata[4] + 2;

			while (recbytes != neededbytes)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
			}
		}

		delete [] socksdata;

		connected = true;

		return;
#endif
	}
	else if (type == STREAM_SOCKS5_BIND)
	{
#ifdef MSDOS
		return;
#else
		unsigned char	*socksdata;
		char		*uname = NULL;
		char		*passwd = NULL;
		char		*remote_host = NULL;
		unsigned short	*remote_port = NULL;
		int		 counter;
		int		 value;
		va_list		 ap;

		va_start(ap, port);

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
		data		= new unsigned char [packagesize];
		isopen		= true;

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype	= STREAM_NONE;
				isopen		= false;
				return;
			}
		}

		initialized++;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (socket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		hostent		*host = gethostbyname(proxy);
		sockaddr_in	 saddr;

		if (host == NULL)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		saddr.sin_family	= AF_INET;
		saddr.sin_addr		= *((in_addr *) *host->h_addr_list);
		saddr.sin_port		= htons((short) socksport);

		if (connect(socket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		int	 recbytes = 0;
		int	 neededbytes;

		if (subtype == STREAM_SOCKS5_NOAUTH)
		{
			socksdata = new unsigned char [3];

			socksdata[0] = 5;
			socksdata[1] = 1;
			socksdata[2] = 0;

			send(socket, (char *) socksdata, 3, 0);

			while (recbytes != 2)
			{
				recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
			}

			if (socksdata[1] == 255)	// proxy requires authentication
			{
				delete [] data;
				delete [] socksdata;
				streamtype = STREAM_NONE;
				CloseSocket(socket);
				isopen = false;
				return;
			}

			delete [] socksdata;
		}
		else
		{
			socksdata = new unsigned char [4];

			socksdata[0] = 5;
			socksdata[1] = 2;
			socksdata[2] = 0;
			socksdata[3] = 2;

			send(socket, (char *) socksdata, 4, 0);

			while (recbytes != 2)
			{
				recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
			}

			if (socksdata[1] == 255)	// proxy needs authentication, but doesn't support username/password
			{
				delete [] data;
				delete [] socksdata;
				streamtype = STREAM_NONE;
				CloseSocket(socket);
				isopen = false;
				return;
			}

			if (socksdata[1] == 2)
			{
				delete [] socksdata;

				socksdata = new unsigned char [3 + strlen(uname) + strlen(passwd)];

				socksdata[0] = 1;
				socksdata[1] = strlen(uname);

				for (int i = 0; i < (int) strlen(uname); i++) socksdata[2 + i] = uname[i];

				socksdata[2 + strlen(uname)] = strlen(passwd);

				for (int j = 0; j < (int) strlen(passwd); j++) socksdata[3 + strlen(uname) + j] = passwd[j];

				send(socket, (char *) socksdata, 3 + strlen(uname) + strlen(passwd), 0);

				recbytes = 0;

				while (recbytes != 2)
				{
					recbytes += recv(socket, (char *) socksdata + recbytes, 2 - recbytes, 0);
				}

				if (socksdata[1] != 0)	// proxy rejected username/password
				{
					delete [] data;
					delete [] socksdata;
					streamtype = STREAM_NONE;
					CloseSocket(socket);
					isopen = false;
					return;
				}
			}

			delete [] socksdata;
		}

		socksdata = new unsigned char [7 + strlen(server)];

		socksdata[0] = 5;
		socksdata[1] = 2;
		socksdata[2] = 0;
		socksdata[3] = 3;
		socksdata[4] = strlen(server);

		for (int j = 0; j < (int) strlen(server); j++) socksdata[5 + j] = server[j];

		socksdata[5 + strlen(server)] = htons((short) port) % 256;
		socksdata[6 + strlen(server)] = htons((short) port) / 256;

		send(socket, (char *) socksdata, 7 + strlen(server), 0);

		delete [] socksdata;

		socksdata = new unsigned char [300];

		recbytes = 0;

		while (recbytes != 5)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return;
		}

		if (socksdata[3] == 1)
		{
			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}
		else if (socksdata[3] == 3)
		{
			recbytes	= 0;
			neededbytes	= socksdata[4] + 2;

			while (recbytes != neededbytes)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
			}
		}

		int	 i = 0;
		
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

		delete [] socksdata;

		return;
#endif
	}
	else
	{
		return;
	}
}

OutStream::OutStream(int type, int port)
{
	if (type == STREAM_POSIX)
	{
		streamtype	= STREAM_POSIX;
		stream		= port;
		closefile	= false;
		currentfilepos	= tell(stream);
		lastfilepos	= 0;

		lseek(stream, 0, SEEK_END);

		size		= tell(stream);

		lseek(stream, currentfilepos, SEEK_SET);

		data		= new unsigned char [packagesize];
		isopen		= true;

		return;
	}
	else if (type == STREAM_WINDOWS)
	{
#if defined __WIN32__
		streamtype	= STREAM_WINDOWS;
		stream		= port;
		closefile	= false;
		currentfilepos	= _llseek(stream, 0, FILE_CURRENT);
		lastfilepos	= 0;

		_llseek(stream, 0, FILE_END);

		size		= _llseek(stream, 0, FILE_END);

		_llseek(stream, currentfilepos, FILE_BEGIN);

		data		= new unsigned char [packagesize];
		isopen		= true;

		return;
#else
		return;
#endif
	}
	else if (type == STREAM_SOCKET)
	{
#ifdef MSDOS
		return;
#else
		streamtype	= STREAM_SOCKET;
		socketstream	= true;
		socket		= (SOCKET) port;
		currentfilepos	= -1;
		lastfilepos	= 0;
		closefile	= false;
		connected	= true;
		data		= new unsigned char [packagesize];
		isopen		= true;

		return;
#endif
	}
	else if (type == STREAM_SERVER)
	{
#ifdef MSDOS
		return;
#else
		streamtype	= STREAM_SERVER;
		socketstream	= true;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];

#if defined __WIN32__ && !defined __CYGWIN32__
		WORD	 wVersionRequested = MAKEWORD(1,1);
		WSADATA	 wsaData;

		if (initialized == 0)
		{
			WSAStartup(wVersionRequested, &wsaData);

			if (wsaData.wVersion != wVersionRequested) // Wrong WinSock Version
			{
				delete [] data;
				streamtype = STREAM_NONE;
				return;
			}
		}

		initialized++;
#endif

		localSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (localSocket == (SOCKET)(~0))
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(localSocket);
			return;
		}

		sockaddr_in	 saddr;

		saddr.sin_family	= AF_INET;
		saddr.sin_addr.s_addr	= INADDR_ANY;
		saddr.sin_port		= htons((short) port);

		if (bind(localSocket, (sockaddr *) &saddr, sizeof(struct sockaddr)) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(localSocket);
			return;
		}

		if (listen(localSocket, SOMAXCONN) == -1)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			CloseSocket(localSocket);
			return;
		}

		return;
#endif
	}
	else
	{
		return;
	}
}

OutStream::OutStream(int type, FILE *openfile)
{
	if (type == STREAM_ANSI)
	{
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
		data		= new unsigned char [packagesize];
		isopen		= true;

		return;
	}
	else
	{
		return;
	}
}

OutStream::OutStream(int type, void *outbuffer, long bufsize)
{
	if (type == STREAM_BUFFER)
	{
		streamtype	= STREAM_BUFFER;
		lastfilepos	= 0;
		packagesize	= 1;
		stdpacksize	= packagesize;
		origpacksize	= packagesize;
		data		= new unsigned char [packagesize];
		closefile	= false;
		isopen		= true;
		buffersize	= bufsize;
		buffer		= outbuffer;

		return;
	}
	else
	{
		return;
	}
}

OutStream::OutStream(int type, InStream *in)
{
	if (type != STREAM_STREAM || in->streamtype == STREAM_SERVER || in->streamtype == STREAM_SOCKS5_BIND || in->streamtype == STREAM_SOCKS4_BIND)			return;
	if ((!in->isopen && !(in->streamtype == STREAM_SERVER || in->streamtype == STREAM_SOCKS5_BIND || in->streamtype == STREAM_SOCKS4_BIND)) || in->crosslinked)	return;

	streamtype = STREAM_STREAM;

	crosslinked = true;
	instream = in;
	instream->outstream = this;
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
		data		= new unsigned char [packagesize];
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
		data		= new unsigned char [packagesize];
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
		isopen		= in->isopen;
		connected	= instream->connected;
		socket		= instream->socket;
		localSocket	= instream->localSocket;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];

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
		stdpacksize	= packagesize;
		origpacksize	= packagesize;
		data		= new unsigned char [packagesize];
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
		data		= new unsigned char [packagesize];
		size		= instream->origsize;

		return;
	}
}

OutStream::OutStream(int type, OutStream *out)
{
	if (type == STREAM_CHILD)
	{
		initialized++;

		streamtype	= STREAM_CHILD;
		socketstream	= true;
		isopen		= true;
		connected	= true;
		socket		= out->socket;
		currentfilepos	= -1;
		lastfilepos	= 0;
		data		= new unsigned char [packagesize];

		return;
	}
	else
	{
		return;
	}
}

OutStream::~OutStream()
{
	if (crosslinked)
	{
		instream->crosslinked	= false;
		instream->outstream	= NULL;
	}

	Close();	// close stream
}

OutStream *OutStream::WaitForClient()
{
#ifdef MSDOS
	return NULL;
#else
	if (streamtype != STREAM_SERVER && streamtype != STREAM_SOCKS5_BIND && streamtype != STREAM_SOCKS4_BIND) return NULL;

	OutStream	*out;
	unsigned char	*socksdata;
	int		 recbytes = 0;
	int		 neededbytes;

#if defined __WIN32__ || defined __CYGWIN32__ || defined __BEOS__ || defined sgi
	int		 faraddrsize = sizeof(struct sockaddr_in);
#else
	unsigned int	 faraddrsize = sizeof(struct sockaddr_in);
#endif

	if (streamtype == STREAM_SERVER)
	{
		socket = accept(localSocket, (struct sockaddr *) &faraddr, &faraddrsize);

		if (socket == (SOCKET)(~0))
		{
			return NULL;
		}
		else
		{
			out = new OutStream(STREAM_CHILD, this);

			out->faraddr = faraddr;

			return out;
		}
	}
	else if (streamtype == STREAM_SOCKS4_BIND)
	{
		socksdata = new unsigned char [8];

		while (recbytes != 8)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 8 - recbytes, 0);
		}

		if (socksdata[1] != 90)	// an error occurred
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return NULL;
		}

		delete [] socksdata;

		out = new OutStream(STREAM_CHILD, this);

		return out;
	}
	else if (streamtype == STREAM_SOCKS5_BIND)
	{
		socksdata = new unsigned char [300];

		while (recbytes != 5)
		{
			recbytes += recv(socket, (char *) socksdata + recbytes, 5 - recbytes, 0);
		}

		if (socksdata[1] != 0)	// an error occurred
		{
			delete [] data;
			delete [] socksdata;
			streamtype = STREAM_NONE;
			CloseSocket(socket);
			isopen = false;
			return NULL;
		}

		if (socksdata[3] == 1)
		{
			recbytes = 0;

			while (recbytes != 5)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, 5 - recbytes, 0);
			}
		}
		else if (socksdata[3] == 3)
		{
			recbytes	= 0;
			neededbytes	= socksdata[4] + 2;

			while (recbytes != neededbytes)
			{
				recbytes += recv(socket, (char *) socksdata + 5 + recbytes, neededbytes - recbytes, 0);
			}
		}

		delete [] socksdata;

		out = new OutStream(STREAM_CHILD, this);

		return out;
	}

	return NULL;
#endif
}

void OutStream::Flush()
{
	if (!isopen)		return;
	if (currentpos <= 0)	return;

	if (pbd) CompletePBD();

	int	 ps		= packagesize;
	int	 oldcpos	= currentpos;

	if (filter != NULLFILTER && filter->packsize > 0)
	{
		for (int i = 0; i < (packagesize - oldcpos); i++)
		{
			OutputNumber(0, 1);
		}
	}
	else
	{
		packagesize = currentpos;

		WriteData();

		packagesize = ps;
	}
}

void OutStream::WriteData()
{
	if (!isopen)			return;
	if (currentpos < packagesize)	return;

	unsigned char	*databuffer;
	int		 encsize = 0;

	if (filter->packsize == -1)
	{
		databuffer = new unsigned char [packagesize];

		memcpy((void *) databuffer, (void *) data, packagesize);

		delete [] data;

		data = new unsigned char [packagesize + DEFAULT_PACKAGE_SIZE];

		memcpy((void *) data, (void *) databuffer, packagesize);

		delete [] databuffer;

		packagesize += DEFAULT_PACKAGE_SIZE;
		stdpacksize = packagesize;

		return;
	}

	if (streamtype == STREAM_ANSI)
	{
		fseek(hlstream, lastfilepos, SEEK_SET);

		filter->EncodeData(&data, packagesize, &encsize);

		fwrite((void *) data, encsize, 1, hlstream);
		fflush(hlstream);

		delete [] data;

		data = new unsigned char [packagesize];

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
#if defined __WIN32__
	else if (streamtype == STREAM_WINDOWS)
	{
		_llseek(stream, lastfilepos, FILE_BEGIN);

		filter->EncodeData(&data, packagesize, &encsize);

		_hwrite(stream, (char *) data, encsize);

		delete [] data;

		data = new unsigned char [packagesize];

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
#endif
#ifndef MSDOS
	else if (socketstream)
	{
		if (send(socket, (char *) data, packagesize, 0) == -1)
		{							// looks like other end has been closed
			delete [] data;
			connected	= false;
			isopen		= false;
			streamtype	= STREAM_NONE;

			CloseSocket(socket);
		}

		currentpos -= packagesize;
	}
#endif
	else if (buffersize == -1)
	{
		lseek(stream, lastfilepos, SEEK_SET);

		filter->EncodeData(&data, packagesize, &encsize);

		write(stream, data, encsize);

		delete [] data;

		data = new unsigned char [packagesize];

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
		lastfilepos += encsize;
	}
	else
	{
		filter->EncodeData(&data, packagesize, &encsize);

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

		delete [] data;

		data = new unsigned char [packagesize];

		currentpos -= packagesize;
		if (size == currentfilepos) size -= (packagesize - encsize);
		currentfilepos -= (packagesize - encsize);
	}
}

bool OutStream::OutputNumber(long number, int bytes)
{
	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	for (int i = 0; i < bytes; i++)
	{
		if (currentpos >= packagesize) WriteData();

		data[currentpos] = GetByte(number, i);
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	if (currentpos >= packagesize) WriteData();

	if (socketstream && !holdpbd) Flush();

	return true;
}

bool OutStream::OutputNumberRaw(long number, int bytes)
{
	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	for (int i = bytes - 1; i >= 0; i--)
	{
		if (currentpos >= packagesize) WriteData();

		data[currentpos] = GetByte(number, i);
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	if (currentpos >= packagesize) WriteData();

	if (socketstream && !holdpbd) Flush();

	return true;
}

bool OutStream::OutputNumberVAX(long number, int bytes)
{
	if (!isopen)	return false;
	if (bytes > 4)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	number <<= 8 * (4 - bytes);

	for (int i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (currentpos >= packagesize) WriteData();

			data[currentpos] = GetByte(number, (3 - i) ^ 1);
			if (currentfilepos == size) size++;
			currentpos++;
			if (!socketstream) currentfilepos++;
		}
	}

	if (currentpos >= packagesize) WriteData();

	if (socketstream && !holdpbd) Flush();

	return true;
}

bool OutStream::OutputNumberPBD(long number, int bits)
{
	if (!isopen)	return false;
	if (bits > 32)	return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (!pbd) InitPBD();

	unsigned char	 out;

	for (int j = 0; j < bits; j++)
	{
		pbdbuffer[pbdlen] = GetBit(number, j);
		pbdlen++;
	}

	while (pbdlen >= 8)
	{
		out = 0;

		for (int i = 0; i < 8; i++)
		{
			out = out | (pbdbuffer[i] << i);
		}

		pbdlen = pbdlen - 8;

		for (int j = 0; j < pbdlen; j++)
		{
			pbdbuffer[j] = pbdbuffer[j+8];
		}

		data[currentpos] = out;
		if (currentfilepos == size) size++;
		currentpos++;
		if (!socketstream) currentfilepos++;
		if (currentpos >= packagesize) WriteData();
	}

	return true;
}

bool OutStream::OutputString(const char *string)
{
	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	if (string == NULL) return false;

	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		WriteData();
	}

	if (socketstream) Flush();

	return true;
}

bool OutStream::OutputLine(const char *string)
{
	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	if (string == NULL) return false;

	int	 bytesleft = strlen(string);
	int	 databufferpos = 0;
	int	 amount = 0;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) string + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		WriteData();
	}

#if (defined __WIN32__ || defined MSDOS) && !defined __CYGWIN32__
	OutputNumber(13, 1);
#endif

	OutputNumber(10, 1);

	if (socketstream) Flush();

	return true;
}

bool OutStream::OutputData(const void *pointer, int bytes)
{
	if (!isopen) return false;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return false;

	if (pbd && !holdpbd) CompletePBD();

	if (pointer == NULL) return false;

	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	while (bytesleft)
	{
		amount = ((packagesize - currentpos)<(bytesleft))?(packagesize - currentpos):(bytesleft);

		memcpy((void *) (data + currentpos), (void *) ((unsigned char *) pointer + databufferpos), amount);

		bytesleft -= amount;
		databufferpos += amount;
		currentpos += amount;
		if (!socketstream) currentfilepos += amount;

		if (size < currentfilepos) size = currentfilepos;

		WriteData();
	}

	if (socketstream) Flush();

	return true;
}

void OutStream::InitPBD()
{
	pbdlen	= 0;
	pbd	= 1;
}

void OutStream::CompletePBD()
{
	if (pbdlen > 0)
	{
		int	out = 0;

		for (int i = 0; i < 8; i++)
		{
			if (i < pbdlen) out = out | (pbdbuffer[i] << i);
		}

		holdpbd = true;
		OutputNumber(out, 1);
		holdpbd = false;
	}

	if (socketstream) Flush();

	pbd = 0;
}

bool OutStream::SetPackageSize(int newPackagesize)
{
	if (!isopen)			return false;
	if (!allowpackset)		return false;
	if (newPackagesize <= 0)	return false;

	Flush();

	lastfilepos = currentfilepos;

	delete [] data;

	data = new unsigned char [newPackagesize];

	packagesize = newPackagesize;
	stdpacksize = packagesize;

	return true;
}

bool OutStream::SetFilter(IOLibFilter *newFilter)
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	if (pbd && !holdpbd) CompletePBD();

	allowpackset = true;

	if (newFilter->packsize > 0)
	{
		SetPackageSize(newFilter->packsize);	// package size must be eqv filter size

		allowpackset = false;
	}
	else if (newFilter->packsize == -1)
	{
		SetPackageSize(DEFAULT_PACKAGE_SIZE);

		allowpackset = false;
	}

	filter = newFilter;

	return true;
}

bool OutStream::RemoveFilter()
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	if (pbd && !holdpbd) CompletePBD();

	int	 oldcpos = currentpos;

	if (filter->packsize > 0 && currentpos != 0)
	{
		for (int i = 0; i < (packagesize - oldcpos); i++)
		{
			OutputNumber(0, 1);
		}
	}
	else if (filter->packsize == -1)
	{
		filter->packsize = 0;

		allowpackset = true;

		Flush();

		filter->packsize = -1;
	}

	allowpackset = true;

	filter = NULLFILTER;

	SetPackageSize(origpacksize);

	return true;
}

bool OutStream::Close()
{
	if (!isopen && streamtype == STREAM_NONE) return false;

	if (filter != NULLFILTER) RemoveFilter();

	Flush();

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
				if (connected) closesocket(socket);
#else
				close(localSocket);
				if (connected) close(socket);
#endif
			}
			else
			{
#ifdef __WIN32__
				closesocket(socket);
#else
				close(socket);
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

	delete [] data;
	data = NULL;

	isopen		= false;
	streamtype	= STREAM_NONE;

	return true;
}

bool OutStream::Seek(long position)
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	Flush();

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

bool OutStream::RelSeek(long offset)
{
	if (!isopen)		return false;
	if (socketstream)	return false;

	Flush();

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

#endif
