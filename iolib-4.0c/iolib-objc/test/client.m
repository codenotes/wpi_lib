#include <iolib-objc.h>
#include <math.h>
#include <stdlib.h>

char *itos(int);

int main()
{
	OutStream	*console = [OutStream new];
	OutStream	*out = [OutStream new];
	InStream	*in;
	char		*dummy;

	[out Connect: STREAM_CLIENT, "127.0.0.1", 17000];
	[console Connect: STREAM_ANSI, stdout];

	if ([out GetStreamType] == STREAM_NONE)
	{
		[console OutputString: "Error!\n"];
		return 0;
	}

	[console OutputString: "Writing 1 byte..."];

	if (![out OutputNumber: 214: 1])
	{
		[console OutputString: "Error!\n"];
		return 0;
	}

	[console OutputString: "OK\n"];
	
	[out Flush];

	in = [InStream new];
	[in Connect: STREAM_STREAM, out];

	[console OutputString: "Reading 1 byte..."];
	[console OutputString: dummy = itos([in InputNumber: 1])];
	free(dummy);
	[console OutputString: "\n"];

	[out free];

	[console OutputString: "Reading 1 byte..."];
	[console OutputString: dummy = itos([in InputNumber: 1])];
	free(dummy);
	[console OutputString: "\n"];

	[in free];
	[console free];

	return 1;
}

char *itos(int number)
{
	int	 sz;
	int	 i;
	char	*s = NULL;

	if (number == 0)	sz = 1;
	else if (number < 0)	sz = (int) log10(-number) + 2;
	else			sz = (int) log10(number) + 1;

	s = malloc(sz + 1);

	if (number < 0)
	{
		s[0] = 45;

		for (i = 0; i < sz-1; i++)
		{
			s[i + 1] = (char) floor(((-number) % (int) pow(10, sz - i - 1)) / pow(10, sz - (i + 1) - 1)) + 48;
		}
	}
	else
	{
		for (i = 0; i < sz; i++)
		{
			s[i] = (char) floor((number % (int) pow(10, sz - i)) / pow(10, sz - (i + 1))) + 48;
		}
	}

	s[sz] = 0;

	return s;
}
