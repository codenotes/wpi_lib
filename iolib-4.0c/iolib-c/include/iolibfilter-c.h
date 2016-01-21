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

#ifndef _H_IOLIB_FILTERS_
#define _H_IOLIB_FILTERS_

#include "iolib-c.h"
#include <stdlib.h>

#define IOLibFilter_New(x)		x=malloc(sizeof(IOLibFilter));IOLibFilter_Init_real(x);
#define IOLibFilter_Free(x)		IOLibFilter_Free_real(x);free(x);

typedef struct _IOLibFilter
{
	int	 packsize;
	void	 (*encodeproc)(void *, unsigned char **, int, int *, void *);
	void	 (*decodeproc)(void *, unsigned char **, int, int *, void *);
	void	*pointer;
} IOLibFilter;

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void IOLibFilter_Init_real(IOLibFilter *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void IOLibFilter_Free_real(IOLibFilter *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void IOLibFilter_SetEncodeProc(IOLibFilter *, void (*)(IOLibFilter *, unsigned char **, int, int *, void *), void *);

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

void IOLibFilter_SetDecodeProc(IOLibFilter *, void (*)(IOLibFilter *, unsigned char **, int, int *, void *), void *);

void IOLibFilter_DefaultEncodeProc(IOLibFilter *, unsigned char **, int, int *, void *);
void IOLibFilter_DefaultDecodeProc(IOLibFilter *, unsigned char **, int, int *, void *);

#endif
