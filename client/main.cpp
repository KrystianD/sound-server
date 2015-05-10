#include <stdio.h>
#include <string.h>

#include "UdpSocket.h"
#include "capture.h"
#include "resample.h"

UdpSocket socket;
int outSampleRate = 44100;

int srcSampleRate, srcFormat, srcChannels;
void onSetFormat(int sampleRate, int format, int channels)
{
	srcSampleRate = sampleRate;
	srcFormat = format;
	srcChannels = channels;
	printf("sampleRate %d format %d ch %d\n", sampleRate, format, channels);
}
void onNewData(const void* data, int framesCnt)
{
	int16_t *samples = 0;
	int samplesCnt = framesCnt * 2;
	
	if (srcFormat == FORMAT_FLOAT)
	{
		float *origSamples = (float*)data;
		
		samples = new int16_t[samplesCnt];
		
		for (int i = 0; i < samplesCnt; i++)
			samples[i] = origSamples[i] * 32767;
	}
	
	if (samples)
	{
		int inBytes = samplesCnt * sizeof(int16_t);
		uint8_t *outData = new uint8_t[inBytes * (outSampleRate + srcSampleRate - 1) / srcSampleRate];
		int outSamplesCnt = resampleCH2S16LE(samplesCnt, srcSampleRate, samples, outSampleRate, outData);
		socket.sendData("192.168.1.108", 14141, outData, outSamplesCnt * sizeof(int16_t));
		delete [] samples;
		delete [] outData;
	}
}

int main()
{
	initSocket();
	init();
	
	socket.init();
	
	for (;;)
	{
		process();
	}

	return 0;
}
