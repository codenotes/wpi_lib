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

#ifndef __IOLIB_TOOLS_
#define __IOLIB_TOOLS_

#include <iolib-objc.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

int initialized = 0;

#if defined __CYGWIN32__ || defined linux || defined __BEOS__ || defined MSDOS || defined __FreeBSD__
	long tell(int file)
	{
		return lseek(file, 0, 1);
	}
#endif

#ifndef MSDOS
	unsigned long GetIPAddress(const char *host)
	{
		unsigned long	 ipaddr = 0;
		int		 part1chars = 0;
		int		 part2chars = 0;
		int		 part3chars = 0;
		int		 part4chars = 0;
		int		 part1 = 0;
		int		 part2 = 0;
		int		 part3 = 0;
		int		 part4 = 0;
		int		 i;

		if (strlen(host) >= 7)
		{
			for (i = 0; i < 3; i++)
			{
				if (host[i] >= 48 && host[i] <= 57) part1chars++;
				else break;
			}
		}

		if (strlen(host) >= (unsigned int) part1chars + 6)
		{
			if (host[part1chars] == '.')
			{
				for (i = part1chars + 1; i < part1chars + 4; i++)
				{
					if (host[i] >= 48 && host[i] <= 57) part2chars++;
					else break;
				}
			}
		}

		if (strlen(host) >= (unsigned int) part2chars + (unsigned int) part2chars + 5)
		{
			if (host[part1chars + part2chars + 1] == '.')
			{
				for (i = part1chars + part2chars + 2; i < part1chars + part2chars + 5; i++)
				{
					if (host[i] >= 48 && host[i] <= 57) part3chars++;
					else break;
				}
			}
		}

		if (strlen(host) >= (unsigned int) part1chars + (unsigned int) part2chars + (unsigned int) part3chars + 4)
		{
			if (host[part1chars + part2chars + part3chars + 2] == '.')
			{
				for (i = part1chars + part2chars + part3chars + 3; i < part1chars + part2chars + part3chars + 6; i++)
				{
					if (host[i] == 0) break;

					if (host[i] >= 48 && host[i] <= 57) part4chars++;
					else break;
				}
			}
		}

		if (part1chars > 0 && part2chars > 0 && part3chars > 0 && part4chars > 0)
		{
			for (i = 0; i < part1chars; i++)	part1 += (int) pow(10, (part1chars - 1) - i) * (host[i] - 48);
			for (i = 0; i < part2chars; i++)	part2 += (int) pow(10, (part2chars - 1) - i) * (host[part1chars + i + 1] - 48);
			for (i = 0; i < part3chars; i++)	part3 += (int) pow(10, (part3chars - 1) - i) * (host[part1chars + part2chars + i + 2] - 48);
			for (i = 0; i < part4chars; i++)	part4 += (int) pow(10, (part4chars - 1) - i) * (host[part1chars + part2chars + part3chars + i + 3] - 48);
		}

		if (part1 >= 0 && part1 <= 255 && part2 >= 0 && part2 <= 255 && part3 >= 0 && part3 <= 255 && part4 >= 0 && part4 <= 255)
		{
			ipaddr = ntohl(part1 + 256 * part2 + 65536 * part3 + 16777216 * part4);
		}

		return ipaddr;
	}
#endif

unsigned char GetByte(unsigned long number, int byte)
{
	if (byte > 3) return false;

	return (unsigned char) ((number >> (8 * byte)) % 256);
}

bool GetBit(unsigned long number, int bit)
{
	if (bit > 31) return false;

	return (number >> bit) & 1;
}

#endif
