#pragma once

#include "psptypes.h"

typedef void PSP_AUDIO_CALLBACK(void *buf, unsigned int reqn, void *pdata);

int pspAudioInit(void);
void pspAudioEnd(void);
void pspAudioSetChannelCallback(int a, PSP_AUDIO_CALLBACK *callback, void *param);
