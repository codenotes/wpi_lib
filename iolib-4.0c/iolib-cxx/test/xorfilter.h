#ifndef _IOLIB_XORFILTER_
#define _IOLIB_XORFILTER_

#include <iolib-cxx.h>

class FilterXOR : public IOLibFilter
{
	private:
		int	 modifier;
	public:
			 FilterXOR();
			~FilterXOR();
		void	 EncodeData(unsigned char **, int, int *);
		void	 DecodeData(unsigned char **, int, int *);
		void	 SetModifier(int);
};

#endif
