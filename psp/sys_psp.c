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

#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <psprtc.h>

/* Define the module info section */
PSP_MODULE_INFO("Quake 2", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

void *GetGameAPI (void *import);

int	curtime;
unsigned	sys_frame_time;

static qboolean go = true;
static u64 first_ticks;
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

	pspDebugScreenInit();
	pspDebugScreenPuts("Sys_Error:\n\n");
	pspDebugScreenSetTextColor(GU_RGBA(255, 0, 0, 255));
	pspDebugScreenPuts(buffer);
	pspDebugScreenPuts("\n");

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
#if 0
	FILE *file = NULL;

	file = fopen("log.txt", "a");
	if (file != NULL)
	{
		fputs(string, file);
		//fflush(file);
		fclose(file);
		file = NULL;
	}
#endif
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
	u64 ms;

	sceRtcGetCurrentTick(&ticks);

	ms = (ticks - first_ticks) / ticks_per_ms;

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

int main (int argc, char **argv)
{
	FILE *log = NULL;
	u32 ticks_per_s;
	int oldtime;

	SetupCallbacks();

	// Wipe log file.
	log = fopen("log.txt", "w");
	if (log != NULL)
	{
		fputs("LOG START\n\n", log);
		fclose(log);
		log = NULL;
	}

	// Calculate the clock resolution.
	sceRtcGetCurrentTick(&first_ticks);
	ticks_per_s = sceRtcGetTickResolution();
	ticks_per_ms = ticks_per_s / 1000.0;

	// TODO Move elsewhere.
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	// Initialise Quake.
	Qcommon_Init (argc, argv);

	oldtime = Sys_Milliseconds();

	// Run the main loop.
	while (go)
	{
		SceCtrlData pad;

		curtime = Sys_Milliseconds();

		if (curtime < oldtime)
		{
			Sys_Error("curtime (%d) < oldtime (%d)\n", curtime, oldtime);
		}

		sceCtrlPeekBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			go = false;
		}

		Qcommon_Frame (curtime - oldtime);

		oldtime = curtime;
	}

	sceKernelExitGame();

	return 0;
}


