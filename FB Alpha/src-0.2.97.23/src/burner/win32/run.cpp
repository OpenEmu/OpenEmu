// Run module
#include "burner.h"

// for NeoGeo CD (WAV playback)
void wav_pause(bool bResume);

int bRunPause = 0;
int bAltPause = 0;

int bAlwaysDrawFrames = 0;

static bool bShowFPS = false;
static unsigned int nDoFPS = 0;

static bool bMute = false;
static int nOldAudVolume;

int kNetGame = 0;							// Non-zero if Kaillera is being used

#ifdef FBA_DEBUG
int counter;								// General purpose variable used when debugging
#endif

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
		InpdUpdate();
	}

	i++;

	if (i >= 3) {
		i = 0;
	}

	// Update Input Set dialog
	InpsUpdate();
	return 0;
}

static void DisplayFPS()
{
	static time_t fpstimer;
	static unsigned int nPreviousFrames;

	TCHAR fpsstring[8];
	time_t temptime = clock();
	double fps = (double)(nFramesRendered - nPreviousFrames) * CLOCKS_PER_SEC / (temptime - fpstimer);
	_sntprintf(fpsstring, 7, _T("%2.2lf"), fps);
	VidSNewShortMsg(fpsstring, 0xDFDFFF, 480, 0);

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
	}

	if (!bDrvOkay) {
		return 1;
	}

	if (bPause) {
		GetInput(false);						// Update burner inputs, but not game inputs
		if (bPause != bPrevPause) {
			VidPaint(2);                        // Redraw the screen (to ensure mode indicators are updated)
		}
	} else {

		nFramesEmulated++;
		nCurrentFrame++;

		if (kNetGame) {
			GetInput(true);						// Update inputs
			if (KailleraGetInput()) {			// Synchronize input with Kaillera
				return 0;
			}
		} else {
			if (nReplayStatus == 2) {
				GetInput(false);				// Update burner inputs, but not game inputs
				if (ReplayInput()) {			// Read input from file
					bAltPause = 1;
					bRunPause = 1;
					MenuEnableItems();
					InputSetCooperativeLevel(false, false);
				}
			} else {
				GetInput(true);					// Update inputs
			}
		}

		if (nReplayStatus == 1) {
			RecordInput();						// Write input to file
		}

		if (bDraw) {
			nFramesRendered++;

			if (VidFrame()) {					// Do one frame
				AudBlankSound();
			}
		} else {								// frame skipping
			pBurnDraw = NULL;					// Make sure no image is drawn
			BurnDrvFrame();
		}

		if (bShowFPS) {
			if (nDoFPS < nFramesRendered) {
				DisplayFPS();
				nDoFPS = nFramesRendered + 30;
			}
		}
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

	if (WaveLog != NULL && pBurnSoundOut != NULL) {		// log to the file
		fwrite(pBurnSoundOut, 1, nBurnSoundLen << 2, WaveLog);
		pBurnSoundOut = NULL;
	}

	if (bAppDoStep) {
		memset(nAudNextSound, 0, nAudSegLen << 2);		// Write silence into the buffer
	}
	bAppDoStep = 0;										// done one step

	return 0;
}

extern bool bRunFrame;

int RunIdle()
{
	int nTime, nCount;

	if (bAudPlaying) {
		// Run with sound
		
		if(bRunFrame) {
			bRunFrame = false;
			if(bAlwaysDrawFrames) {
				RunFrame(1, 0);
			} else {
				RunGetNextSound(0);
			}
			return 0;
		} else {
			AudSoundCheck();
			return 0;
		}
	}

	// Run without sound
	nTime = timeGetTime() - nNormalLast;
	nCount = (nTime * nAppVirtualFps - nNormalFrac) / 100000;
	if (nCount <= 0) {						// No need to do anything for a bit
		//Sleep(2);
		return 0;
	}

	nNormalFrac += nCount * 100000;
	nNormalLast += nNormalFrac / nAppVirtualFps;
	nNormalFrac %= nAppVirtualFps;

	//if (bAppDoFast){						// Temporarily increase virtual fps
	//	nCount *= nFastSpeed;
	//}
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

	if (bAppDoFast) {									// do more frames
		for (int i = 0; i < nFastSpeed; i++) {
			RunFrame(0, 0);
		}
	}

	if(!bAlwaysDrawFrames) {
		for (int i = nCount / 10; i > 0; i--) {	// Mid-frames
			RunFrame(0, 0);
		}
	}
	RunFrame(1, 0);							// End-frame

	return 0;
}

