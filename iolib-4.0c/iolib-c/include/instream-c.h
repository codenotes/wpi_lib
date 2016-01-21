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

#ifndef _H_IOLIB_INSTREAM_
#define _H_IOLIB_INSTREAM_

#include "iolib-c.h"
#include <stdlib.h>

#define InputNumberIntel InputNumber

#define InStream_New(x)		x=malloc(sizeof(InStream));InStream_Init_real(x);
#define InStream_Free(x)	InStream_Free_real(x);free(x);

typedef struct _InStream
{
	void		*outstream;
	IOLibStream	*iostream;
} InStream;

void		 InStream_InitPBD		(InStream *);
void		 InStream_CompletePBD		(InStream *);
void		 InStream_ReadData		(InStream *);
void		 InStream_CloseSocket		(InStream *, SOCKET);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_InputNumber		(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_InputNumberRaw	(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_InputNumberVAX	(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_InputNumberPBD	(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

char		*InStream_InputString		(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

char		*InStream_InputLine		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

char		*InStream_InputSocketString	(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		*InStream_InputData		(InStream *, void *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_SetPackageSize	(InStream *, int);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_SetFilter		(InStream *, IOLibFilter *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_RemoveFilter		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_Close			(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_GetPos		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_Seek			(InStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_RelSeek		(InStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

InStream	*InStream_WaitForClient		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 InStream_Connect		(InStream *, int, ...);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

long		 InStream_Size			(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

bool		 InStream_SetMode		(InStream *, long);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

int		 InStream_GetStreamType		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

const char	*InStream_GetRemoteHostname	(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

unsigned long	 InStream_GetRemoteIP		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 InStream_Init_real		(InStream *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void		 InStream_Free_real		(InStream *);

#endif
