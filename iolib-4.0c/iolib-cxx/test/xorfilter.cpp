#include <iolib-cxx.h>
#include "xorfilter.h"

FilterXOR::FilterXOR()
{
	packsize = 4;
	modifier = 0;
}

FilterXOR::~FilterXOR()
{
}

void FilterXOR::EncodeData(unsigned char **data, int size, int *outsize)
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

void FilterXOR::DecodeData(unsigned char **data, int size, int *outsize)
{
	EncodeData(data, size, outsize);
}

void FilterXOR::SetModifier(int mod)
{
	modifier = mod;
}
