#pragma once

#include "psptypes.h"

#include <SDL_rwops.h>

#define PSP_O_RDONLY 0

#define PSP_SEEK_CUR RW_SEEK_CUR
#define PSP_SEEK_SET RW_SEEK_SET
#define PSP_SEEK_END RW_SEEK_END

SceUID sceIoOpen(const char *path, int prot, int mode);
void sceIoClose(SceUID file);
SceSize sceIoRead(SceUID file, void *buffer, SceSize size);
SceSize sceIoLseek32(SceUID file, long offset, int whence);
