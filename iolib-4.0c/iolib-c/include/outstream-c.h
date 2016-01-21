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

#ifndef _H_IOLIB_OUTSTREAM_
#define _H_IOLIB_OUTSTREAM_

#include "iolib-c.h"
#include <stdlib.h>

#define OutputNumberIntel OutputNumber

#define OutStream_New(x)	x=malloc(sizeof(OutStream));OutStream_Init_real(x);
#define OutStream_Free(x)	OutStream_Free_real(x);free(x);

#define OS_APPEND	0
#define OS_OVERWRITE	1

typedef struct _OutStream
{
	long		 lastfilepos;
	void		*instream;
	IOLibStream	*iostream;

} OutStream;

void		 OutStream_InitPBD		(OutStream *);
void		 OutStream_CompletePBD		(OutStream *);
void		 OutStream_WriteData		(OutStream *);
void		 OutStream_CloseSocket		(OutStream *, SOCKET);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputNumber		(OutStream *, long, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputNumberRaw	(OutStream *, long, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputNumberVAX	(OutStream *, long, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputNumberPBD	(OutStream *, long, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputString		(OutStream *, const char *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputLine		(OutStream *, const char *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_OutputData		(OutStream *, const void *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_SetPackageSize	(OutStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_SetFilter		(OutStream *, IOLibFilter *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_RemoveFilter		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_Close		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 OutStream_GetPos		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_Seek			(OutStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_RelSeek		(OutStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

OutStream	*OutStream_WaitForClient	(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 OutStream_Flush		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 OutStream_Connect		(OutStream *, int, ...);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 OutStream_Size			(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 OutStream_SetMode		(OutStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

int		 OutStream_GetStreamType	(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

const char	*OutStream_GetRemoteHostname	(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

unsigned long	 OutStream_GetRemoteIP		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 OutStream_Free_real		(OutStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 OutStream_Init_real	(OutStream *);

#endif
