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

#ifndef __IOLIB_INSTREAM_
#define __IOLIB_INSTREAM_

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <iolib-cxx.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

#ifndef SOMAXCONN
	#define SOMAXCONN 5
#endif

InStream::InStream(int type, const char *filename)
{
	if (type == STREAM_FILE)
	{
		streamtype	= STREAM_FILE;
		currentpos	= DEFAULT_PACKAGE_SIZE;
		data		= new unsigned char [packagesize];
		stream		= open(filename, O_RDWR | O_BINARY);

		if (stream <= 0)
		{
			delete [] data;
			streamtype = STREAM_NONE;
			return;
		}

		lseek(stream, 0, SEEK_END);

		size		= tell(stream);
		origsize	= size;

		lseek(stream, 0, SEEK_SET);

		isopen		= true;

		return;
	}
	else
	{
		return;
	}
}

InStream::InStream(int type, const char *server, int port)
{
	if (type == STREAM_CLIENT)
	{
#ifdef MSDOS
		return;
#else
		streamtype	= STREAM_CLIENT;
		socketstream	= true;
		currentfilepos	= -1;
		allowpackset	= false;
		currentpos	= DEFAULT_PACKAGE_SIZE;
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
		saddr.sin_addr	= *((in_addr *) *host->h_addr_list);
		saddr.sin_port	= htons((short) port);

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

InStream::InStream(int type, int subtype, const char *proxy, unsigned short socksport, const char *server, unsigned short port, ...)
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
		allowpackset	= false;
		currentpos	= DEFAULT_PACKAGE_SIZE;
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
		saddr.sin_addr	= *((in_addr *) *host->h_addr_list);
		saddr.sin_port	= htons((short) socksport);

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
		allowpackset	= false;
		currentpos	= DEFAULT_PACKAGE_SIZE;
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
		saddr.sin_addr	= *((in_addr *) *host->h_addr_list);
		saddr.sin_port	= htons((short) socksport);

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

		int recbytes = 0;

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
		allowpackset	= false;
		currentpos	= DEFAULT_PACKAGE_SIZE;
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
		currentpos	= DEFAULT_PACKAGE_SIZE;
		allowpackset	= false;
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

		int	  i = 0;

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

InStream::InStream(int type, int port)
{
	if (type == STREAM_POSIX)
	{
		streamtype	= STREAM_POSIX;
		stream		= port;
		closefile	= false;
		currentfilepos	= tell(stream);
		currentpos	= DEFAULT_PACKAGE_SIZE;

		lseek(stream, 0, SEEK_END);

		size		= tell(stream);
		origsize	= size;

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
		currentpos	= DEFAULT_PACKAGE_SIZE;

		_llseek(stream, 0, FILE_END);

		size		= _llseek(stream, 0, FILE_CURRENT);
		origsize	= size;

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
		closefile	= false;
		socket		= (SOCKET) port;
		socketstream	= true;
		currentfilepos	= -1;
		allowpackset	= false;
		currentpos	= DEFAULT_PACKAGE_SIZE;
		data		= new unsigned char [packagesize];
		isopen		= true;
		connected	= true;

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
		allowpackset	= false;
		currentfilepos	= -1;
		currentpos	= DEFAULT_PACKAGE_SIZE;
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

InStream::InStream(int type, FILE *openfile)
{
	if (type == STREAM_ANSI)
	{
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
		data		= new unsigned char [packagesize];
		isopen		= true;

		return;
	}
	else
	{
		return;
	}
}

InStream::InStream(int type, void *inbuffer, long bufsize)
{
	if (type == STREAM_BUFFER)
	{
		streamtype	= STREAM_BUFFER;
		buffer		= inbuffer;
		buffersize	= bufsize;
		packagesize	= 1;
		stdpacksize	= packagesize;
		origpacksize	= packagesize;
		currentpos	= packagesize;
		size		= buffersize;
		origsize	= size;
		data		= new unsigned char [packagesize];
		closefile	= false;
		isopen		= true;

		return;
	}
	else
	{
		return;
	}
}

InStream::InStream(int type, OutStream *out)
{
	if (type != STREAM_STREAM || out->streamtype == STREAM_SERVER || out->streamtype == STREAM_SOCKS5_BIND || out->streamtype == STREAM_SOCKS4_BIND)			return;
	if ((!out->isopen && !(out->streamtype == STREAM_SERVER || out->streamtype == STREAM_SOCKS5_BIND || out->streamtype == STREAM_SOCKS4_BIND)) || out->crosslinked)	return;

	streamtype = STREAM_STREAM;

	crosslinked = true;
	outstream = out;
	outstream->instream = this;
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
		data		= new unsigned char [packagesize];
		currentpos	= packagesize;
		size		= outstream->size;
		origsize	= size;

		return;
	}
#ifdef __WIN32__
	else if (outstream->streamtype == STREAM_WINDOWS)
	{
		streamtype	= STREAM_WINDOWS;
		closefile	= false;
		isopen		= true;
		stream		= outstream->stream;
		currentfilepos	= outstream->currentfilepos;
		data		= new unsigned char [packagesize];
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
		socket		= outstream->socket;
		localSocket	= outstream->localSocket;
		allowpackset	= false;
		currentfilepos	= -1;
		data		= new unsigned char [packagesize];
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
		data		= new unsigned char [packagesize];
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
		data		= new unsigned char [packagesize];
		currentpos	= packagesize;
		size		= outstream->size;
		origsize	= size;

		return;
	}
}

InStream::InStream(int type, InStream *in)
{
	if (type == STREAM_CHILD)
	{
		initialized++;

		streamtype	= STREAM_CHILD;
		socketstream	= true;
		isopen		= true;
		connected	= true;
		socket		= in->socket;
		allowpackset	= false;
		currentfilepos	= -1;
		data		= new unsigned char [packagesize];
		currentpos	= packagesize;

		return;
	}
	else
	{
		return;
	}
}

InStream::~InStream()
{
	if (crosslinked)
	{
		outstream->crosslinked	= false;
		outstream->instream	= NULL;
	}

	Close();	// close stream
}

InStream *InStream::WaitForClient()
{
#ifdef MSDOS
	return NULL;
#else
	if (streamtype != STREAM_SERVER && streamtype != STREAM_SOCKS5_BIND && streamtype != STREAM_SOCKS4_BIND) return NULL;

	InStream	*in;
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
			in = new InStream(STREAM_CHILD, this);

			in->faraddr = faraddr;

			return in;
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

		in = new InStream(STREAM_CHILD, this);

		return in;
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

		in = new InStream(STREAM_CHILD, this);

		return in;
	}

	return NULL;
#endif
}

void InStream::ReadData()
{
	if (!isopen) return;

	int decsize = 0;

	if (streamtype == STREAM_ANSI)
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize > 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		delete [] data;

		data = new unsigned char [packagesize];

		fseek(hlstream, currentfilepos, SEEK_SET); // Do this because a crosslinked OutStream might have changed the file pointer

		fread((void *) data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), 1, hlstream);

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			filter->DecodeData(&data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), &decsize);

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
			if (filter->packsize > 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		delete [] data;

		data = new unsigned char [packagesize];

		_llseek(stream, currentfilepos, FILE_BEGIN); // Do this because a crosslinked OutStream might have changed the file pointer

		_hread(stream, (void *) data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)));

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			filter->DecodeData(&data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), &decsize);

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
		packagesize = recv(socket, (char *) data, packagesize, 0);

		if (packagesize == -1)
		{
			delete [] data;
			connected	= false;
			isopen		= false;
			streamtype	= STREAM_NONE;

			CloseSocket(socket);
		}

		currentpos = 0;
	}
