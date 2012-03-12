/*----------------
Stuff to finish:
redo config stuff + add support for per game config files
decide how to add the sound code via the sound interface
add config options for video and sound
add dat.cpp and sshot.cpp


------------------*/
#include <psppower.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <math.h>
#include "burner.h"


int nAppVirtualFps = 6000;			// App fps * 100
bool bRunPause=0;
bool bAlwaysProcessKeyboardInput=0;


/* Define the module info section */
PSP_MODULE_INFO("fbapsp", 0, 1, 0); 
PSP_HEAP_SIZE_MAX(); 


#ifdef logging_build

static FILE *fp;


void log_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(fp, format, ap);
    va_end(ap);
//	fflush(fp);

}

#endif

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	//need to cleanup here
#ifdef logging_build	
	fflush(fp);
	fclose(fp);
#endif
	scePowerSetClockFrequency(222,222,111);
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


void init_emu(unsigned int gamenum)
{
 	bCheatsAllowed=false;
//	ConfigAppLoad();
//	ConfigAppSave();
	//CreateDirectory(".:\\cfg", NULL);
	//CreateDirectory(".:\\state", NULL);
	DrvInit(gamenum,0);
 }



void fbamain()
{
	unsigned int i=0;
	int menureturn=0;

#ifdef logging_build
	fp = fopen("fbalog.txt", "a");
#endif
	scePowerSetClockFrequency(333,333,166);

	BurnLibInit(); 
	// samsho 317
	// ffight 131
	// 2020bb 14
	nBurnDrvSelect = 131;  // game to start

	InputInit();
	init_emu(nBurnDrvSelect);
	
	RunMessageLoop();

	DrvExit();
	InputExit();

	ConfigAppSave();
	BurnLibExit();

}

int user_main(SceSize args, void *argp)
{
	SetupCallbacks();

	scePowerSetClockFrequency(333,333,166);

	sceCtrlSetSamplingCycle( 0 );	
	sceCtrlSetSamplingMode( 1 );

	fbamain();
	
	scePowerSetClockFrequency(222,222,111);
	sceKernelExitGame();

	return 0;
}

int main(int argc, char* argv[])	
{

	// create user thread, tweek stack size here if necessary
	SceUID thid = sceKernelCreateThread("User Mode Thread", user_main, 0x11,256 * 1024, PSP_THREAD_ATTR_USER, NULL);
	// start user thread, then wait for it to do everything else
	sceKernelStartThread(thid, 0, 0);
	sceKernelWaitThreadEnd(thid, NULL);
	return 0;
}



/* const */ TCHAR* ANSIToTCHAR(const char* pszInString, TCHAR* pszOutString, int nOutSize)
{
#if defined (UNICODE)
	static TCHAR szStringBuffer[1024];

	TCHAR* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize  = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (MultiByteToWideChar(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize)) {
		return pszBuffer;
	}

	return NULL;
#else
	if (pszOutString) {
		_tcscpy(pszOutString, pszInString);
		return pszOutString;
	}

	return (TCHAR*)pszInString;
#endif
}


/* const */ char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize)
{
#if defined (UNICODE)
	static char szStringBuffer[1024];
	memset(szStringBuffer, 0, sizeof(szStringBuffer));

	char* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (WideCharToMultiByte(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize, NULL, NULL)) {
		return pszBuffer;
	}

	return NULL;
#else
	if (pszOutString) {
		strcpy(pszOutString, pszInString);
		return pszOutString;
	}

	return (char*)pszInString;
#endif
}


bool AppProcessKeyboardInput()
{
	return true;
}
