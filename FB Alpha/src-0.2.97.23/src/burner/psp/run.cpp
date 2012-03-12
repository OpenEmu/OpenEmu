// Run module
#include "burner.h"
#include "pg.h"
//#include "app.h"
//#include "kailleraclient.h"


//stolen from psp sdl
#include <time.h>
#include <sys/time.h>

static struct timeval start;

void PspStartTicks(void)
{
	gettimeofday(&start, NULL);
}

unsigned int PspGetTicks(void)
{
	struct timeval now;
	unsigned int ticks;

	gettimeofday(&now, NULL);
	ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
	return(ticks);
}

void PspDelay(unsigned int ms)
{
	const unsigned int max_delay = 0xffffffffUL / 1000;
	if(ms > max_delay)
		ms = max_delay;
	sceKernelDelayThreadCB(ms * 1000);
}

//end of stolen. :)


bool bAltPause = 0;

int bAlwaysDrawFrames = 0;

static bool bShowFPS = false;

//int kNetGame = 0;							// Non-zero if Kaillera is being used

int counter;								// General purpose variable used when debugging

static unsigned int nNormalLast = 0;		// Last value of timeGetTime()
static int nNormalFrac = 0;					// Extra fraction we did

static bool bAppDoStep = 0;
static bool bAppDoFast = 0;
static int nFastSpeed = 6;

static int GetInput(bool bCopy)
{
	static int i = 0;
	InputMake(bCopy); 						// get input

	// Update Input dialog ever 3 frames
	if (i == 0) {
		//InpdUpdate();
	}

	i++;

	if (i >= 3) {
		i = 0;
	}

	// Update Input Set dialog
	//InpsUpdate();
	return 0;
}

static void DisplayFPS()
{
	static time_t fpstimer;
	static unsigned int nPreviousFrames;

	char fpsstring[8];
	time_t temptime = clock();
	float fps = static_cast<float>(nFramesRendered - nPreviousFrames) * CLOCKS_PER_SEC / (temptime - fpstimer);
	sprintf(fpsstring, "%2.1f", fps);
	//VidSNewShortMsg(fpsstring, 0xDFDFFF, 480, 0);
	pgPrint(1,1,0xFFFFFF,fpsstring,0);

	fpstimer = temptime;
	nPreviousFrames = nFramesRendered;
}

// define this function somewhere above RunMessageLoop()
void ToggleLayer(unsigned char thisLayer)
{
	nBurnLayer ^= thisLayer;				// xor with thisLayer
	VidRedraw();
	VidPaint(0);
}



// With or without sound, run one frame.
// If bDraw is true, it's the last frame before we are up to date, and so we should draw the screen
static int RunFrame(int bDraw, int bPause)
{
	static int bPrevPause = 0;
	static int bPrevDraw = 0;

	if (bPrevDraw && !bPause) {
		VidPaint(0);							// paint the screen (no need to validate)
		DisplayFPS();
	}

	if (!bDrvOkay) {
		return 1;
	}

	if (bPause) 
	{
		GetInput(false);						// Update burner inputs, but not game inputs
		if (bPause != bPrevPause) 
		{
			VidPaint(2);                        // Redraw the screen (to ensure mode indicators are updated)
		}
	} 
	else 
	{
		nFramesEmulated++;
		nCurrentFrame++;

	/*	{
		if (nReplayStatus == 2) {
			GetInput(false);				// Update burner inputs, but not game inputs
			if (ReplayInput()) {			// Read input from file
				bAltPause = 1;
				bRunPause = 1;
				MenuEnableItems();
				InputSetCooperativeLevel(false, false);
			}
		} 
		else 
	*/	{
			GetInput(true);					// Update inputs
		}
	}
/*	if (nReplayStatus == 1) {
		RecordInput();						// Write input to file
	}
*/	if (bDraw) {
		nFramesRendered++;
		if (VidFrame()) {					// Do one frame
			AudBlankSound();
			
		}
	} 
	else {								// frame skipping
		pBurnDraw = NULL;					// Make sure no image is drawn
		BurnDrvFrame();
	}
	bPrevPause = bPause;
	bPrevDraw = bDraw;

	return 0;
}


