#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 SceSize;
typedef int SceUID;

typedef u16 ScePspRGB565;
typedef u32 ScePspRGBA8888;

typedef struct ScePspFVector3
{
	float x, y, z;
} ScePspFVector3;

#define __attribute__(x)
