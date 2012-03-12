// Burner Shots Factory Test Dialog Rev. 0.02
// history:
// rev 0.00: hacked together code
// rev 0.01: Added sprite toggle.
// rev 0.02: now uses VidRedraw()
//
// Known Problems:
// Swiching anything causes the frame to advance
//
// "ghosting" that appears on screen is not captured (this is a good thing)

#include "burner.h"

static HWND hSFactdlg=NULL;
unsigned char layerBackup;

static bool bShotsFactory;
static bool bOldPause;

void ToggleSprite(unsigned char PriNum)
{
	nSpriteEnable^=PriNum; // xor with thisLayer
	VidRedraw();
	VidPaint(0);
}

static int SFactdUpdate()
{

	CheckDlgButton(hSFactdlg,IDC_LAYER1,(nBurnLayer & 1)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER2,(nBurnLayer & 2)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER3,(nBurnLayer & 4)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER4,(nBurnLayer & 8)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE1,(nSpriteEnable & 0x01)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE2,(nSpriteEnable & 0x02)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE3,(nSpriteEnable & 0x04)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE4,(nSpriteEnable & 0x08)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE5,(nSpriteEnable & 0x10)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE6,(nSpriteEnable & 0x20)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE7,(nSpriteEnable & 0x40)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE8,(nSpriteEnable & 0x80)?BST_CHECKED:BST_UNCHECKED);
	return 0;
}

static int SFactdInit()
{
	layerBackup=nBurnLayer;
	bRunPause=1;
	CheckDlgButton(hSFactdlg,IDC_LAYER1,(nBurnLayer & 1)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER2,(nBurnLayer & 2)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER3,(nBurnLayer & 4)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_LAYER4,(nBurnLayer & 8)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE1,(nSpriteEnable & 0x01)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE2,(nSpriteEnable & 0x02)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE3,(nSpriteEnable & 0x04)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE4,(nSpriteEnable & 0x08)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE5,(nSpriteEnable & 0x10)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE6,(nSpriteEnable & 0x20)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE7,(nSpriteEnable & 0x40)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hSFactdlg,IDC_SPRITE8,(nSpriteEnable & 0x80)?BST_CHECKED:BST_UNCHECKED);
	return 0;
}


static int SFactdExit()
{
	nBurnLayer=layerBackup;
	nSpriteEnable= 0xFF;
	hSFactdlg=NULL;
	bShotsFactory = 0;
	bRunPause=bOldPause;
	GameInpCheckMouse();
	return 0;
}


static INT_PTR CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  (void)lParam; (void)wParam; (void)hDlg;

  if (Msg==WM_INITDIALOG)
  {
	  hSFactdlg=hDlg;
	  SFactdInit();
	  SFactdUpdate();
	  WndInMid(hDlg, hScrnWnd);
      SetFocus(hDlg); // Enable Esc=close
	  return 0;
  }
  if (Msg==WM_CLOSE)
  {
	  //DestroyWindow(hSFactdlg);
	  EndDialog(hDlg, 0);
	  return 0;
  }
  if (Msg==WM_DESTROY) { SFactdExit(); return 0; }

  if (Msg==WM_COMMAND)
  {
    int Id=LOWORD(wParam); int Notify=HIWORD(wParam);
    if (Id==IDOK && Notify==BN_CLICKED) { SendMessage(hDlg,WM_CLOSE,0,0); return 0; }  // cancel=close
    if (Id==IDCAPTURE && Notify==BN_CLICKED)
		{
		if (bDrvOkay)
			{
			MakeScreenShot();
			SFactdUpdate();
			}
		}
    if (Id==IDC_LAYER1 && Notify==BN_CLICKED)
		{
		ToggleLayer(1);
		SFactdUpdate();
		}
    if (Id==IDC_LAYER2 && Notify==BN_CLICKED)
		{
		ToggleLayer(2);
		SFactdUpdate();
		}
    if (Id==IDC_LAYER3 && Notify==BN_CLICKED)
		{
		ToggleLayer(4);
		SFactdUpdate();
		}
    if (Id==IDC_LAYER4 && Notify==BN_CLICKED)
		{
		ToggleLayer(8);
		SFactdUpdate();
		}
    if (Id==IDC_SPRITE1 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x01);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE2 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x02);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE3 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x04);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE4 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x08);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE5 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x10);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE6 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x20);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE7 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x40);
		SFactdUpdate();
	}
    if (Id==IDC_SPRITE8 && Notify==BN_CLICKED)
		{
		ToggleSprite(0x80);
		SFactdUpdate();
	}

	if (Id==IDC_ADVANCE && Notify==BN_CLICKED)
		{
		VidFrame();
		VidPaint(0);
		}
		if (bRunPause) {
			memset(nAudNextSound, 0, nAudSegLen << 2);		// Write silence into the buffer
		}
  }
  return 0;
}



int SFactdCreate()
{
	if (bShotsFactory) {
		return 1;
	}
	bShotsFactory = 1;
	bOldPause = bRunPause;
	bRunPause = 1;
	AudBlankSound();
//	hSFactdlg=FBACreateDialog(hAppInst,MAKEINTRESOURCE(IDD_CAPTURE),hScrnWnd,DialogProc);
//	if (hSFactdlg==NULL) return 1;
//	WndInMid(hSFactdlg,hScrnWnd);
//	ShowWindow(hSFactdlg,SW_NORMAL);
//	SFactdUpdate();
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_CAPTURE), hScrnWnd, (DLGPROC)DialogProc);
	return 0;
}
