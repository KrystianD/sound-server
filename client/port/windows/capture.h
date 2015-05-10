/*
 * Copyright (C) 2015 Krystian Dużyński <krystian.duzynski@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#define FORMAT_FLOAT 0

extern void onSetFormat(int sampleRate, int format, int channels);
extern void onNewData(const void* data, int framesCnt);

bool init();
void initSocket();
bool process();

#endif
