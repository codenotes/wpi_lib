#include <iolib-c.h>
#include <math.h>
#include <stdlib.h>

char *itos(int);

int main()
{
	OutStream	*console;
	OutStream	*out;
	InStream	*in;
	char		*dummy;

	OutStream_New(console);
	OutStream_Connect(console, STREAM_ANSI, stdout);

	OutStream_New(out);
	OutStream_Connect(out, STREAM_CLIENT, "127.0.0.1", 17000);

	if (OutStream_GetStreamType(out) == STREAM_NONE)
	{
		OutStream_OutputString(console, "Error!\n");
		return 0;
	}

	OutStream_OutputString(console, "Writing 1 byte...");

	if (!OutStream_OutputNumber(out, 214, 1))
	{
		OutStream_OutputString(console, "Error!\n");
		return 0;
	}

	OutStream_OutputString(console, "OK\n");
	
	OutStream_Flush(out);

	InStream_New(in);
	InStream_Connect(in, STREAM_STREAM, out);

	OutStream_OutputString(console, "Reading 1 byte...");
	OutStream_OutputString(console, dummy = itos(InStream_InputNumber(in, 1)));
	free(dummy);
	OutStream_OutputString(console, "\n");

	OutStream_Free(out);

	OutStream_OutputString(console, "Reading 1 byte...");
	OutStream_OutputString(console, dummy = itos(InStream_InputNumber(in, 1)));
	free(dummy);
	OutStream_OutputString(console, "\n");

	InStream_Free(in);

	OutStream_Free(console);

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
