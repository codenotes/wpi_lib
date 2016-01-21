#include <iolib-cxx.h>
#include <math.h>

char *itos(int);

int main()
{
	OutStream	*out = new OutStream(STREAM_CLIENT, "127.0.0.1", 17000);
	OutStream	*console = new OutStream(STREAM_ANSI, stdout);
	char		*dummy;

	if (out->GetStreamType() == STREAM_NONE)
	{
		console->OutputString("Error!\n");
		return 0;
	}

	console->OutputString("Writing 1 byte...");

	if (!out->OutputNumber(214, 1))
	{
		console->OutputString("Error!\n");
		return 0;
	}

	console->OutputString("OK\n");
	
	out->Flush();

	InStream *in = new InStream(STREAM_STREAM, out);

	console->OutputString("Reading 1 byte...");
	console->OutputString(dummy = itos(in->InputNumber(1)));
	delete [] dummy;
	console->OutputString("\n");

	delete out;

	console->OutputString("Reading 1 byte...");
	console->OutputString(dummy = itos(in->InputNumber(1)));
	delete [] dummy;
	console->OutputString("\n");

	delete in;
	delete console;

	return 1;
}

char *itos(int number)
{
	int	 sz;
	char	*s = NULL;

	if (number == 0)	sz = 1;
	else if (number < 0)	sz = (int) log10(-number) + 2;
	else			sz = (int) log10(number) + 1;

	s = new char [sz + 1];

	if (number < 0)
	{
		s[0] = 45;

		for (int i = 0; i < sz-1; i++)
		{
			s[i + 1] = (char) floor(((-number) % (int) pow(10, sz - i - 1)) / pow(10, sz - (i + 1) - 1)) + 48;
		}
	}
	else
	{
		for (int i = 0; i < sz; i++)
		{
			s[i] = (char) floor((number % (int) pow(10, sz - i)) / pow(10, sz - (i + 1))) + 48;
		}
	}

	s[sz] = 0;

	return s;
}
