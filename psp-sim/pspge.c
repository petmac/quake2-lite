#include "pspge.h"

#include <stdlib.h>

static char vram[2 * 1024 * 1024];

void *sceGeEdramGetAddr(void)
{
	return vram;
}
