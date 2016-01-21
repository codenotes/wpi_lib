#ifndef _IOLIB_XORFILTER_
#define _IOLIB_XORFILTER_

#include <iolib-c.h>

#define FilterXOR_New(x)	x=malloc(sizeof(FilterXOR));FilterXOR_Init_real(x);
#define FilterXOR_Free(x)	FilterXOR_Free_real(x);free(x);

typedef struct _FilterXOR
{
	int		 modifier;
	IOLibFilter	*filter;
} FilterXOR;

void FilterXOR_Init_real(FilterXOR *);
void FilterXOR_Free_real(FilterXOR *);
void FilterXOR_EncodeProc(IOLibFilter *, unsigned char **, int, int *, void *);
void FilterXOR_DecodeProc(IOLibFilter *, unsigned char **, int, int *, void *);
void FilterXOR_SetModifier(FilterXOR *, int);
IOLibFilter *FilterXOR_GetFilter(FilterXOR *);

#endif
