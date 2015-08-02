#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <thread>

#include "UdpSocket.h"
#include "capture.h"
#include "resample.h"
#include "kdutils.h"

using namespace std;

static UdpSocket socket;
int outSampleRate = 44100;
int volume = 100;

int srcSampleRate, srcFormat, srcChannels;
void onSetFormat(int sampleRate, int format, int channels)
{
	srcSampleRate = sampleRate;
	srcFormat = format;
	srcChannels = channels;
	printf("sampleRate %d format %d ch %d\n", sampleRate, format, channels);
}
void onNewData(void* data, int framesCnt)
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
	if (srcFormat == FORMAT_INT16)
	{
		samples = (int16_t*)data;
	}
	
	if (samples)
	{
		for (int i = 0; i < samplesCnt; i++)
			samples[i] = (int16_t)(((int32_t)samples[i]) * volume / 100);
			
		int inBytes = samplesCnt * sizeof(int16_t);
		uint8_t *outData;
		int outSamplesCnt;
		if (outSampleRate != srcSampleRate)
		{
			outData	= new uint8_t[inBytes * (outSampleRate + srcSampleRate - 1) / srcSampleRate];
			outSamplesCnt = resampleCH2S16LE(samplesCnt, srcSampleRate, samples, outSampleRate, outData);
		}
		else
		{
			outData = (uint8_t*)samples;
			outSamplesCnt = samplesCnt;
		}
		socket.sendData("192.168.2.2", 14141, outData, outSamplesCnt * sizeof(int16_t));
		if (outSampleRate != srcSampleRate)
		{
			delete [] outData;
		}
	}
	
	if (srcFormat == FORMAT_FLOAT)
	{
		delete [] samples;
	}
}

void controlThread()
{
	for (;;)
	{
		int c = getch();
		if (c == 43)
		{
			volume += 5;
		}
		if (c == 45)
		{
			volume -= 5;
		}
		if (volume < 0)
			volume = 0;
		if (volume > 100)
			volume = 100;
		printf("%d\r\n", volume);
	}
}

int main()
{
	initSocket();
	init();
	
	socket.init();
	
	std::thread t(controlThread);
	
	for (;;)
	{
		process();
	}
	t.join();
	
	return 0;
}
