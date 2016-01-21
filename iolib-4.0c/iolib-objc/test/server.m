#include <iolib-objc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

char *itos(int);

int main()
{
	OutStream	*console = [OutStream new];
	OutStream	*out;
	InStream	*in = [InStream new];
	InStream	*client;
	char		*newhost = malloc(1024);
	char		*dummy;

	[console Connect: STREAM_ANSI, stdout];
	[in Connect: STREAM_SERVER, 17000];

	if ([in GetStreamType] == STREAM_NONE)
	{
		[console OutputString: "Error!\n"];
		return 0;
	}

	while (true)
	{
		[console OutputString: "Waiting for client (stop with CTRL-C)..."];

		client = [in WaitForClient];

		if (client == NULL)
		{
			[console OutputString: "Error!\n"];
			return 0;
		}

		out = [OutStream new];
		[out Connect: STREAM_STREAM, client];

		[console OutputString: "OK\nConnected to Client.\nHostname = "];
		[console OutputString: strcpy(newhost, [client GetRemoteHostname])];
		[console OutputString: "\nIP = "];
		[console OutputString: dummy = itos([client GetRemoteIP] & 255)];
		free(dummy);
		[console OutputString: "."];
		[console OutputString: dummy = itos(([client GetRemoteIP] >> 8) & 255)];
		free(dummy);
		[console OutputString: "."];
		[console OutputString: dummy = itos(([client GetRemoteIP] >> 16) & 255)];
		free(dummy);
		[console OutputString: "."];
		[console OutputString: dummy = itos(([client GetRemoteIP] >> 24) & 255)];
		free(dummy);
		[console OutputString: "\nReading 1 byte..."];
		[console OutputString: dummy = itos([client InputNumber: 1])];
		free(dummy);
		[console OutputString: "\n"];

		[console OutputString: "Writing 1 byte..."];
		[out OutputNumber: 121: 1];
		[console OutputString: "OK\n"];

		[console OutputString: "Writing 1 byte..."];
		[out OutputNumber: 113: 1];
		[console OutputString: "OK\n"];

		[client free];
		[out free];
	}

	free(newhost);
	[in free];
	[console free];

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
