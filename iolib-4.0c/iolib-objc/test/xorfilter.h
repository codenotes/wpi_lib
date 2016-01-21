#ifndef _IOLIB_XORFILTER_
#define _IOLIB_XORFILTER_

#include <iolib-objc.h>

@interface FilterXOR : IOLibFilter
{
	@private
		int modifier;
}

- init;
- (void)	EncodeData: (unsigned char **) data: (int) size: (int *) outsize;
- (void)	DecodeData: (unsigned char **) data: (int) size: (int *) outsize;
- (void)	SetModifier: (int) mod;

@end

#endif
