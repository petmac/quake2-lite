#pragma once

#include "pspgu.h"

void sceGumMatrixMode(GU_MATRIX_MODE mode);
void sceGumLoadIdentity(void);
void sceGumPushMatrix(void);
void sceGumPopMatrix(void);

void sceGumTranslate(const ScePspFVector3 *v);
void sceGumRotateX(float rad);
void sceGumRotateY(float rad);
void sceGumRotateZ(float rad);

void sceGumOrtho(float left, float right, float bottom, float top, float n, float f);
void sceGumPerspective(float fov, float aspect, float n, float f);

void sceGumDrawArray(int prim, int type, int numverts, const void *indices, const void *vertices);
