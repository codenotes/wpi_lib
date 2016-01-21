#include <iolib-objc.h>
#include "xorfilter.h"

@implementation FilterXOR

- init
{
	[super init];

	packsize = 4;
	modifier = 0;

	return self;
}

- (void) EncodeData: (unsigned char **) data: (int) size: (int *) outsize
{
	unsigned int value;

	*outsize = size;

	value = (*data)[3] + 256 * (*data)[2] + 65536 * (*data)[1] + 16777216 * (*data)[0];

	value ^= modifier;

	(*data)[3] = value % 256;
	(*data)[2] = value % 16777216 % 65536 / 256;
	(*data)[1] = value % 16777216 / 65536;
	(*data)[0] = value / 16777216;
}

- (void) DecodeData: (unsigned char **) data: (int) size: (int *) outsize
{
	[self EncodeData: data: size: outsize];
}

- (void) SetModifier: (int) mod
{
	modifier = mod;
}

@end