#endif
	else if (buffersize == -1)
	{
		if (filter != NULLFILTER)
		{
			if (filter->packsize > 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		delete [] data;

		data = new unsigned char [packagesize];

		lseek(stream, currentfilepos, SEEK_SET);

		read(stream, data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)));

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			filter->DecodeData(&data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), &decsize);

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
			if (filter->packsize > 0)	packagesize = filter->packsize;
			else				packagesize = stdpacksize;
		}
		else	packagesize = stdpacksize;

		size		= origsize;
		currentfilepos	= origfilepos;

		delete [] data;

		data = new unsigned char [packagesize];

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

		if (packagesize <= size-currentfilepos || (filter != NULLFILTER && filter->packsize == 0))
		{
			origfilepos = currentfilepos + packagesize;

			filter->DecodeData(&data, ((packagesize)<(size-currentfilepos)?(packagesize):(size-currentfilepos)), &decsize);

			packagesize	= decsize;
			origsize	= size;

			if (packagesize + currentfilepos > size) size = packagesize + currentfilepos;
		}

		currentpos = 0;
	}
}

long InStream::InputNumber(int bytes)	// Intel byte order DCBA
{
	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) CompletePBD();

	if (crosslinked && socketstream) outstream->Flush();

	long	 rval = 0;

	for (int i = 0; i < bytes; i++)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) ReadData();
		if (packagesize == 0) return -1;

		rval += data[currentpos] * (1 << (i * 8));
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	return rval;
}

