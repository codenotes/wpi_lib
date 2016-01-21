#include <iolib-c.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

char *itos(int);

int main()
{
	OutStream	*console;
	OutStream	*out;
	InStream	*in;
	InStream	*client;
	char		*newhost = malloc(1024);
	char		*dummy;

	InStream_New(in);
	InStream_Connect(in, STREAM_SERVER, 17000);

	OutStream_New(console);
	OutStream_Connect(console, STREAM_ANSI, stdout);

	if (InStream_GetStreamType(in) == STREAM_NONE)
	{
		OutStream_OutputString(console, "Error!\n");
		return 0;
	}

	while (true)
	{
		OutStream_OutputString(console, "Waiting for client (stop with CTRL-C)...");

		client = InStream_WaitForClient(in);

		if (client == NULL)
		{
			OutStream_OutputString(console, "Error!\n");
			return 0;
		}

		OutStream_New(out);
		OutStream_Connect(out, STREAM_STREAM, client);

		OutStream_OutputString(console, "OK\nConnected to Client.\nHostname = ");
		OutStream_OutputString(console, strcpy(newhost, InStream_GetRemoteHostname(client)));
		OutStream_OutputString(console, "\nIP = ");
		OutStream_OutputString(console, dummy = itos(InStream_GetRemoteIP(client) & 255));
		free(dummy);
		OutStream_OutputString(console, ".");
		OutStream_OutputString(console, dummy = itos((InStream_GetRemoteIP(client) >> 8) & 255));
		free(dummy);
		OutStream_OutputString(console, ".");
		OutStream_OutputString(console, dummy = itos((InStream_GetRemoteIP(client) >> 16) & 255));
		free(dummy);
		OutStream_OutputString(console, ".");
		OutStream_OutputString(console, dummy = itos((InStream_GetRemoteIP(client) >> 24) & 255));
		free(dummy);
		OutStream_OutputString(console, "\nReading 1 byte...");
		OutStream_OutputString(console, dummy = itos(InStream_InputNumber(client, 1)));
		free(dummy);
		OutStream_OutputString(console, "\n");

		OutStream_OutputString(console, "Writing 1 byte...");
		OutStream_OutputNumber(out, 121, 1);
		OutStream_OutputString(console, "OK\n");

		OutStream_OutputString(console, "Writing 1 byte...");
		OutStream_OutputNumber(out, 113, 1);
		OutStream_OutputString(console, "OK\n");

		InStream_Free(client);
		OutStream_Free(out);
	}

	free(newhost);

	InStream_Free(in);
	OutStream_Free(console);

	return 0;
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
