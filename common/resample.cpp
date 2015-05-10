/*
 * resample.cpp
 * Copyright (C) 2015 Krystian Dużyński <krystian.duzynski@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "resample.h"

#include <stdio.h>
#include <cmath>

int resampleCH2S16LE(int inSamples, int inSampleRate, const void* inData, int outSampleRate, void* outData)
{
	int outSamples = inSamples * outSampleRate / inSampleRate;
	// printf("o %d %d\n", inSamples, outSamples);
	
	int16_t* ptr = (int16_t*)inData;
	int16_t* ptrDest = (int16_t*)outData;
	for (int i = 0; i < outSamples; i++)
	{
		float idx = (float)i / outSamples * inSamples;
		int s1 = floor(idx);
		int s2 = ceil(idx);
		float r = idx - floor(idx);
		ptrDest[i * 2 + 0] = ptr[s1 * 2 + 0] * (1 - r) + ptr[s2 * 2 + 0] * r;
		ptrDest[i * 2 + 1] = ptr[s1 * 2 + 1] * (1 - r) + ptr[s2 * 2 + 1] * r;
	}

	return outSamples;
}
