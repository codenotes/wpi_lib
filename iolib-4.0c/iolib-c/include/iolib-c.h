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

#ifndef _H_IOLIB_IOLIB_
#define _H_IOLIB_IOLIB_

#include <stdio.h>

#if !defined __BEOS__
	typedef int bool;

	#define false 0
	#define true -1
#endif

#if defined MSDOS
	struct in_addr {
		union {
			struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
			struct { unsigned short s_w1,s_w2; } S_un_w;
			unsigned long S_addr;
		} S_un;
	#define s_addr  S_un.S_addr
	#define s_host  S_un.S_un_b.s_b2
	#define s_net   S_un.S_un_b.s_b1
	#define s_imp   S_un.S_un_w.s_w2
	#define s_impno S_un.S_un_b.s_b4
	#define s_lh    S_un.S_un_b.s_b3
	};

	struct sockaddr_in {
		short			sin_family;
		unsigned short	sin_port;
		struct			in_addr sin_addr;
		char			sin_zero[8];
	};
#endif

#if defined WIN32 && !defined __WIN32__
	#define __WIN32__
#endif

#if defined __WIN32__ && !defined __CYGWIN32__
	#include <io.h>
	#include <winsock.h>
#else
	#if defined __CYGWIN32__
		#include <sys/ioctl.h>
	#endif
	#if defined __FreeBSD__
		#include <sys/types.h>
	#endif
	#include <netinet/in.h>
	#if !defined MSDOS
		#include <sys/socket.h>
		#include <netdb.h>
	#endif
#endif

#if defined __WIN32__
	#include <windows.h>
#endif

#if defined __WIN32__ && !defined i386
	#define i386
#endif

#ifndef STREAM_TYPES
	#define	 STREAM_TYPES

	#define	 STREAM_NONE		0
	#define	 STREAM_FILE		1
	#define	 STREAM_POSIX		2
	#define	 STREAM_ANSI		3
	#define	 STREAM_WINDOWS		4
	#define	 STREAM_SOCKET		5
	#define	 STREAM_SERVER		6
	#define	 STREAM_CLIENT		7
	#define	 STREAM_STREAM		8
	#define	 STREAM_BUFFER		9
	#define	 STREAM_CHILD		10
	#define	 STREAM_SOCKS4_CLIENT	11
	#define	 STREAM_SOCKS4_BIND	12
	#define	 STREAM_SOCKS5_CLIENT	13
	#define	 STREAM_SOCKS5_BIND	14

	#define	 STREAM_SOCKS4_NOAUTH	1
	#define	 STREAM_SOCKS5_NOAUTH	2
	#define	 STREAM_SOCKS5_AUTH	3
#endif

#ifndef IOLIB_MODES
	#define	 IOLIB_MODES

	#define	 MODE_SOCKET_BLOCKING		0
	#define	 MODE_SOCKET_NONBLOCKING	1
#endif

#ifndef DEFAULT_PACKAGE_SIZE
	#define	 DEFAULT_PACKAGE_SIZE	131072
#endif

#if !defined SOCKET && !defined __WIN32__
	typedef unsigned int SOCKET;
#endif

#include "iolibfilter-c.h"
#include "iolibstream-c.h"
#include "instream-c.h"
#include "outstream-c.h"

#if defined __CYGWIN32__ && defined IOLIB_DLL
	#include <windows.h>

	BOOL APIENTRY _cygwin_dll_entry(HINSTANCE, DWORD, LPVOID);
#endif

#if defined __LCC__ && defined IOLIB_DLL
	#include <windows.h>

	BOOL APIENTRY LibMain(HINSTANCE, DWORD, LPVOID);
#endif

extern int initialized;

#if defined __CYGWIN32__ || defined linux || defined __BEOS__ || defined MSDOS || defined __FreeBSD__
	long tell(int);
#endif

#ifndef MSDOS
	unsigned long GetIPAddress(const char *);
#endif

unsigned char GetByte(unsigned long, int);
bool GetBit(unsigned long, int);

#endif