// Callback used when DSound needs more sound
static int RunGetNextSound(int bDraw)
{
	if (nAudNextSound == NULL) {
		return 1;
	}

	if (bRunPause) {
		if (bAppDoStep) {
			RunFrame(bDraw, 0);
			memset(nAudNextSound, 0, nAudSegLen << 2);	// Write silence into the buffer
		} else {
			RunFrame(bDraw, 1);
		}

		bAppDoStep = 0;									// done one step
		return 0;
	}

	if (bAppDoFast) {									// do more frames
		for (int i = 0; i < nFastSpeed; i++) {
			RunFrame(0, 0);
		}
	}

	// Render frame with sound
	pBurnSoundOut = nAudNextSound;
	RunFrame(bDraw, 0);
/*
	if (WaveLog != NULL && pBurnSoundOut != NULL) {		// log to the file
		fwrite(pBurnSoundOut, 1, nBurnSoundLen << 2, WaveLog);
		pBurnSoundOut = NULL;
	}
*/
	if (bAppDoStep) {
		memset(nAudNextSound, 0, nAudSegLen << 2);		// Write silence into the buffer
	}
	bAppDoStep = 0;										// done one step

	return 0;
}

int RunIdle()
{
	int nTime, nCount;

	if (bAudPlaying) {
		// Run with sound
		AudSoundCheck();
		return 0;
	}

	// Run without sound
	nTime = PspGetTicks() - nNormalLast;
	nCount = (nTime * nAppVirtualFps - nNormalFrac) / 100000;
	if (nCount <= 0) {						// No need to do anything for a bit
		PspDelay(2);

		return 0;
	}

	nNormalFrac += nCount * 100000;
	nNormalLast += nNormalFrac / nAppVirtualFps;
	nNormalFrac %= nAppVirtualFps;

	if (bAppDoFast){						// Temporarily increase virtual fps
		nCount *= nFastSpeed;
	}
	if (nCount > 100) {						// Limit frame skipping
		nCount = 100;
	}
	if (bRunPause) {
		if (bAppDoStep) {					// Step one frame
			nCount = 10;
		} else {
			RunFrame(1, 1);					// Paused
			return 0;
		}
	}
	bAppDoStep = 0;

	for (int i = nCount / 10; i > 0; i--) {	// Mid-frames
		RunFrame(!bAlwaysDrawFrames, 0);
	}
	RunFrame(1, 0);							// End-frame
	// temp added for SDLFBA
	//VidPaint(0);
	return 0;
}

int RunReset()
{
	// Reset the speed throttling code
	nNormalLast = 0; nNormalFrac = 0;
	if (!bAudPlaying) {
		// run without sound
		nNormalLast = PspGetTicks();
	}
	return 0;
}

static int RunInit()
{
	// Try to run with sound
	AudSetCallback(RunGetNextSound);
	AudSoundPlay();

	RunReset();

	return 0;
}

static int RunExit()
{
	nNormalLast = 0;
	// Stop sound if it was playing
	AudSoundStop();
	return 0;
}

// The main message loop
int RunMessageLoop()
{
	int bRestartVideo;
	int finished= 0;
	do {
		bRestartVideo = 0;

		//MediaInit();

		if (!bVidOkay) {

			// Reinit the video plugin
			VidInit();
			if (!bVidOkay && nVidFullscreen) {

				nVidFullscreen = 0;

		//		MediaExit(bRestartVideo);
		//		MediaInit();
				VidInit();
			}
			if (!nVidFullscreen) {
				//ScrnSize();
			}

		/*	if (!bVidOkay && (bDrvOkay || bVidUsePlaceholder)) {
				// Maske sure the error will be visible
				SplashDestroy(1);

				AppError("VidInit Failed", 0);
			}
*/
/*			if (bVidOkay && (bRunPause || !bDrvOkay)) {
				VidRedraw();
			}
*/		}

		RunInit();

	//	ShowWindow(hScrnWnd, nAppShowCmd);												// Show the screen window
	//	nAppShowCmd = SW_NORMAL;

	//	SetForegroundWindow(hScrnWnd);

		//GameInpCheckLeftAlt();
		GameInpCheckMouse();															// Hide the cursor
		while (!finished) {
				RunIdle();
		}
		RunExit();
	} while (bRestartVideo);

	return 0;
}

