#include <iolib-cxx.h>
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
	OutStream	*console = new OutStream(STREAM_ANSI, stdout);
	OutStream	*log = new OutStream(STREAM_FILE, "test.log", OS_APPEND);
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

#ifdef __WIN32__
	HFILE		 win_out;
	HFILE		 win_in;
#endif

	console->OutputString("IOLib Test version 3.0 for IOLib version 4.0c\n\n");
	log->OutputString("IOLib Test version 3.0 for IOLib version 4.0c\n\n");

	console->OutputString("Creating ");
	console->OutputString(dummy = itos(DATA_SIZE));
	delete [] dummy;
	console->OutputString("MB random data");

	log->OutputString("\tUsing ");
	log->OutputString(dummy = itos(DATA_SIZE));
	delete [] dummy;
	log->OutputString("MB random data\n\n");

	data = new char [1024 * 1024 * DATA_SIZE];

	srand((unsigned) time(&timer));

	for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
	{
		data[i] = rand() % 255 + 1;

		if (i % (1024 * 1024) == 0) console->OutputString(".");
	}

	console->OutputString("done.\n");

	log->OutputString("\tSpeed tests:\n\n");

	// OutputNumber - 1 byte
	{
		console->OutputString("Writing to \'data.out\' using OutputNumber with 1 byte size");

		out = new OutStream(STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			out->OutputNumber(data[i], 1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		out->Flush();
		out->Close();

		endticks = clock();

		delete out;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tOutputNumber - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// OutputNumber - 4 bytes
	{
		console->OutputString("Writing to \'data.out\' using OutputNumber with 4 bytes size");

		out = new OutStream(STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			out->OutputNumber(*((int *)&(data[i])), 4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		out->Flush();
		out->Close();

		endticks = clock();

		delete out;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tOutputNumber - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// write - 1 byte
	{
		console->OutputString("Writing to \'data.out\' using write with 1 byte size");

		posix_out = open("data.out", O_WRONLY | O_BINARY);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			write(posix_out, (unsigned char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		close(posix_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\twrite - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// write - 4 bytes
	{
		console->OutputString("Writing to \'data.out\' using write with 4 bytes size");

		posix_out = open("data.out", O_WRONLY | O_BINARY);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			write(posix_out, (unsigned char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		close(posix_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\twrite - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// fputc - 1 byte
	{
		console->OutputString("Writing to \'data.out\' using fputc with 1 byte size");

		ansi_out = fopen("data.out", "wb");

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			fputc(data[i], ansi_out);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		fflush(ansi_out);
		fclose(ansi_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tfputc - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// fwrite - 4 bytes
	{
		console->OutputString("Writing to \'data.out\' using fwrite with 4 bytes size");

		ansi_out = fopen("data.out", "wb");

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			fwrite((void *) &(data[i]), 4, 1, ansi_out);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		fflush(ansi_out);
		fclose(ansi_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tfwrite - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

#ifdef __WIN32__
	// _hwrite - 1 byte
	{
		console->OutputString("Writing to \'data.out\' using _hwrite with 1 byte size");

		win_out = _lcreat("data.out", 0);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			_hwrite(win_out, (char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		_lclose(win_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\t_hwrite - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// _hwrite - 4 bytes
	{
		console->OutputString("Writing to \'data.out\' using _hwrite with 4 bytes size");

		win_out = _lcreat("data.out", 0);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			_hwrite(win_out, (char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		_lclose(win_out);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\t_hwrite - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}
#endif

	// OutputNumberPBD - 1 byte
	{
		console->OutputString("Writing to \'data.out\' using OutputNumberPBD with 1 byte size");

		out = new OutStream(STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			out->OutputNumberPBD(data[i], 8);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		out->Flush();
		out->Close();

		endticks = clock();

		delete out;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tOutputNumberPBD - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// OutputNumberPBD - 4 byte
	{
		console->OutputString("Writing to \'data.out\' using OutputNumberPBD with 4 byte size");

		out = new OutStream(STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			out->OutputNumberPBD(*((int *)&(data[i])), 32);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		out->Flush();
		out->Close();

		endticks = clock();

		delete out;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tOutputNumberPBD - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// OutputData
	{
		console->OutputString("Writing to \'data.out\' using OutputData...");

		out = new OutStream(STREAM_FILE, "data.out", OS_OVERWRITE);

		startticks = clock();

		out->OutputData((void *) data, 1024 * 1024 * DATA_SIZE);

		out->Flush();
		out->Close();

		endticks = clock();

		delete out;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tOutputData\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// InputNumber - 1 byte
	{
		console->OutputString("Reading from \'data.out\' using InputNumber with 1 byte size");

		startticks = clock();

		in = new InStream(STREAM_FILE, "data.out");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = (char) in->InputNumber(1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		in->Close();

		endticks = clock();

		delete in;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tInputNumber - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// InputNumber - 4 bytes
	{
		console->OutputString("Reading from \'data.out\' using InputNumber with 4 bytes size");

		startticks = clock();

		in = new InStream(STREAM_FILE, "data.out");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			*((int *)&(data[i])) = in->InputNumber(4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		in->Close();

		endticks = clock();

		delete in;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tInputNumber - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// read - 1 byte
	{
		console->OutputString("Reading from \'data.out\' using read with 1 byte size");

		startticks = clock();

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			read(posix_in, (unsigned char *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		close(posix_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tread - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// read - 4 bytes
	{
		console->OutputString("Reading from \'data.out\' using read with 4 bytes size");

		startticks = clock();

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			read(posix_in, (unsigned char *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		close(posix_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tread - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// fgetc - 1 byte
	{
		console->OutputString("Reading from \'data.out\' using fgetc with 1 byte size");

		startticks = clock();

		ansi_in = fopen("data.out", "rb");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = fgetc(ansi_in);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		fclose(ansi_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tfgetc - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// fread - 4 bytes
	{
		console->OutputString("Reading from \'data.out\' using fread with 4 bytes size");

		startticks = clock();

		ansi_in = fopen("data.out", "rb");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			fread((void *) &(data[i]), 4, 1, ansi_in);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		fclose(ansi_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tfread - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

#ifdef __WIN32__
	// _hread - 1 byte
	{
		console->OutputString("Reading from \'data.out\' using _hread with 1 byte size");

		startticks = clock();

		win_in = _lopen("data.out", OF_READ);

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			_hread(win_in, (void *) &(data[i]), 1);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		_lclose(win_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\t_hread - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// _hread - 4 bytes
	{
		console->OutputString("Reading from \'data.out\' using _hread with 4 bytes size");

		startticks = clock();

		win_in = _lopen("data.out", OF_READ);

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			_hread(win_in, (void *) &(data[i]), 4);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		_lclose(win_in);

		endticks = clock();

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\t_hread - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}
#endif

	// InputNumberPBD - 1 byte
	{
		console->OutputString("Reading from \'data.out\' using InputNumberPBD with 1 byte size");

		startticks = clock();

		in = new InStream(STREAM_FILE, "data.out");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i++)
		{
			data[i] = (char) in->InputNumberPBD(8);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		in->Close();

		endticks = clock();

		delete in;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tInputNumberPBD - 1 byte\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// InputNumberPBD - 4 byte
	{
		console->OutputString("Reading from \'data.out\' using InputNumberPBD with 4 byte size");

		startticks = clock();

		in = new InStream(STREAM_FILE, "data.out");

		for (int i = 0; i < 1024 * 1024 * DATA_SIZE; i += 4)
		{
			*((int *)&(data[i])) = in->InputNumberPBD(32);

			if (i % (1024 * 1024) == 0) console->OutputString(".");
		}

		in->Close();

		endticks = clock();

		delete in;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tInputNumberPBD - 4 bytes\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	// InputData
	{
		console->OutputString("Reading from \'data.out\' using InputData...");

		startticks = clock();

		in = new InStream(STREAM_FILE, "data.out");

		data = (char *) in->InputData((void *) data, 1024 * 1024 * DATA_SIZE);

		in->Close();

		endticks = clock();

		delete in;

		console->OutputString("done.\n");

		console->OutputString("\n\tResults:\tTime:\t");
		console->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		console->OutputString(" seconds\n\t\t\tRate:\t");
		console->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		console->OutputString(" MB/s\n\n");

		log->OutputString("\tInputData\n");
		log->OutputString("\n\t\tResults:\tTime:\t");
		log->OutputString(dummy = dtos((endticks-startticks)/CLOCK_DIVIDER));
		delete [] dummy;
		log->OutputString(" seconds\n\t\t\t\tRate:\t");
		log->OutputString(dummy = dtos(DATA_SIZE/((endticks-startticks)/CLOCK_DIVIDER)));
		delete [] dummy;
		log->OutputString(" MB/s\n\n");
	}

	delete [] data;

	log->OutputString("\tIntegrity tests:\n\n");

	buffer = new char [4];
	buffer2 = new char [4];

	// Intel Hex Output
	{
		console->OutputString("Output integrity test for Intel Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumber('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumber('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);

			console->OutputString("\tThis should be 'ba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumber('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);

			console->OutputString("\tThis should be 'cba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumber('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);

			console->OutputString("\tThis should be 'dcba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	// Raw/Motorola Hex Output
	{
		console->OutputString("\nOutput integrity test for Raw Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumberRaw('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumberRaw('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);

			console->OutputString("\tThis should be 'ab':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumberRaw('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);

			console->OutputString("\tThis should be 'abc':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumberRaw('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);

			console->OutputString("\tThis should be 'abcd':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	// VAX Hex Output
	{
		console->OutputString("\nOutput integrity test for VAX Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumberVAX('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumberVAX('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);

			console->OutputString("\tThis should be 'ba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumberVAX('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);

			console->OutputString("\tThis should be 'bac':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumberVAX('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);

			console->OutputString("\tThis should be 'badc':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	// Intel Hex Input
	{
		console->OutputString("\nInput integrity test for Intel Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumberRaw('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 1);

			out->OutputNumberRaw(in->InputNumber(1), 1);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumberRaw('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 2);

			out->OutputNumberRaw(in->InputNumber(2), 2);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 2);

			console->OutputString("\tThis should be 'ba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumberRaw('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 3);

			out->OutputNumberRaw(in->InputNumber(3), 3);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 3);

			console->OutputString("\tThis should be 'cba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumberRaw('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 4);

			out->OutputNumberRaw(in->InputNumber(4), 4);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 4);

			console->OutputString("\tThis should be 'dcba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	// Raw/Motorola Hex Input
	{
		console->OutputString("\nInput integrity test for Raw Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumberRaw('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 1);

			out->OutputNumberRaw(in->InputNumberRaw(1), 1);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumberRaw('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 2);

			out->OutputNumberRaw(in->InputNumberRaw(2), 2);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 2);

			console->OutputString("\tThis should be 'ab':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumberRaw('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 3);

			out->OutputNumberRaw(in->InputNumberRaw(3), 3);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 3);

			console->OutputString("\tThis should be 'abc':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumberRaw('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 4);

			out->OutputNumberRaw(in->InputNumberRaw(4), 4);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 4);

			console->OutputString("\tThis should be 'abcd':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	// VAX Hex Input
	{
		console->OutputString("\nInput integrity test for VAX Hex format:\n");

		// 1 byte
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 1);

			out->OutputNumberRaw('a', 1);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 1);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 1);

			out->OutputNumberRaw(in->InputNumberVAX(1), 1);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 1);

			console->OutputString("\tThis should be 'a':\t");

			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 2 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 2);

			out->OutputNumberRaw('a' << 8 | 'b', 2);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 2);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 2);

			out->OutputNumberRaw(in->InputNumberVAX(2), 2);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 2);

			console->OutputString("\tThis should be 'ba':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 3 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 3);

			out->OutputNumberRaw('a' << 16 | 'b' << 8 | 'c', 3);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 3);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 3);

			out->OutputNumberRaw(in->InputNumberVAX(3), 3);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 3);

			console->OutputString("\tThis should be 'bac':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}

		// 4 bytes
		{
			out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

			out->OutputNumberRaw('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

			out->Close();

			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer, 4);
			out = new OutStream(STREAM_BUFFER, (void *) buffer2, 4);

			out->OutputNumberRaw(in->InputNumberVAX(4), 4);

			in->Close();
			out->Close();

			delete in;
			delete out;

			in = new InStream(STREAM_BUFFER, (void *) buffer2, 4);

			console->OutputString("\tThis should be 'badc':\t");

			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);
			console->OutputNumber(in->InputNumber(1), 1);

			console->OutputString("\n");

			in->Close();

			delete in;
		}
	}

	delete [] buffer;
	delete [] buffer2;

	log->OutputString("\tAbility tests:\n\n");

	// standard InStream
	{
		console->OutputString("\nAbility test for standard InStreams:\n");

		console->OutputString("\tOpen a normal file...");

		in = new InStream(STREAM_FILE, "data.out");

		if (in->GetStreamType() == STREAM_FILE)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");

			delete in;
			delete log;
			delete console;

			return 1;
		}

		console->OutputString("\tSeek 1000 bytes forward and report position...");

		in->RelSeek(1000);

		if (in->GetPos() == 1000)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tSeek to position 500 and report position...");

		in->Seek(500);

		if (in->GetPos() == 500)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tReport file size...");

		if (in->Size() == 1024 * 1024 * DATA_SIZE)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		in->Close();

		delete in;
	}

	// POSIX InStream
	{
		console->OutputString("\nAbility test for POSIX InStreams:\n");

		console->OutputString("\tConnect to a POSIX file handle...");

		posix_in = open("data.out", O_RDONLY | O_BINARY);

		in = new InStream(STREAM_POSIX, posix_in);

		if (in->GetStreamType() == STREAM_POSIX)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");

			delete in;
			delete log;
			delete console;

			return 1;
		}

		console->OutputString("\tSeek 1000 bytes forward and report position...");

		in->RelSeek(1000);

		if (in->GetPos() == 1000)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tSeek to position 500 and report position...");

		in->Seek(500);

		if (in->GetPos() == 500)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tReport file size...");

		if (in->Size() == 1024 * 1024 * DATA_SIZE)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		delete in;

		close(posix_in);
	}

	// ANSI InStream
	{
		console->OutputString("\nAbility test for ANSI InStreams:\n");

		console->OutputString("\tConnect to an ANSI file handle...");

		ansi_in = fopen("data.out", "rb");

		in = new InStream(STREAM_ANSI, ansi_in);

		if (in->GetStreamType() == STREAM_ANSI)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");

			delete in;
			delete log;
			delete console;

			return 1;
		}

		console->OutputString("\tSeek 1000 bytes forward and report position...");

		in->RelSeek(1000);

		if (in->GetPos() == 1000)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tSeek to position 500 and report position...");

		in->Seek(500);

		if (in->GetPos() == 500)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tReport file size...");

		if (in->Size() == 1024 * 1024 * DATA_SIZE)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		delete in;

		fclose(ansi_in);
	}

#ifdef __WIN32__
	// Windows InStream
	{
		console->OutputString("\nAbility test for Windows HFILE InStreams:\n");

		console->OutputString("\tConnect to a Windows file handle...");

		win_in = _lopen("data.out", OF_READ);

		in = new InStream(STREAM_WINDOWS, win_in);

		if (in->GetStreamType() == STREAM_WINDOWS)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");

			delete in;
			delete log;
			delete console;

			return 1;
		}

		console->OutputString("\tSeek 1000 bytes forward and report position...");

		in->RelSeek(1000);

		if (in->GetPos() == 1000)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tSeek to position 500 and report position...");

		in->Seek(500);

		if (in->GetPos() == 500)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		console->OutputString("\tReport file size...");

		if (in->Size() == 1024 * 1024 * DATA_SIZE)
		{
			console->OutputString("OK.\n");
		}
		else
		{
			console->OutputString("error!\n");
		}

		delete in;

		_lclose(win_in);
	}
#endif

	log->OutputString("\tFilter tests:\n\n");

	buffer = new char [4];

	XORFilter = new FilterXOR();
	XORFilter->SetModifier(1373256436);

	// Writing filtered data
	{
		console->OutputString("\nWriting filtered data:\n");

		out = new OutStream(STREAM_BUFFER, (void *) buffer, 4);

		out->SetFilter(XORFilter);
		out->OutputNumberRaw('a' << 24 | 'b' << 16 | 'c' << 8 | 'd', 4);

		out->Close();

		delete out;

		in = new InStream(STREAM_BUFFER, (void *) buffer, 4);

		console->OutputString("\tThis should be garbage:\t");

		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);

		console->OutputString("\n");

		in->Seek(0);
		in->SetFilter(XORFilter);

		console->OutputString("\tThis should be 'abcd':\t");

		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);
		console->OutputNumber(in->InputNumber(1), 1);

		console->OutputString("\n");

		in->Close();

		delete in;
	}

	delete XORFilter;

	delete [] buffer;

	log->OutputString("IOLib Test finished.\n\n");

	remove("data.out");

	delete console;
	delete log;

	return 0;
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

char *dtos(double number)
{
	int		 sz;
	int		 afpslen;
	int		 lastnn = 0;
	char		*s = NULL;
	char		*afps = NULL;

	if ((long) number == 0)	sz = 1;
	else if (number < 0)	sz = (int) log10(-number) + 2;
	else			sz = (int) log10(number) + 1;

	s = new char [sz + 16];

	for (int j = 0; j < sz + 16; j++)
	{
		s[j] = 0;
	}

	if (number < 0)
	{
		s[0] = 45;

		for (int i = 0; i < sz-1; i++)
		{
			s[i+1] = (char) floor(((long) (-number) % (int) pow(10, sz - i - 1)) / pow(10, sz - (i + 1) - 1)) + 48;
		}

		afps = itos((int) (-(number - (int) number) * (int) pow(10, 9)));
	}
	else
	{
		for (int i = 0; i < sz; i++)
		{
			s[i] = (char) floor(((long) number % (int) pow(10, sz - i)) / pow(10, sz - (i + 1))) + 48;
		}

		afps = itos((int) ((number - (int) number) * (int) pow(10, 3)));
	}

	afpslen = strlen(afps);

	for (int k = 0; k < afpslen; k++)
	{
		if (afps[k] != 48) lastnn = k + 1;
	}

	afps = strncpy(afps, afps, lastnn);

	if (number - (long) number != 0)
	{
		strcat(s, ".");

		for (int i = 0; i < (3 - afpslen); i++)
		{
			strcat(s, "0");
		}

		strcat(s, afps);
	}

	delete [] afps;

	return s;
}
