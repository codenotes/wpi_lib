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

#ifndef _H_IOLIB_STREAM_
#define _H_IOLIB_STREAM_

class IOLibStream;

#include "iolib-cxx.h"

class

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

IOLibStream
{
	protected:
		int			 streamtype;
		bool			 socketstream;
		bool			 isopen;
		long			 size;
		long			 origsize;
		long			 currentfilepos;
		long			 origfilepos;
		long			 currentpos;
		unsigned char		*data;
		SOCKET			 socket;
		int			 stream;
		FILE			*hlstream;
		void			*buffer;
		int			 packagesize;
		int			 stdpacksize;
		int			 origpacksize;
		long			 bufferpos;
		long			 buffersize;
		bool			 pbd;
		bool			 holdpbd;
		int			 pbdlen;
		bool			 pbdbuffer[128];
		bool			 allowpackset;
		bool			 closefile;
		bool			 crosslinked;
		bool			 connected;
		SOCKET			 localSocket;
		IOLibFilter		*filter;
		IOLibFilter		*NULLFILTER;
		sockaddr_in		 faraddr;

		void			 CloseSocket		(SOCKET);
	public:
		long			 Size			();
		long			 GetPos			();
		int			 GetStreamType		();
		const char		*GetRemoteHostname	();
		unsigned long		 GetRemoteIP		();
		bool			 SetMode		(long);
					 IOLibStream		();
					~IOLibStream		();
};

#endif
