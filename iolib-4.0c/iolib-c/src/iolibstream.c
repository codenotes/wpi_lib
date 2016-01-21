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

#ifndef __IOLIB_STREAM_
#define __IOLIB_STREAM_

#include <iolib-c.h>
#include <stdlib.h>
#include <string.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#if !defined __WIN32__
	#include <sys/ioctl.h>
#endif

void IOLibStream_Init(IOLibStream *stream)
{
	IOLibFilter_New(stream->NULLFILTER);

	stream->streamtype	= STREAM_NONE;
	stream->socketstream	= false;
	stream->isopen		= false;
	stream->size		= 0;
	stream->currentfilepos	= 0;
	stream->currentpos	= 0;
	stream->buffer		= NULL;
	stream->bufferpos	= 0;
	stream->buffersize	= -1;
	stream->pbd		= false;
	stream->holdpbd		= false;
	stream->pbdlen		= 0;
	stream->data		= NULL;
	stream->socket		= 0;
	stream->localSocket	= 0;
	stream->closefile	= true;
	stream->crosslinked	= false;
	stream->connected	= false;
	stream->filter		= stream->NULLFILTER;
	stream->allowpackset	= true;
	stream->packagesize	= DEFAULT_PACKAGE_SIZE;
	stream->stdpacksize	= stream->packagesize;
	stream->origpacksize	= stream->packagesize;
	stream->origsize	= 0;
	stream->origfilepos	= 0;

	stream->faraddr.sin_addr.s_addr = 0;
	stream->faraddr.sin_family = 0;
	stream->faraddr.sin_port = 0;
	memset((void *) stream->faraddr.sin_zero, 0, 8);
}

void IOLibStream_Free(IOLibStream *stream)
{
	IOLibFilter_Free(stream->NULLFILTER);
}

#endif
