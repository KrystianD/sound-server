#include "capture.h"

#include <stdio.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

pa_simple *s_in = 0;

bool init()
{
	pa_sample_spec ss;
	int error;
	
	ss.format = PA_SAMPLE_S16LE;
	ss.rate = 44100;
	ss.channels = 2;
	
	if (!(s_in = pa_simple_new(NULL, "default", PA_STREAM_RECORD,
	                           NULL, "capture", &ss, NULL, NULL, &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		return false;
	}
	
	onSetFormat(ss.rate, FORMAT_INT16, ss.channels);
	
	return true;
}

void initSocket()
{
}

bool process()
{
	int numFrames = 100;
	char buffer[numFrames * 2 * 2];
	
	printf("r\r\n");
	int r = pa_simple_read(s_in, buffer, sizeof(buffer), 0);
	if (r == 0)
	{
		onNewData(buffer, numFrames);
	}
}
