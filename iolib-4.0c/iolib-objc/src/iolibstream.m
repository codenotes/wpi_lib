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

#ifndef __IOLIB_STREAM_
#define __IOLIB_STREAM_

#include <iolib-objc.h>
#include <string.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#if !defined __WIN32__
	#include <sys/ioctl.h>
#endif

@implementation IOLibStream

- init
{
	[super init];

	NULLFILTER = [IOLibFilter new];

	streamtype	= STREAM_NONE;
	socketstream	= false;
	isopen		= false;
	size		= 0;
	currentfilepos	= 0;
	currentpos	= 0;
	buffer		= NULL;
	bufferpos	= 0;
	buffersize	= -1;
	pbd		= false;
	holdpbd		= false;
	pbdlen		= 0;
	data		= NULL;
	mysocket	= 0;
	localSocket	= 0;
	closefile	= true;
	crosslinked	= false;
	connected	= false;
	filter		= NULLFILTER;
	allowpackset	= true;
	packagesize	= DEFAULT_PACKAGE_SIZE;
	stdpacksize	= packagesize;
	origpacksize	= packagesize;
	origsize	= 0;
	origfilepos	= 0;

	faraddr.sin_addr.s_addr = 0;
	faraddr.sin_family = 0;
	faraddr.sin_port = 0;
	memset((void *) faraddr.sin_zero, 0, 8);

	return self;
}

- free
{
	[NULLFILTER free];

	return [super free];
}

- (int) GetStreamType
{
	return streamtype;
}

- (long) Size
{
	if (!isopen)		return -1;
	if (socketstream)	return -1;

	return size;
}

- (long) GetPos
{
	if (!isopen)		return -1;
	if (socketstream)	return -1;

	return currentfilepos;
}

- (bool) SetMode: (long) mode
{
	unsigned long	 var;

	if (!isopen)		return false;
	if (!socketstream)	return false;

#if defined MSDOS | defined __BEOS__ | defined __svr4__
	return false;
#else

	switch (mode)
	{
		case MODE_SOCKET_BLOCKING:
			var = 0;

#if defined __WIN32__
			if (ioctlsocket(mysocket, FIONBIO, &var) != 0) return false;
#else
			if (ioctl(mysocket, FIONBIO, &var) != 0) return false;
#endif
			break;
		case MODE_SOCKET_NONBLOCKING:
			var = 1;

#if defined __WIN32__
			if (ioctlsocket(mysocket, FIONBIO, &var) != 0) return false;
#else
			if (ioctl(mysocket, FIONBIO, &var) != 0) return false;
#endif

			break;
		default:
			return false;
	}

	return true;
#endif
}

- (const char *) GetRemoteHostname
{
#ifdef MSDOS
	return NULL;
#else
	if (faraddr.sin_addr.s_addr == 0) return NULL;

	return gethostbyaddr((char *) &faraddr.sin_addr.s_addr, 4, AF_INET)->h_name;
#endif
}

- (unsigned long) GetRemoteIP
{
#ifdef MSDOS
	return 0;
#else
	if (faraddr.sin_addr.s_addr == 0) return 0;

	return faraddr.sin_addr.s_addr;
#endif
}

- (void) CloseSocket: (SOCKET) sock
{
	if (!socketstream) return;

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

@end

#endif
