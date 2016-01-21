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

#ifndef __IOLIB_FILTERS_
#define __IOLIB_FILTERS_

#include <iolib-c.h>
#include <stdlib.h>

void IOLibFilter_Init_real(IOLibFilter *filter)
{
	filter->packsize = 0;
	IOLibFilter_SetEncodeProc(filter, IOLibFilter_DefaultEncodeProc, NULL);
	IOLibFilter_SetDecodeProc(filter, IOLibFilter_DefaultDecodeProc, NULL);
}

void IOLibFilter_Free_real(IOLibFilter *filter)
{
}

void IOLibFilter_SetEncodeProc(IOLibFilter *filter, void (*proc)(IOLibFilter *, unsigned char **, int, int *, void *), void *pointer)
{
	filter->encodeproc = (void (*)(void *, unsigned char **, int, int *, void *)) proc;
	filter->pointer = pointer;
}

void IOLibFilter_SetDecodeProc(IOLibFilter *filter, void (*proc)(IOLibFilter *, unsigned char **, int, int *, void *), void *pointer)
{
	filter->decodeproc = (void (*)(void *, unsigned char **, int, int *, void *)) proc;
	filter->pointer = pointer;
}

void IOLibFilter_DefaultEncodeProc(IOLibFilter *filter, unsigned char **data, int size, int *outsize, void *dummy)
{
	*outsize = size;
}

void IOLibFilter_DefaultDecodeProc(IOLibFilter *filter, unsigned char **data, int size, int *outsize, void *dummy)
{
	*outsize = size;
}

#endif
