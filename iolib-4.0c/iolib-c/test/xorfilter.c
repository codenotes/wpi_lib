#include <iolib-c.h>
#include "xorfilter.h"

void FilterXOR_Init_real(FilterXOR *filter)
{
	IOLibFilter_New(filter->filter);

	filter->filter->packsize = 4;

	IOLibFilter_SetEncodeProc(filter->filter, FilterXOR_EncodeProc, (void *) filter);
	IOLibFilter_SetDecodeProc(filter->filter, FilterXOR_DecodeProc, (void *) filter);

	filter->modifier = 0;
}

void FilterXOR_Free_real(FilterXOR *filter)
{
	IOLibFilter_Free(filter->filter);
}

void FilterXOR_EncodeProc(IOLibFilter *dummy, unsigned char **data, int size, int *outsize, void *filter)
{
	unsigned int value;

	*outsize = size;

	value = (*data)[3] + 256 * (*data)[2] + 65536 * (*data)[1] + 16777216 * (*data)[0];

	value ^= ((FilterXOR *) filter)->modifier;

	(*data)[3] = value % 256;
	(*data)[2] = value % 16777216 % 65536 / 256;
	(*data)[1] = value % 16777216 / 65536;
	(*data)[0] = value / 16777216;
}

void FilterXOR_DecodeProc(IOLibFilter *dummy, unsigned char **data, int size, int *outsize, void *filter)
{
	FilterXOR_EncodeProc(dummy, data, size, outsize, filter);
}

void FilterXOR_SetModifier(FilterXOR *filter, int mod)
{
	filter->modifier = mod;
}

IOLibFilter *FilterXOR_GetFilter(FilterXOR *filter)
{
	return filter->filter;
}