int RunReset()
{
	// Reset the speed throttling code
	nNormalLast = 0; nNormalFrac = 0;
	// Reset FPS display
	nDoFPS = 0;

	if (!bAudPlaying) {
		// run without sound
		nNormalLast = timeGetTime();
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
	MSG Msg;

	do {
		bRestartVideo = 0;

		// Remove pending initialisation messages from the queue
		while (PeekMessage(&Msg, NULL, WM_APP + 0, WM_APP + 0, PM_NOREMOVE)) {
			if (Msg.message != WM_QUIT)	{
				PeekMessage(&Msg, NULL, WM_APP + 0, WM_APP + 0, PM_REMOVE);
			}
		}

		RunInit();

		ShowWindow(hScrnWnd, nAppShowCmd);												// Show the screen window
		nAppShowCmd = SW_NORMAL;

		SetForegroundWindow(hScrnWnd);

		GameInpCheckLeftAlt();
		GameInpCheckMouse();															// Hide the cursor

		if(bVidDWMCore) {
			DWM_StutterFix();
		}

		while (1) {
			if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
				// A message is waiting to be processed
				if (Msg.message == WM_QUIT)	{											// Quit program
					break;
				}
				if (Msg.message == (WM_APP + 0)) {										// Restart video
					bRestartVideo = 1;
					break;
				}

				if (bMenuEnabled && nVidFullscreen == 0) {								// Handle keyboard messages for the menu
					if (MenuHandleKeyboard(&Msg)) {
						continue;
					}
				}

				if (Msg.message == WM_SYSKEYDOWN || Msg.message == WM_KEYDOWN) {
					if (Msg.lParam & 0x20000000) {
						// An Alt/AltGr-key was pressed
						switch (Msg.wParam) {

#if defined (FBA_DEBUG)
							case 'C': {
								static int count = 0;
								if (count == 0) {
									count++;
									{ char* p = NULL; if (*p) { printf("crash...\n"); } }
								}
								break;
							}
#endif
							
              				// 'Silence' & 'Sound Restored' Code (added by CaptainCPS-X) 
							case 'S': {
								TCHAR buffer[15];
								bMute = !bMute;

								if (bMute) {
									nAudVolume = 0;// mute sound
									_stprintf(buffer, FBALoadStringEx(hAppInst, IDS_SOUND_MUTE, true), nAudVolume / 100);
								} else {
									nAudVolume = nOldAudVolume;// restore volume
									_stprintf(buffer, FBALoadStringEx(hAppInst, IDS_SOUND_MUTE_OFF, true), nAudVolume / 100);
								}
								if (AudSoundSetVolume() == 0) {
									VidSNewShortMsg(FBALoadStringEx(hAppInst, IDS_SOUND_NOVOLUME, true));
								} else {
									VidSNewShortMsg(buffer);
								}
								break;
							}
							
							case VK_OEM_PLUS: {
								if (bMute) break; // if mute, not do this
								nOldAudVolume = nAudVolume;
								TCHAR buffer[15];

								nAudVolume += 100;
								if (GetAsyncKeyState(VK_CONTROL) & 0x80000000) {
									nAudVolume += 900;
								}

								if (nAudVolume > 10000) {
									nAudVolume = 10000;
								}
								if (AudSoundSetVolume() == 0) {
									VidSNewShortMsg(FBALoadStringEx(hAppInst, IDS_SOUND_NOVOLUME, true));
								} else {
									_stprintf(buffer, FBALoadStringEx(hAppInst, IDS_SOUND_VOLUMESET, true), nAudVolume / 100);
									VidSNewShortMsg(buffer);
								}
								break;
							}
							case VK_OEM_MINUS: {
								if (bMute) break; // if mute, not do this
							  	nOldAudVolume = nAudVolume;
								TCHAR buffer[15];

								nAudVolume -= 100;
								if (GetAsyncKeyState(VK_CONTROL) & 0x80000000) {
									nAudVolume -= 900;
								}

								if (nAudVolume < 0) {
									nAudVolume = 0;
								}
								if (AudSoundSetVolume() == 0) {
									VidSNewShortMsg(FBALoadStringEx(hAppInst, IDS_SOUND_NOVOLUME, true));
								} else {
									_stprintf(buffer, FBALoadStringEx(hAppInst, IDS_SOUND_VOLUMESET, true), nAudVolume / 100);
									VidSNewShortMsg(buffer);
								}
								break;
							}
							case VK_MENU: {
								continue;
							}
						}
					} else {
						switch (Msg.wParam) {

#if defined (FBA_DEBUG)
							case 'N':
								counter--;
								bprintf(PRINT_IMPORTANT, _T("*** New counter value: %04X.\n"), counter);
								break;
							case 'M':
								counter++;
								bprintf(PRINT_IMPORTANT, _T("*** New counter value: %04X.\n"), counter);
								break;
#endif
							case VK_ESCAPE: {
								if (hwndChat) {
									DeActivateChat();
								} else {
									if (bCmdOptUsed) {
										PostQuitMessage(0);
									} else {
										if (nVidFullscreen) {
											nVidFullscreen = 0;
											POST_INITIALISE_MESSAGE;
										}
									}
								}
								break;
							}
							case VK_RETURN: {
								if (hwndChat) {
									int i = 0;
									while (EditText[i]) {
										if (EditText[i++] != 0x20) {
											break;
										}
									}
									if (i) {
										Kaillera_Chat_Send(TCHARToANSI(EditText, NULL, 0));
										//kailleraChatSend(TCHARToANSI(EditText, NULL, 0));
									}
									DeActivateChat();

									break;
								}
								if (GetAsyncKeyState(VK_CONTROL) & 0x80000000) {
									bMenuEnabled = !bMenuEnabled;
									POST_INITIALISE_MESSAGE;

									break;
								}

								break;
							}
							case VK_F1: {
								if (kNetGame) {
									break;
								}

								if (((GetAsyncKeyState(VK_CONTROL) | GetAsyncKeyState(VK_SHIFT)) & 0x80000000) == 0) {
									if (bRunPause) {
										bAppDoStep = 1;
									} else {
										bAppDoFast = 1;
									}
								}
								break;
							}

							case VK_BACK: {
								bShowFPS = !bShowFPS;
								if (bShowFPS) {
									DisplayFPS();
								} else {
									VidSKillShortMsg();
									VidSKillOSDMsg();
								}
								break;
							}
							case 'T': {
								if (kNetGame && hwndChat == NULL) {
									if (AppMessage(&Msg)) {
										ActivateChat();
									}
								}
								break;
							}
						}
					}
				} else {
					if (Msg.message == WM_SYSKEYUP || Msg.message == WM_KEYUP) {
						switch (Msg.wParam) {
							case VK_MENU:
								continue;
							case VK_F1:
								bAppDoFast = 0;
								break;
						}
					}
				}

				// Check for messages for dialogs etc.
				if (AppMessage(&Msg)) {
					if (TranslateAccelerator(hScrnWnd, hAccel, &Msg) == 0) {
						if (hwndChat) {
							TranslateMessage(&Msg);
						}
						DispatchMessage(&Msg);
					}
				}
			} else {
				
				bRunPause ? wav_pause(false) : wav_pause(true);

				// No messages are waiting
				SplashDestroy(0);
				RunIdle();
			}
		}

		RunExit();
		MediaExit();
		if (bRestartVideo) {
			MediaInit();
		}
	} while (bRestartVideo);

	return 0;
}