long InStream::InputNumberRaw(int bytes)	// Raw byte order ABCD
{
	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) CompletePBD();

	if (crosslinked && socketstream) outstream->Flush();

	long	 rval = 0;

	for (int i = bytes - 1; i >= 0; i--)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) ReadData();
		if (packagesize == 0) return -1;

		rval += data[currentpos] * (1 << (i * 8));
		currentpos++;
		if (!socketstream) currentfilepos++;
	}

	return rval;
}

long InStream::InputNumberVAX(int bytes)	// VAX byte order BADC
{
	if (!isopen)	return -1;
	if (bytes > 4)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (pbd && !holdpbd) CompletePBD();

	if (crosslinked && socketstream) outstream->Flush();

	long	 rval = 0;

	for (int i = 0; i < 4; i++)
	{
		if (bytes >= (i ^ 1) + 1)
		{
			if (currentfilepos >= size) return -1;

			if (currentpos >= packagesize) ReadData();
			if (packagesize == 0) return -1;

			rval += (data[currentpos] << (((3 - i) ^ 1) * 8)) >> (8 * (4 - bytes));
			currentpos++;
			if (!socketstream) currentfilepos++;
		}
	}

	return rval;
}

long InStream::InputNumberPBD(int bits)
{
	if (!isopen)	return -1;
	if (bits > 32)	return -1;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return -1;

	if (!pbd) InitPBD();

	if (crosslinked && socketstream) outstream->Flush();

	unsigned char	 inp;
	long		 rval = 0;

	while (pbdlen < bits)
	{
		if (currentfilepos >= size) return -1;

		if (currentpos >= packagesize) ReadData();
		if (packagesize == 0) return -1;

		inp = data[currentpos];
		currentpos++;
		if (!socketstream) currentfilepos++;

		for (int i = 0; i < 8; i++)
		{
			pbdbuffer[pbdlen] = GetBit(inp, i);
			pbdlen++;
		}
	}

	for (int i = 0; i < bits; i++)
	{
		rval = rval | (pbdbuffer[i] << i);
	}

	pbdlen = pbdlen - bits;

	for (int j = 0; j < pbdlen; j++)
	{
		pbdbuffer[j] = pbdbuffer[j + bits];
	}

	return rval;
}

char *InStream::InputString(int bytes)
{
	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) CompletePBD();

	if (bytes == 0) return NULL;

	if (crosslinked && socketstream) outstream->Flush();

	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;
	char	*rval = new char [bytes+1];

	while (bytesleft)
	{
		if (currentfilepos >= size)
		{
			delete [] rval;
			return NULL;
		}

		if (currentpos >= packagesize) ReadData();

		if (packagesize == 0)
		{
			delete [] rval;
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

char *InStream::InputLine()
{
	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) CompletePBD();

	if (crosslinked && socketstream) outstream->Flush();

	long	 inpval;
	int	 bytes = 0;
	char	*rval;
	char	*linebuffer1 = new char [1024];
	char	*linebuffer2;

	for (int j = 0; j >= 0; j++)
	{
		for (int i = 0; i < 1024; i++)
		{
			inpval = InputNumber(1);

			if (inpval == -1)
			{
				linebuffer1[bytes] = 0;

				rval = new char [bytes + 1];
				for (int k = 0; k <= bytes; k++) rval[k] = linebuffer1[k];
				delete [] linebuffer1;

				return rval;
			}

			if (inpval != 13 && inpval != 10)
			{
				linebuffer1[bytes] = (char) inpval;
				bytes++;
			}
			else
			{
				if (inpval == 13) InputNumber(1);

				linebuffer1[bytes] = 0;

				rval = new char [bytes + 1];
				for (int k = 0; k <= bytes; k++) rval[k] = linebuffer1[k];
				delete [] linebuffer1;

				return rval;
			}
		}

		linebuffer2 = new char [bytes];

		for (int l = 0; l < bytes; l++) linebuffer2[l] = linebuffer1[l];

		delete [] linebuffer1;

		linebuffer1 = new char [bytes + 1024];

		for (int k = 0; k < bytes; k++) linebuffer1[k] = linebuffer2[k];

		delete [] linebuffer2;
	}

	return NULL;
}

char *InStream::InputSocketString()
{
	if (!isopen)		return NULL;
	if (!socketstream)	return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) CompletePBD();

	if (crosslinked && socketstream) outstream->Flush();

	int	 datasize;

	if (currentpos >= packagesize)
	{
		ReadData();

		if (packagesize == 0) return NULL;

		datasize = packagesize;
	}
	else
	{
		datasize = packagesize - currentpos;
	}

	if (datasize == 0) return NULL;

	char	*rval = new char [datasize + 1];

	for (int i = 0; i < datasize; i++)
	{
		rval[i] = (char) InputNumber(1);
	}

	rval[datasize] = 0;

	return rval;
}

