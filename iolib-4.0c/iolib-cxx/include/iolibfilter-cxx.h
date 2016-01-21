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

#ifndef _H_IOLIB_FILTERS_
#define _H_IOLIB_FILTERS_

class IOLibFilter;

#include "iolib-cxx.h"

class

#ifdef __WIN32__
#ifdef IOLIB_DLL
	__declspec (dllexport)
#endif
#endif

IOLibFilter
{
	friend class InStream;
	friend class OutStream;
	protected:
		int		 packsize;
	public:
				 IOLibFilter();
		virtual		~IOLibFilter();
		virtual void	 EncodeData(unsigned char **, int, int *);
		virtual void	 DecodeData(unsigned char **, int, int *);
};

#endif
