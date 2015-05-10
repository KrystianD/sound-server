/*
 * Copyright (C) 2015 Krystian Dużyński <krystian.duzynski@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __RESAMPLE_H__
#define __RESAMPLE_H__

#include <stdint.h>

int resampleCH2S16LE(int inSamples, int inSampleRate, const void* inData, int outSampleRate, void* outData);

#endif
