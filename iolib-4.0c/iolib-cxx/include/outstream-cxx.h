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

#ifndef _H_IOLIB_OUTSTREAM_
#define _H_IOLIB_OUTSTREAM_

class OutStream;

#include "iolib-cxx.h"

const int	 OS_APPEND	= 0;
const int	 OS_OVERWRITE	= 1;

#define OutputNumberIntel OutputNumber

class

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

OutStream : public IOLibStream
{
	friend class InStream;
	private:
		long			 lastfilepos;
		InStream		*instream;
		void			 InitPBD		();
		void			 CompletePBD		();
		void			 WriteData		();
					 OutStream		(int, OutStream *);
	public:
		bool			 OutputNumber		(long, int);
		bool			 OutputNumberRaw	(long, int);
		bool			 OutputNumberVAX	(long, int);
		bool			 OutputNumberPBD	(long, int);
		bool			 OutputString		(const char *);
		bool			 OutputLine		(const char *);
		bool			 OutputData		(const void *, int);
		bool			 SetPackageSize		(int);
		bool			 SetFilter		(IOLibFilter *);
		bool			 RemoveFilter		();
		bool			 Close			();
		bool			 Seek			(long);
		bool			 RelSeek		(long);
		OutStream		*WaitForClient		();
		void			 Flush			();
					 OutStream		(int, const char *, int);
					 OutStream		(int, int, const char *, unsigned short, const char *, unsigned short, ...);
					 OutStream		(int, int);
					 OutStream		(int, FILE *);
					 OutStream		(int, void *, long);
					 OutStream		(int, InStream *);
					~OutStream		();
};

#endif
