#include "pspgum.h"

#include <math.h>

static const float pi = 3.1415926535897932384626433832795f;

static GU_MATRIX_MODE matrix_mode = GU_MODEL;

static void MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * pi / 360.0);
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void sceGumMatrixMode(GU_MATRIX_MODE mode)
{
	matrix_mode = mode;

	switch (mode)
	{
	case GU_VIEW:
		glMatrixMode(GL_MODELVIEW);
		break;
	case GU_PROJECTION:
		glMatrixMode(GL_PROJECTION);
		break;
	case GU_TEXTURE:
		glMatrixMode(GL_TEXTURE);
		break;
	}
}

void sceGumLoadIdentity(void)
{
	if (matrix_mode != GU_MODEL)
	{
		glLoadIdentity();
	}
}

void sceGumPushMatrix(void)
{
	if (matrix_mode != GU_MODEL)
	{
		glPushMatrix();
	}
}

void sceGumPopMatrix(void)
{
	if (matrix_mode != GU_MODEL)
	{
		glPopMatrix();
	}
}

void sceGumTranslate(const ScePspFVector3 *v)
{
	if (matrix_mode != GU_MODEL)
	{
		glTranslatef(v->x, v->y, v->z);
	}
}

void sceGumRotateX(float rad)
{
	const float deg = (rad * 180) / pi;

	if (matrix_mode != GU_MODEL)
	{
		glRotatef(deg, 1, 0, 0);
	}
}

void sceGumRotateY(float rad)
{
	const float deg = (rad * 180) / pi;

	if (matrix_mode != GU_MODEL)
	{
		glRotatef(deg, 0, 1, 0);
	}
}

void sceGumRotateZ(float rad)
{
	const float deg = (rad * 180) / pi;

	if (matrix_mode != GU_MODEL)
	{
		glRotatef(deg, 0, 0, 1);
	}
}

void sceGumOrtho(float left, float right, float bottom, float top, float n, float f)
{
	if (matrix_mode != GU_MODEL)
	{
		glOrtho(left, right, bottom, top, n, f);
	}
}

void sceGumPerspective(float fov, float aspect, float n, float f)
{
	if (matrix_mode != GU_MODEL)
	{
		MYgluPerspective(fov, aspect, n, f);
	}
}

void sceGumDrawArray(int prim, int type, int numverts, const void *indices, const void *vertices)
{
	sceGuDrawArray(prim, type, numverts, indices, vertices);
}
