#include <iolib-c.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "xorfilter.h"

#if !defined __WIN32__ || defined __CYGWIN32__
	#include <unistd.h>
#endif

#define DATA_SIZE 10

#if defined linux || defined sgi
	#define CLOCK_DIVIDER 1000000
#else
	#define CLOCK_DIVIDER 1000
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

char *itos(int);
char *dtos(double);

int main()
{
	OutStream	*console;
	OutStream	*log;
	OutStream	*out;
	InStream	*in;
	FILE		*ansi_out;
	FILE		*ansi_in;
	int		 posix_out;
	int		 posix_in;
	char		*data;
	time_t		 timer;
	double		 startticks;
	double		 endticks;
	char		*buffer;
	char		*buffer2;
	FilterXOR	*XORFilter;
	char		*dummy;
	int		 i;

#ifdef __WIN32__
	HFILE		 win_out;
	HFILE		 win_in;
#endif

	OutStream_New(console);
	OutStream_Connect(console, STREAM_ANSI, stdout);

	OutStream_New(log);
	OutStream_Connect(log, STREAM_FILE, "test.log", OS_APPEND);

	OutStream_OutputString(console, "IOLib Test version 3.0 for IOLib version 4.0c\n\n");
	OutStream_OutputString(log, "IOLib Test version 3.0 for IOLib version 4.0c\n\n");

	OutStream_OutputString(console, "Creating ");
	OutStream_OutputString(console, dummy = itos(DATA_SIZE));
	free(dummy);
	OutStream_OutputString(console, "MB random data");

	OutStream_OutputString(log, "\tUsing ");
	OutStream_OutputString(log, dummy = itos(DATA_SIZE));
	free(dummy);
	OutStream_OutputString(log, "MB random data\n\n");

	data = malloc(1024 * 1024 * DATA_SIZE);

	srand((unsigned) time(&timer));

	for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
	{
		data[i] = rand() % 255 + 1;

		if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
	}

	OutStream_OutputString(console, "done.\n");

	OutStream_OutputString(log, "\tSpeed tests:\n\n");

	// OutputNumber - 1 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using OutputNumber with 1 byte size");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			OutStream_OutputNumber(out, data[i], 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		OutStream_Flush(out);
		OutStream_Close(out);

		endticks = clock();

		OutStream_Free(out);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tOutputNumber - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// OutputNumber - 4 bytes
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using OutputNumber with 4 bytes size");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			OutStream_OutputNumber(out, *((int *)&(data[i])), 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		OutStream_Flush(out);
		OutStream_Close(out);

		endticks = clock();

		OutStream_Free(out);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tOutputNumber - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// write - 1 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using write with 1 byte size");

		posix_out = open("data.out", O_WRONLY | O_BINARY);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			write(posix_out, (unsigned char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		close(posix_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\twrite - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// write - 4 bytes
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using write with 4 bytes size");

		posix_out = open("data.out", O_WRONLY | O_BINARY);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			write(posix_out, (unsigned char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		close(posix_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\twrite - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// fputc - 1 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using fputc with 1 byte size");

		ansi_out = fopen("data.out", "wb");

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			fputc(data[i], ansi_out);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		fflush(ansi_out);
		fclose(ansi_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tfputc - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// fwrite - 4 bytes
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using fwrite with 4 bytes size");

		ansi_out = fopen("data.out", "wb");

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			fwrite((void *) &(data[i]), 4, 1, ansi_out);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		fflush(ansi_out);
		fclose(ansi_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tfwrite - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

#ifdef __WIN32__
	// _hwrite - 1 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using _hwrite with 1 byte size");

		win_out = _lcreat("data.out", 0);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			_hwrite(win_out, (char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		_lclose(win_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\t_hwrite - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// _hwrite - 4 bytes
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using _hwrite with 4 bytes size");

		win_out = _lcreat("data.out", 0);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			_hwrite(win_out, (char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		_lclose(win_out);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\t_hwrite - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}
#endif

	// OutputNumberPBD - 1 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using OutputNumberPBD with 1 byte size");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			OutStream_OutputNumberPBD(out, data[i], 8);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		OutStream_Flush(out);
		OutStream_Close(out);

		endticks = clock();

		OutStream_Free(out);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tOutputNumberPBD - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// OutputNumberPBD - 4 byte
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using OutputNumberPBD with 4 byte size");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			OutStream_OutputNumberPBD(out, *((int *)&(data[i])), 32);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		OutStream_Flush(out);
		OutStream_Close(out);

		endticks = clock();

		OutStream_Free(out);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tOutputNumberPBD - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// OutputData
	{
		OutStream_OutputString(console, "Writing to \'data.out\' using OutputData...");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		OutStream_OutputData(out, (void *) data, 1024 * 1024 * DATA_SIZE);

		OutStream_Flush(out);
		OutStream_Close(out);

		endticks = clock();

		OutStream_Free(out);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tOutputData\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// InputNumber - 1 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using InputNumber with 1 byte size");

		startticks = clock();

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = (char) InStream_InputNumber(in, 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		InStream_Close(in);

		endticks = clock();

		InStream_Free(in);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tInputNumber - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// InputNumber - 4 bytes
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using InputNumber with 4 bytes size");

		startticks = clock();

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			*((int *)&(data[i])) = InStream_InputNumber(in, 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		InStream_Close(in);

		endticks = clock();

		InStream_Free(in);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tInputNumber - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// read - 1 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using read with 1 byte size");

		startticks = clock();

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			read(posix_in, (unsigned char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		close(posix_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tread - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// read - 4 bytes
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using read with 4 bytes size");

		startticks = clock();

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			read(posix_in, (unsigned char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		close(posix_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tread - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// fgetc - 1 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using fgetc with 1 byte size");

		startticks = clock();

		ansi_in = fopen("data.out", "rb");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = fgetc(ansi_in);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		fclose(ansi_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tfgetc - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// fread - 4 bytes
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using fread with 4 bytes size");

		startticks = clock();

		ansi_in = fopen("data.out", "rb");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			fread((void *) &(data[i]), 4, 1, ansi_in);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		fclose(ansi_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tfread - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

#ifdef __WIN32__
	// _hread - 1 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using _hread with 1 byte size");

		startticks = clock();

		win_in = _lopen("data.out", OF_READ);

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			_hread(win_in, (void *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		_lclose(win_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\t_hread - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// _hread - 4 bytes
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using _hread with 4 bytes size");

		startticks = clock();

		win_in = _lopen("data.out", OF_READ);

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			_hread(win_in, (void *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		_lclose(win_in);

		endticks = clock();

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\t_hread - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}
#endif

	// InputNumberPBD - 1 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using InputNumberPBD with 1 byte size");

		startticks = clock();

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = (char) InStream_InputNumberPBD(in, 8);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		InStream_Close(in);

		endticks = clock();

		InStream_Free(in);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tInputNumberPBD - 1 byte\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// InputNumberPBD - 4 byte
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using InputNumberPBD with 4 byte size");

		startticks = clock();

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		for (i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			*((int *)&(data[i])) = InStream_InputNumberPBD(in, 32);

			if (i % (1024 * 1024) == 0) OutStream_OutputString(console, ".");
		}

		InStream_Close(in);

		endticks = clock();

		InStream_Free(in);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tInputNumberPBD - 4 bytes\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	// InputData
	{
		OutStream_OutputString(console, "Reading from \'data.out\' using InputData...");

		startticks = clock();

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		data = (char *) InStream_InputData(in, (void *) data, 1024 * 1024 * DATA_SIZE);

		InStream_Close(in);

		endticks = clock();

		InStream_Free(in);

		OutStream_OutputString(console, "done.\n");

		OutStream_OutputString(console, "\n\tResults:\tTime:\t");
		OutStream_OutputString(console, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(console, " seconds\n\t\t\tRate:\t");
		OutStream_OutputString(console, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(console, " MB/s\n\n");

		OutStream_OutputString(log, "\tInputData\n");
		OutStream_OutputString(log, "\n\t\tResults:\tTime:\t");
		OutStream_OutputString(log, dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		free(dummy);
		OutStream_OutputString(log, " seconds\n\t\t\t\tRate:\t");
		OutStream_OutputString(log, dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		free(dummy);
		OutStream_OutputString(log, " MB/s\n\n");
	}

	free(data);

	OutStream_OutputString(log, "\tIntegrity tests:\n\n");

	buffer = malloc(4);
	buffer2 = malloc(4);

	// Intel Hex Output
	{
		OutStream_OutputString(console, "Output integrity test for Intel Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumber(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumber(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputString(console, "\tThis should be 'ba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumber(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputString(console, "\tThis should be 'cba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumber(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputString(console, "\tThis should be 'dcba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	// Raw/Motorola Hex Output
	{
		OutStream_OutputString(console, "\nOutput integrity test for Raw Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumberRaw(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumberRaw(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputString(console, "\tThis should be 'ab':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumberRaw(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputString(console, "\tThis should be 'abc':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumberRaw(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputString(console, "\tThis should be 'abcd':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	// VAX Hex Output
	{
		OutStream_OutputString(console, "\nOutput integrity test for VAX Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumberVAX(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumberVAX(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputString(console, "\tThis should be 'ba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumberVAX(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputString(console, "\tThis should be 'bac':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumberVAX(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputString(console, "\tThis should be 'badc':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	// Intel Hex Input
	{
		OutStream_OutputString(console, "\nInput integrity test for Intel Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumberRaw(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputNumberRaw(out, InStream_InputNumber(in, 1), 1);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumberRaw(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputNumberRaw(out, InStream_InputNumber(in, 2), 2);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputString(console, "\tThis should be 'ba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumberRaw(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputNumberRaw(out, InStream_InputNumber(in, 3), 3);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputString(console, "\tThis should be 'cba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumberRaw(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputNumberRaw(out, InStream_InputNumber(in, 4), 4);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputString(console, "\tThis should be 'dcba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	// Raw/Motorola Hex Input
	{
		OutStream_OutputString(console, "\nInput integrity test for Raw Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumberRaw(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputNumberRaw(out, InStream_InputNumberRaw(in, 1), 1);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumberRaw(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputNumberRaw(out, InStream_InputNumberRaw(in, 2), 2);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputString(console, "\tThis should be 'ab':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumberRaw(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputNumberRaw(out, InStream_InputNumberRaw(in, 3), 3);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputString(console, "\tThis should be 'abc':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumberRaw(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputNumberRaw(out, InStream_InputNumberRaw(in, 4), 4);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputString(console, "\tThis should be 'abcd':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	// VAX Hex Input
	{
		OutStream_OutputString(console, "\nInput integrity test for VAX Hex format:\n");

		// 1 byte
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_OutputNumberRaw(out, 'a', 1);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 1);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputNumberRaw(out, InStream_InputNumberVAX(in, 1), 1);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 1);

			OutStream_OutputString(console, "\tThis should be 'a':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 2 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_OutputNumberRaw(out, 'a' << 8 | 'b', 2);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 2);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputNumberRaw(out, InStream_InputNumberVAX(in, 2), 2);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 2);

			OutStream_OutputString(console, "\tThis should be 'ba':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 3 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_OutputNumberRaw(out, 'a' << 16 | 'b' << 8 | 'c', 3);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 3);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputNumberRaw(out, InStream_InputNumberVAX(in, 3), 3);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 3);

			OutStream_OutputString(console, "\tThis should be 'bac':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}

		// 4 bytes
		{
			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_OutputNumberRaw(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			OutStream_Close(out);

			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

			OutStream_New(out);
			OutStream_Connect(out, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputNumberRaw(out, InStream_InputNumberVAX(in, 4), 4);

			InStream_Close(in);
			OutStream_Close(out);

			InStream_Free(in);
			OutStream_Free(out);

			InStream_New(in);
			InStream_Connect(in, STREAM_BUFFER, (void *) buffer2, 4);

			OutStream_OutputString(console, "\tThis should be 'badc':\t");

			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
			OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

			OutStream_OutputString(console, "\n");

			InStream_Close(in);

			InStream_Free(in);
		}
	}

	free(buffer);
	free(buffer2);

	OutStream_OutputString(log, "\tAbility tests:\n\n");

	// standard InStream
	{
		OutStream_OutputString(console, "\nAbility test for standard InStreams:\n");

		OutStream_OutputString(console, "\tOpen a normal file...");

		InStream_New(in);
		InStream_Connect(in, STREAM_FILE, "data.out");

		if (InStream_GetStreamType(in) == STREAM_FILE)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");

			InStream_Free(in);
			OutStream_Free(log);
			OutStream_Free(console);

			return 1;
		}

		OutStream_OutputString(console, "\tSeek 1000 bytes forward and report position...");

		InStream_RelSeek(in, 1000);

		if (InStream_GetPos(in) == 1000)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tSeek to position 500 and report position...");

		InStream_Seek(in, 500);

		if (InStream_GetPos(in) == 500)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tReport file size...");

		if (InStream_Size(in) == 1024 * 1024 * DATA_SIZE)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		InStream_Close(in);

		InStream_Free(in);
	}

	// POSIX InStream
	{
		OutStream_OutputString(console, "\nAbility test for POSIX InStreams:\n");

		OutStream_OutputString(console, "\tConnect to a POSIX file handle...");

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		InStream_New(in);
		InStream_Connect(in, STREAM_POSIX, posix_in);

		if (InStream_GetStreamType(in) == STREAM_POSIX)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");

			InStream_Free(in);
			OutStream_Free(log);
			OutStream_Free(console);

			return 1;
		}

		OutStream_OutputString(console, "\tSeek 1000 bytes forward and report position...");

		InStream_RelSeek(in, 1000);

		if (InStream_GetPos(in) == 1000)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tSeek to position 500 and report position...");

		InStream_Seek(in, 500);

		if (InStream_GetPos(in) == 500)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tReport file size...");

		if (InStream_Size(in) == 1024 * 1024 * DATA_SIZE)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		InStream_Free(in);

		close(posix_in);
	}

	// ANSI InStream
	{
		OutStream_OutputString(console, "\nAbility test for ANSI InStreams:\n");

		OutStream_OutputString(console, "\tConnect to an ANSI file handle...");

		ansi_in = fopen("data.out", "rb");

		InStream_New(in);
		InStream_Connect(in, STREAM_ANSI, ansi_in);

		if (InStream_GetStreamType(in) == STREAM_ANSI)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");

			InStream_Free(in);
			OutStream_Free(log);
			OutStream_Free(console);

			return 1;
		}

		OutStream_OutputString(console, "\tSeek 1000 bytes forward and report position...");

		InStream_RelSeek(in, 1000);

		if (InStream_GetPos(in) == 1000)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tSeek to position 500 and report position...");

		InStream_Seek(in, 500);

		if (InStream_GetPos(in) == 500)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tReport file size...");

		if (InStream_Size(in) == 1024 * 1024 * DATA_SIZE)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		InStream_Free(in);

		fclose(ansi_in);
	}

#ifdef __WIN32__
	// Windows InStream
	{
		OutStream_OutputString(console, "\nAbility test for Windows HFILE InStreams:\n");

		OutStream_OutputString(console, "\tConnect to a Windows file handle...");

		win_in = _lopen("data.out", OF_READ);

		InStream_New(in);
		InStream_Connect(in, STREAM_WINDOWS, win_in);

		if (InStream_GetStreamType(in) == STREAM_WINDOWS)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");

			InStream_Free(in);
			OutStream_Free(log);
			OutStream_Free(console);

			return 1;
		}

		OutStream_OutputString(console, "\tSeek 1000 bytes forward and report position...");

		InStream_RelSeek(in, 1000);

		if (InStream_GetPos(in) == 1000)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tSeek to position 500 and report position...");

		InStream_Seek(in, 500);

		if (InStream_GetPos(in) == 500)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		OutStream_OutputString(console, "\tReport file size...");

		if (InStream_Size(in) == 1024 * 1024 * DATA_SIZE)
		{
			OutStream_OutputString(console, "OK.\n");
		}
		else
		{
			OutStream_OutputString(console, "error!\n");
		}

		InStream_Free(in);

		_lclose(win_in);
	}
#endif

	OutStream_OutputString(log, "\tFilter tests:\n\n");

	buffer = malloc(4);

	FilterXOR_New(XORFilter);
	FilterXOR_SetModifier(XORFilter, 1373256436);

	// Writing filtered data
	{
		OutStream_OutputString(console, "\nWriting filtered data:\n");

		OutStream_New(out);
		OutStream_Connect(out, STREAM_BUFFER, (void *) buffer, 4);

		OutStream_SetFilter(out, FilterXOR_GetFilter(XORFilter));
		OutStream_OutputNumberRaw(out, 'a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

		OutStream_Close(out);

		OutStream_Free(out);

		InStream_New(in);
		InStream_Connect(in, STREAM_BUFFER, (void *) buffer, 4);

		OutStream_OutputString(console, "\tThis should be garbage:\t");

		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

		OutStream_OutputString(console, "\n");

		InStream_Seek(in, 0);
		InStream_SetFilter(in, FilterXOR_GetFilter(XORFilter));

		OutStream_OutputString(console, "\tThis should be 'abcd':\t");

		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);
		OutStream_OutputNumber(console, InStream_InputNumber(in, 1), 1);

		OutStream_OutputString(console, "\n");

		InStream_Close(in);

		InStream_Free(in);
	}

	FilterXOR_Free(XORFilter);

	free(buffer);

	OutStream_OutputString(log, "IOLib Test finished.\n\n");

	remove("data.out");

	OutStream_Free(console);
	OutStream_Free(log);

	return 0;
}

char *itos(int number)
{
	int	 sz;
	char	*s = NULL;
	int	 i;

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

char *dtos(double number)
{
	int		 sz;
	int		 afpslen;
	int		 lastnn = 0;
	char		*s = NULL;
	char		*afps = NULL;
	int		 i;

	if ((long) number == 0)	sz = 1;
	else if (number < 0)	sz = (int) log10(-number) + 2;
	else			sz = (int) log10(number) + 1;

	s = malloc(sz + 16);

	for (i = 0; i < sz + 16; i++)
	{
		s[i] = 0;
	}

	if (number < 0)
	{
		s[0] = 45;

		for (i = 0; i < sz-1; i++)
		{
			s[i+1] = (char) floor(((long) (-number) % (int) pow(10, sz - i - 1)) / pow(10, sz - (i + 1) - 1)) + 48;
		}

		afps = itos((int) (-(number - (int) number) * (int) pow(10, 9)));
	}
	else
	{
		for (i = 0; i < sz; i++)
		{
			s[i] = (char) floor(((long) number % (int) pow(10, sz - i)) / pow(10, sz - (i + 1))) + 48;
		}

		afps = itos((int) ((number - (int) number) * (int) pow(10, 3)));
	}

	afpslen = strlen(afps);

	for (i = 0; i < afpslen; i++)
	{
		if (afps[i] != 48) lastnn = i + 1;
	}

	afps = strncpy(afps, afps, lastnn);

	if (number - (long) number != 0)
	{
		strcat(s, ".");

		for (i = 0; i < (3 - afpslen); i++)
		{
			strcat(s, "0");
		}

		strcat(s, afps);
	}

	free(afps);

	return s;
}
