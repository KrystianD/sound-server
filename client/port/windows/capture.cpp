#include "capture.h"

#include <stdio.h>
#include <tchar.h>

#include <winsock2.h>
#include <windows.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#define snprintf  _snprintf
#define vsnprintf _vsnprintf
#define sockerrno WSAGetLastError()
#else
#include <ws2tcpip.h>
#define sockerrno errno
#endif
#define socklen_t int
#else
#include <sys/socket.h>
#include <netdb.h>
#ifdef __APPLE__
#define SOL_IP IPPROTO_IP
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#define sockerrno errno
#endif

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
	if (FAILED(hres)) { return false; }
#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
								{ (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

IAudioCaptureClient *pCaptureClient = NULL;

bool init()
{
	CoInitialize(0);
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IAudioClient *pAudioClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;

	hr = CoCreateInstance(
	       CLSID_MMDeviceEnumerator, NULL,
	       CLSCTX_ALL, IID_IMMDeviceEnumerator,
	       (void**)&pEnumerator);
	EXIT_ON_ERROR(hr);
	
	IMMDevice *pDevice = NULL;
	
	hr = pEnumerator->GetDefaultAudioEndpoint(
	       eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr);
	
	hr = pDevice->Activate(
	       IID_IAudioClient, CLSCTX_ALL,
	       NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr);
	
	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);
	
	hr = pAudioClient->Initialize(
	       AUDCLNT_SHAREMODE_SHARED,
	       AUDCLNT_STREAMFLAGS_LOOPBACK,
	       hnsRequestedDuration,
	       0,
	       pwfx,
	       NULL);
	EXIT_ON_ERROR(hr);
	
	// Get the size of the allocated buffer.
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr);
	
	hr = pAudioClient->GetService(
	       IID_IAudioCaptureClient,
	       (void**)&pCaptureClient);
	EXIT_ON_ERROR(hr);
	
	// Notify the audio sink which format to use.
	//hr = pMySink->SetFormat(pwfx);
	// printf("sa %d\n", pwfx->nSamplesPerSec);
	// printf("bps %d\n", pwfx->wBitsPerSample);
	// printf("ch %d\n", pwfx->nChannels);
	// printf("a %d\n", pwfx->nAvgBytesPerSec);
	// printf("f %x\n", pwfx->wFormatTag);
	
	int format = -1;
	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE *ex = (WAVEFORMATEXTENSIBLE*)pwfx;
		// printf("ex chma %x\n", ex->dwChannelMask);
		// printf("ex sa %d\n", ex-> Samples);
		// printf("ex sub %d f1\n", ex->  SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
		// printf("ex sub %d f2\n", ex->  SubFormat == KSDATAFORMAT_SUBTYPE_PCM);
		
		if (ex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			format = FORMAT_FLOAT;
	}
	onSetFormat(pwfx->nSamplesPerSec, format, pwfx->nChannels);
	EXIT_ON_ERROR(hr);
	
	// Calculate the actual duration of the allocated buffer.
	hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;
	
	hr = pAudioClient->Start();  // Start recording.
	EXIT_ON_ERROR(hr);
	
	return true;
}

void initSocket()
{
	WSADATA wsa;
	// printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// printf("Initialised.\n");
}

bool process()
{
	char buf[BUFLEN];
	char message[BUFLEN];
	HRESULT hr;
	unsigned int packetLength, numFramesAvailable;
	BYTE *pData;
	DWORD flags;
	
	hr = pCaptureClient->GetNextPacketSize(&packetLength);
	EXIT_ON_ERROR(hr);
	
	while (packetLength != 0)
	{
// Get the available data in the shared buffer.
		hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
		EXIT_ON_ERROR(hr);
		
		if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
		{
			pData = NULL;
		}
		
		if (pData)
		{
			onNewData(pData, numFramesAvailable);
		}
		
		EXIT_ON_ERROR(hr);
		
		hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
		EXIT_ON_ERROR(hr);
		
		hr = pCaptureClient->GetNextPacketSize(&packetLength);
		EXIT_ON_ERROR(hr);
	}
	
	// hr = pAudioClient->Stop();  // Stop recording.
	// EXIT_ON_ERROR(hr);
	
	
// Exit:
	// CoTaskMemFree(pwfx);
	// SAFE_RELEASE(pEnumerator)
	// SAFE_RELEASE(pDevice)
	// SAFE_RELEASE(pAudioClient)
	// SAFE_RELEASE(pCaptureClient)
	
	// return hr;
}

// int _tmain(int argc, _TCHAR* argv[])
// {
// RecordAudioStream();
// system("pause");
// return 0;
// }