void *InStream::InputData(void *pointer, int bytes)
{
	if (!isopen) return NULL;
	if (streamtype == STREAM_SERVER || streamtype == STREAM_SOCKS5_BIND || streamtype == STREAM_SOCKS4_BIND) return NULL;

	if (pbd && !holdpbd) CompletePBD();

	if (bytes == 0) return NULL;

	if (crosslinked && socketstream) outstream->Flush();

	int	 bytesleft = bytes;
	int	 databufferpos = 0;
	int	 amount = 0;

	while (bytesleft)
	{
		if (currentfilepos >= size) return NULL;

		if (currentpos >= packagesize) ReadData();
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

void InStream::InitPBD()
{
	pbdlen	= 0;
	pbd	= 1;
}

void InStream::CompletePBD()
{
	pbdlen	= 0;
	pbd	= 0;
}

bool InStream::SetPackageSize(int newPackagesize)
{
	if (!isopen)			return false;
	if (!allowpackset)		return false;
	if (newPackagesize <= 0)	return false;

	if (pbd) CompletePBD();

	delete [] data;

	data = new unsigned char [newPackagesize];

	packagesize = newPackagesize;
	stdpacksize = packagesize;

	Seek(currentfilepos);

	return true;
}

bool InStream::SetFilter(IOLibFilter *newFilter)
{
	if (!isopen)		return false;
	if (socketstream)	return false; // Filters are not allowed on socket streams currently

	filter = newFilter;

	allowpackset = true;

	if (filter->packsize > 0)
	{
		SetPackageSize(filter->packsize);

		allowpackset = false;
	}
	else if (filter->packsize == -1)
	{
		SetPackageSize(size - currentfilepos);

		allowpackset = false;
	}

	return true;
}

bool InStream::RemoveFilter()
{
	if (!isopen)		return false;
	if (socketstream)	return false; // Filters are not allowed on socket streams currently

	filter = NULLFILTER;

	allowpackset = true;

	SetPackageSize(origpacksize);

	return true;
}

bool InStream::Close()
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

	if (pbd) CompletePBD();

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

bool InStream::Seek(long position)
{
	if (!isopen)		return false;
	if (socketstream)	return false; // You cannot seek in a socket stream

	if (pbd) CompletePBD();

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, position, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, position, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, position, FILE_BEGIN);
#endif

	currentfilepos	= position;
	bufferpos	= position;
	origfilepos	= position;

	ReadData();

	return true;
}

bool InStream::RelSeek(long offset)
{
	if (!isopen)		return false;
	if (socketstream)	return false; // You cannot seek in a socket stream

	if (pbd) CompletePBD();

	if (buffersize == -1 && !(streamtype == STREAM_ANSI) && !(streamtype == STREAM_WINDOWS))	lseek(stream, currentfilepos + offset, SEEK_SET);
	else if (buffersize == -1 && streamtype == STREAM_ANSI)						fseek(hlstream, currentfilepos + offset, SEEK_SET);
#ifdef __WIN32__
	else if (buffersize == -1 && streamtype == STREAM_WINDOWS)					_llseek(stream, currentfilepos + offset, FILE_BEGIN);
#endif

	currentfilepos	+= offset;
	bufferpos	+= offset;
	origfilepos	= currentfilepos;

	ReadData();

	return true;
}

#endif
