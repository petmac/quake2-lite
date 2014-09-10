#include "pspkernel.h"

#include <SDL_rwops.h>

static SDL_RWops *sdl_file(SceUID file)
{
	if (file < 0)
	{
		return NULL;
	}
	else
	{
		return (SDL_RWops *)(file + 1);
	}
}

static SceUID psp_file(SDL_RWops *file)
{
	if (file == NULL)
	{
		return -1;
	}
	else
	{
		return ((SceUID)file) - 1;
	}
}

SceUID sceIoOpen(const char *path, int prot, int mode)
{
	SDL_RWops *f = SDL_RWFromFile(path, "rb");
	return psp_file(f);
}

void sceIoClose(SceUID file)
{
	SDL_RWops *f = sdl_file(file);
	SDL_RWclose(f);
}

SceSize sceIoRead(SceUID file, void *buffer, SceSize size)
{
	SDL_RWops *f = sdl_file(file);
	return SDL_RWread(f, buffer, 1, size);
}

SceSize sceIoLseek32(SceUID file, long offset, int whence)
{
	SDL_RWops *f = sdl_file(file);
	return (SceSize)SDL_RWseek(f, offset, whence);
}
