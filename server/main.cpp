#include <stdio.h>
#include <vector>
#include <list>
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <cmath>
#include <pulse/simple.h>
#include <pulse/error.h>

#include "UdpSocket.h"
#include "pid.h"
#include "kdutils.h"

using namespace std;

uint8_t *data;
int capacity = 1024 * 128;
int txPtr = 0, rxPtr = 0;
int dataLen = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
pa_simple *s_out = 0;

// const int ratio = 200;
const int sampleRate = 44100;
// const int toPlayFrames = sampleRate / ratio;
const int bps = 2;
const int channels = 2;

class Listener : public IEthernetDataListener
{
public:
	void onEthernetDataReceived(const string& ip, const void* buffer, int len)
	{
		pthread_mutex_lock(&mutex);
		for (int i = 0; i < len; i++)
		{
			data[txPtr++] = ((uint8_t*)buffer)[i];
			txPtr &= capacity - 1;
		}
		dataLen += len;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
	}
};

void* receiverThread(void*)
{
	Listener listener;
	UdpSocket server;
	
	server.setPort(14141);
	server.init();
	server.bind();
	server.setListener(&listener);
	
	for (;;)
		server.process();
}
void* playerThread(void*)
{
	int error;
	
	// int toPlayBytes = toPlayFrames * bps * channels;
	// uint8_t toPlayBuffer[toPlayBytes];
	uint8_t toPlayBuffer[1024 * 1024];
	
	for (;;)
	{
		pthread_mutex_lock(&mutex);
		
		while (dataLen == 0)
			pthread_cond_wait(&cond, &mutex);
			
		// pthread_mutex_unlock(&mutex);
		
		int framesAvailable = dataLen / bps / channels;
		
		int toFetchFrames = framesAvailable;
		int toFetchBytes = toFetchFrames * bps * channels;
		
		// printf("f %d\n", toFetchFrames);
		float mult = 1;
		if (framesAvailable > 2000)
		{
			mult = 1;
			dataLen = 0;
			rxPtr = txPtr;
			pthread_mutex_unlock(&mutex);
			continue;
			// printf("q\r\n");
		}
		else
			mult = 1;
			
		// if(toFetchFrames>400)
		// {
		// printf("a\r\n");
		// }
		// toFetchFrames = min(400, toFetchFrames);
		
		// int toFetchFrames = framesAvailable * mult;
		
		int toPlayFrames = toFetchFrames / mult;
		if (toPlayFrames != toFetchFrames)
			printf("dif %d %d\r\n", toPlayFrames, toFetchFrames);
		else
			printf("%d\r\n", toFetchFrames);
		int toPlayBytes = toPlayFrames * bps * channels;
		
		// pthread_mutex_lock(&mutex);
		
		// while (dataLen < toFetchBytes)
		// pthread_cond_wait(&cond, &mutex);
		
		if (dataLen > 200000)
		{
			printf("skip\n");
			dataLen = toFetchFrames;
		}
		
		dataLen -= toFetchBytes;
		
		uint8_t d[toFetchBytes];
		int pos = 0;
		
		while (pos < toFetchBytes)
		{
			d[pos++] = data[rxPtr++];
			rxPtr &= capacity - 1;
		}
		
		pthread_mutex_unlock(&mutex);
		
		int16_t* ptr = (int16_t*)d;
		int16_t* ptrDest = (int16_t*)toPlayBuffer;
		for (int i = 0; i < toPlayFrames; i++)
		{
			float idx = (float)i / toPlayFrames * toFetchFrames;
			int s1 = floor(idx);
			int s2 = ceil(idx);
			float r = idx - floor(idx);
			for (int ch = 0; ch < channels; ch++)
				ptrDest[i * channels + ch] = ptr[s1 * channels + ch] * (1 - r) + ptr[s2 * channels + ch] * r;
		}
		
		if (pa_simple_write(s_out, toPlayBuffer, toPlayBytes, &error) < 0)
			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
	}
}

int main(int argc, char** argv)
{
	pa_sample_spec ss;
	int error;
	
	ss.format = PA_SAMPLE_S16LE;
	ss.rate = sampleRate;
	ss.channels = channels;

	const char *dev = 0;
	if (argc == 2)
		dev = argv[1];
	printf("using %s\r\n", dev);
	
	if (!(s_out = pa_simple_new(NULL, "default", PA_STREAM_PLAYBACK,
	                            dev, "playback", &ss, NULL, NULL,  &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		return 1;
	}
	
	pthread_t thread1;
	pthread_t thread2;
	data = new uint8_t[capacity];
	
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);
	pthread_create(&thread1, 0, &receiverThread, 0);
	pthread_create(&thread2, 0, &playerThread, 0);
	
	for (;;)
	{
		int framesAvailable = dataLen / bps / channels;
		printf("frames %6d\r\n", framesAvailable);
		usleep(500000);
	}
	
	return 0;
}
