/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../qcommon/qcommon.h"
#include "../client/keys.h"

#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <psprtc.h>

/* Define the module info section */
PSP_MODULE_INFO("Quake 2", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf
#define puts pspDebugScreenPuts

void *GetGameAPI (void *import);

int	curtime;
unsigned	sys_frame_time;

static qboolean go = true;
static double ticks_per_ms;

/* Exit callback */
static int exit_callback(void)
{
	go = false;

	return 0;
}

/* Callback thread */
static int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void *) exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
static int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

void Sys_Error (char *error, ...)
{
	char buffer[256];
	int i;

	va_list		argptr;

	va_start (argptr, error);
	vsnprintf (buffer, 255, error, argptr);
	buffer[255] = '\0';
	va_end (argptr);

	pspDebugScreenSetTextColor(GU_RGBA(255, 0, 0, 255));
	puts(buffer);
	puts("\n");

	while (go)
	{
		sceDisplayWaitVblankStart();
	}
	
	sceKernelExitGame();
}

void Sys_Quit (void)
{
	sceKernelExitGame();
}

void	Sys_UnloadGame (void)
{
}

void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI(parms);
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
	puts(string);
}

void Sys_SendKeyEvents (void)
{
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData( void )
{
	return NULL;
}

int		Sys_Milliseconds (void)
{
	u64 ticks = 0;
	int ms;

	sceRtcGetCurrentTick(&ticks);

	ms = ticks / ticks_per_ms;

	return ms;
}

void	Sys_Mkdir (char *path)
{
}

char	*Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
	return NULL;
}

char	*Sys_FindNext (unsigned musthave, unsigned canthave)
{
	return NULL;
}

void	Sys_FindClose (void)
{
}

void	Sys_Init (void)
{
}


//=============================================================================

#if 0
static void psp_debug_error_handler(PspDebugRegBlock *regs)
{
	Sys_Error("Error handler invoked.\n");
}
#endif

int main (int argc, char **argv)
{
	u32 ticks_per_s;
	int oldtime;
	int curtime;

	pspDebugScreenInit();
	printf("Debug screen initialised.\n");

	SetupCallbacks();
	printf("Callbacks set up.\n");

#if 0
	// Install debug handler.
	pspDebugInstallErrorHandler(&psp_debug_error_handler);
#endif

	// Calculate the clock resolution.
	ticks_per_s = sceRtcGetTickResolution();
	ticks_per_ms = ticks_per_s / 1000.0;
	printf("%u ticks per second.\n", ticks_per_s);
	printf("%f ticks per ms.\n", ticks_per_ms);

	Qcommon_Init (argc, argv);
	printf("Game initialised.\n");

	oldtime = Sys_Milliseconds();

	while (go)
	{
		curtime = Sys_Milliseconds();

		sceDisplayWaitVblankStart();
		Qcommon_Frame (curtime - oldtime);

		oldtime = curtime;
	}

	printf("Exiting.\n");
	sceKernelExitGame();

	return 0;
}


