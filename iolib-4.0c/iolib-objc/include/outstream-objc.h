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

#ifndef _H_IOLIB_OUTSTREAM_
#define _H_IOLIB_OUTSTREAM_

@class InStream;
@class OutStream;

#include "iolib-objc.h"
#include <objc/Object.h>

#define OS_APPEND	0
#define OS_OVERWRITE	1

#define OutputNumberIntel OutputNumber

@interface OutStream : IOLibStream
{
	@public
		long		 lastfilepos;
		InStream	*instream;
}

- free;
- (void)		InitPBD;
- (void)		CompletePBD;
- (void)		WriteData;
- (bool)		OutputNumber:		(long)		number:		(int)	bytes;
- (bool)		OutputNumberRaw:	(long)		number:		(int)	bytes;
- (bool)		OutputNumberVAX:	(long)		number:		(int)	bytes;
- (bool)		OutputNumberPBD:	(long)		number:		(int)	bits;
- (bool)		OutputString:		(const char *)	string;
- (bool)		OutputLine:		(const char *)	string;
- (bool)		OutputData:		(const void *)	pointer:	(int)	bytes;
- (bool)		SetPackageSize:		(int)		newPackagesize;
- (bool)		SetFilter:		(IOLibFilter *)	newFilter;
- (bool)		RemoveFilter;
- (bool)		Close;
- (bool)		Seek:			(long)		position;
- (bool)		RelSeek:		(long)		offset;
- (OutStream *)		WaitForClient;
- (void)		Flush;
- (void)		Connect:		(int)		type,		...;

@end

#endif
