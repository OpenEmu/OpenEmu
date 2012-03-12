// Burner Game Control
#include "burner.h"

static char szPlay[4][4]={"p1 ", "p2 ", "p3 ", "p4 "};

#define KEY(x) { pgi->nInput = GIT_SWITCH; pgi->Input.Switch.nCode = (UINT16)(x); }
#define MACRO(x) { pgi->Macro.nMode = 1; pgi->Macro.Switch.nCode = (UINT16)(x); }

// Configure the misc game controls
INT32 GamcMisc(struct GameInp* pgi, char* szi, INT32 nPlayer)
{
	switch (nPlayer) {
		case 0:
			// Set general controls according to Player 1 settings
			if (strcmp(szi, "diag") == 0) {
				KEY(FBK_F2);
				return 0;
			}
			if (strcmp(szi, "reset") == 0) {
				KEY(FBK_F3);
				return 0;
			}
			if (strcmp(szi, "service" ) == 0) {
				KEY(FBK_9);
				return 0;
			}
			if (strcmp(szi, "service2" ) == 0) {
				KEY(FBK_0);
				return 0;
			}
			if (strcmp(szi, "service3" ) == 0) {
				KEY(FBK_MINUS);
				return 0;
			}
			if (strcmp(szi, "service4" ) == 0) {
				KEY(FBK_EQUALS);
				return 0;
			}
			if (strcmp(szi, "tilt") == 0) {
				KEY(FBK_T);
				return 0;
			}
			
			if (strcmp(szi, "op menu" ) == 0) {
				KEY(FBK_F1);
				return 0;
			}
			
			if (strcmp(szi, "clear credit" ) == 0) {
				KEY(FBK_G);
				return 0;
			}
			
			if (strcmp(szi, "hopper" ) == 0) {
				KEY(FBK_H);
				return 0;
			}
			
			// Mahjong controls
			if (strcmp(szi, "mah a") == 0) {
				KEY(FBK_A);
				return 0;
			}
			
			if (strcmp(szi, "mah b") == 0) {
				KEY(FBK_B);
				return 0;
			}
			
			if (strcmp(szi, "mah c") == 0) {
				KEY(FBK_C);
				return 0;
			}
			
			if (strcmp(szi, "mah d") == 0) {
				KEY(FBK_D);
				return 0;
			}
			
			if (strcmp(szi, "mah e") == 0) {
				KEY(FBK_E);
				return 0;
			}
			
			if (strcmp(szi, "mah f") == 0) {
				KEY(FBK_F);
				return 0;
			}
			
			if (strcmp(szi, "mah g") == 0) {
				KEY(FBK_G);
				return 0;
			}
			
			if (strcmp(szi, "mah h") == 0) {
				KEY(FBK_H);
				return 0;
			}
			
			if (strcmp(szi, "mah i") == 0) {
				KEY(FBK_I);
				return 0;
			}
			
			if (strcmp(szi, "mah j") == 0) {
				KEY(FBK_J);
				return 0;
			}
			
			if (strcmp(szi, "mah k") == 0) {
				KEY(FBK_K);
				return 0;
			}
			
			if (strcmp(szi, "mah l") == 0) {
				KEY(FBK_L);
				return 0;
			}
			
			if (strcmp(szi, "mah m") == 0) {
				KEY(FBK_M);
				return 0;
			}
			
			if (strcmp(szi, "mah n") == 0) {
				KEY(FBK_N);
				return 0;
			}
			
			if (strcmp(szi, "mah kan") == 0) {
				KEY(FBK_LCONTROL);
				return 0;
			}
			
			if (strcmp(szi, "mah pon") == 0) {
				KEY(FBK_LALT);
				return 0;
			}
			
			if (strcmp(szi, "mah chi") == 0) {
				KEY(FBK_SPACE);
				return 0;
			}
			
			if (strcmp(szi, "mah reach") == 0) {
				KEY(FBK_LSHIFT);
				return 0;
			}
			
			if (strcmp(szi, "mah ron") == 0) {
				KEY(FBK_Z);
				return 0;
			}
			
			if (strcmp(szi, "mah ff") == 0) {
				KEY(FBK_Y);
				return 0;
			}
			
			if (strcmp(szi, "mah lc") == 0) {
				KEY(FBK_RALT);
				return 0;
			}
			
			if (strcmp(szi, "mah bet") == 0) {
				KEY(FBK_2);
				return 0;
			}
			
			if (strcmp(szi, "mah score") == 0) {
				KEY(FBK_RCONTROL);
				return 0;
			}

			// Player 1 controls
			if (strcmp(szi, "p1 start") == 0) {
				KEY(FBK_1);
				return 0;
			}
			if (strcmp(szi, "p1 select" ) == 0) {
				KEY(FBK_3);
				return 0;
			}
			if (strcmp(szi, "p1 coin" ) == 0) {
				KEY(FBK_5);
				return 0;
			}
			break;
		case 1:
			if (strcmp(szi, "p2 start") == 0) {
				KEY(FBK_2);
				return 0;
			}
			if (strcmp(szi, "p2 select" ) == 0) {
				KEY(FBK_4);
				return 0;
			}
			if (strcmp(szi, "p2 coin" ) == 0) {
				KEY(FBK_6);
				return 0;
			}
			break;
		case 2:
			if (strcmp(szi, "p3 coin" ) == 0) {
				KEY(FBK_7);
				return 0;
			}
			if (strcmp(szi, "p3 start") == 0) {
				KEY(FBK_3);
				return 0;
			}
			break;
		case 3:
			if (strcmp(szi, "p4 start") == 0) {
				KEY(FBK_4);
				return 0;
			}
			if (strcmp(szi, "p4 coin" ) == 0) {
				KEY(FBK_8);
				return 0;
			}
			break;
	}

	if (strncmp(szi, "mouse", 5) == 0) {

		if (strcmp(szi, "mouse x-axis") == 0) {
			pgi->nInput = GIT_MOUSEAXIS;
			pgi->Input.MouseAxis.nAxis = 0;
			return 0;

		}
		if (strcmp(szi, "mouse y-axis") == 0) {
			pgi->nInput = GIT_MOUSEAXIS;
			pgi->Input.MouseAxis.nAxis = 1;
			return 0;
		}
		if (strncmp(szi, "mouse button", 12) == 0) {
			char* szb = szi + 13;
			INT32 nButton = strtol(szb, NULL, 0);
			if (nButton >= 1) {
				nButton--;
			}
			KEY(0x8080 + nButton);
			return 0;
		}
	}

	return 0;
}

static void SetSliderKey(struct GameInp* pgi, INT32 k0, INT32 k1, INT32 nSlide)
{
	pgi->nInput = GIT_KEYSLIDER;
	pgi->Input.Slider.SliderAxis.nSlider[0] = (UINT8)k0;
	pgi->Input.Slider.SliderAxis.nSlider[1] = (UINT8)k1;

	if (nSlide == 2) {
		// Sliding
		pgi->Input.Slider.nSliderSpeed = 0x0700;
		pgi->Input.Slider.nSliderCenter = 0;
	} else {
		// Sliding (centering)
		pgi->Input.Slider.nSliderSpeed = 0x0E00;
		pgi->Input.Slider.nSliderCenter = 10;
	}
}

INT32 GamcAnalogKey(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nSlide)
{
	char *szSearch = NULL;
	INT32 k0 = 0, k1 = 0;
	szSearch = szPlay[nPlayer & 3];
	if (_strnicmp(szSearch, szi, 3) != 0) {			// Not our player
		return 1;
	}
	szi += 3;

	if (*szi == 0) {
		return 1;
	}
	if (strcmp(szi + 1, "-axis") != 0) {
		return 1;
	}

	// Keyboard
	if (szi[0] == 'x') {
		k0 = FBK_LEFTARROW;
		k1 = FBK_RIGHTARROW;
	}
	if (szi[0] == 'y') {
		k0 = FBK_UPARROW;
		k1 = FBK_DOWNARROW;
	}
	if (szi[0] == 'z') {
		k0 = FBK_V;
		k1 = FBK_F;
	}
	if (k0 == 0 || k1 == 0) {
		return 1;
	}

	SetSliderKey(pgi, k0, k1, nSlide);

	return 0;
}

INT32 GamcAnalogJoy(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nJoy, INT32 nSlide)
{
	INT32 nAxis = 0;

	char* szSearch = szPlay[nPlayer & 3];
	if (_strnicmp(szSearch, szi, 3) != 0)	{	// Not our player
		return 1;
	}
	szi += 3;

	if (szi[0] == 0) {
		return 1;
	}
	if (strncmp(szi + 1, "-axis", 5) != 0) {
		return 1;
	}

	if (strncmp(szi, "x", 1) == 0) {
		nAxis = 0;
	}
	if (strncmp(szi, "y", 1) == 0) {
		nAxis = 1;
	}
	if (strncmp(szi, "z", 1) == 0) {
		nAxis = 2;
	}

	if (strlen(szi) > 6) {
		if (strcmp(&szi[6], "-neg") == 0) {
			nSlide = 3;
		}
		if (strcmp(&szi[6], "-pos") == 0) {
			nSlide = 4;
		}
	}

	switch (nSlide) {
		case 2:								// Sliding
			pgi->nInput = GIT_JOYSLIDER;
			pgi->Input.Slider.nSliderValue = 0x8000;		// Put slider in the middle
			pgi->Input.Slider.nSliderSpeed = 0x0700;
			pgi->Input.Slider.nSliderCenter = 0;
			pgi->Input.Slider.JoyAxis.nAxis = nAxis;
			pgi->Input.Slider.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 1:								// Sliding (centering)
			pgi->nInput = GIT_JOYSLIDER;
			pgi->Input.Slider.nSliderValue = 0x8000;		// Put slider in the middle
			pgi->Input.Slider.nSliderSpeed = 0x0E00;
			pgi->Input.Slider.nSliderCenter = 10;
			pgi->Input.Slider.JoyAxis.nAxis = nAxis;
			pgi->Input.Slider.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 3:								// Absolute, axis-neg
			pgi->nInput = GIT_JOYAXIS_NEG;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 4:								// Absolute, axis-pos
			pgi->nInput = GIT_JOYAXIS_POS;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		default:							// Absolute, entire axis
			pgi->nInput = GIT_JOYAXIS_FULL;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
	}

	return 0;
}

// Set a Game Input to use Device 'nDevice' if it belongs to 'nPlayer'
// -2 = nothing  -1 == keyboard, 0 == joystick 1, 1 == joystick 2 etc...
INT32 GamcPlayer(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nDevice)
{
	char* szSearch = szPlay[nPlayer & 3];
	INT32 nJoyBase = 0;
	
	if (_strnicmp(szSearch, szi, 3) != 0) {	// Not our player
		return 1;
	}
	szi += 3;
	
	if (nDevice <= -2) {
		INT32 bOurs = 0;
		if (strcmp(szi, "up") == 0 || strcmp(szi, "y-axis-neg") == 0) {
			bOurs = 1;
		}
		if (strcmp(szi, "down") == 0 || strcmp(szi, "y-axis-pos") == 0) {
			bOurs = 1;
		}
		if (strcmp(szi, "left") == 0 || strcmp(szi, "x-axis-neg") == 0) {
			bOurs = 1;
		}
		if (strcmp(szi, "right") == 0 || strcmp(szi, "x-axis-pos") == 0) {
			bOurs = 1;
		}
		if (strncmp(szi, "fire ", 5) == 0) {
			bOurs = 1;
		}

		if (!bOurs)	{
			return 1;
		}

		pgi->nInput = GIT_CONSTANT;			// Constant zero
		pgi->Input.Constant.nConst = 0;		//

		return 0;
	}

	// Now check the rest of it
	if (nDevice == -1) {
		// Keyboard
		if (strcmp(szi, "up") == 0 || strcmp(szi, "y-axis-neg") == 0) {
			KEY(FBK_UPARROW);
		}
		if (strcmp(szi, "down") == 0 || strcmp(szi, "y-axis-pos") == 0) {
			KEY(FBK_DOWNARROW);
		}
		if (strcmp(szi, "left") == 0 || strcmp(szi, "x-axis-neg") == 0) {
			KEY(FBK_LEFTARROW);
		}
		if (strcmp(szi, "right") == 0 || strcmp(szi, "x-axis-pos") == 0) {
			KEY(FBK_RIGHTARROW);
		}
		if (nFireButtons == 4) {
			if (strcmp(szi, "fire 1") == 0) {
				KEY(FBK_Z);
			}
			if (strcmp(szi, "fire 2") == 0) {
				KEY(FBK_X);
			}
			if (strcmp(szi, "fire 3") == 0) {
				KEY(FBK_C);
			}
			if (strcmp(szi, "fire 4") == 0) {
				KEY(FBK_V);
			}

//			if (_stricmp(szi, "Button ABC") == 0) {
//				MACRO(FBK_A);
//			}
//			if (_stricmp(szi, "Button BCD") == 0) {
//				MACRO(FBK_S);
//			}
//			if (_stricmp(szi, "Button ABCD") == 0) {
//				MACRO(FBK_D);
//			}

		} else {
			if (bStreetFighterLayout) {
				if (strcmp(szi, "fire 1") == 0) {
					KEY(FBK_A);
				}
				if (strcmp(szi, "fire 2") == 0) {
					KEY(FBK_S);
				}
				if (strcmp(szi, "fire 3") == 0) {
					KEY(FBK_D);
				}
				if (strcmp(szi, "fire 4") == 0) {
					KEY(FBK_Z);
				}
				if (strcmp(szi, "fire 5") == 0) {
					KEY(FBK_X);
				}
				if (strcmp(szi, "fire 6") == 0) {
					KEY(FBK_C);
				}
				if (_stricmp(szi, "3× Punch") == 0) {
					MACRO(FBK_F);
				}
				if (_stricmp(szi, "3× Kick") == 0) {
					MACRO(FBK_V);
				}
			} else {
				if (strcmp(szi, "fire 1") == 0) {
					KEY(FBK_Z);
				}
				if (strcmp(szi, "fire 2") == 0) {
					KEY(FBK_X);
				}
				if (strcmp(szi, "fire 3") == 0) {
					KEY(FBK_C);
				}
				if (strcmp(szi, "fire 4") == 0) {
					KEY(FBK_A);
				}
				if (strcmp(szi, "fire 5") == 0) {
					KEY(FBK_S);
				}
				if (strcmp(szi, "fire 6") == 0) {
					KEY(FBK_D);
				}
				if (strcmp(szi, "fire 7") == 0) {
					KEY(FBK_Q);
				}
				if (strcmp(szi, "fire 8") == 0) {
					KEY(FBK_W);
				}
				if (strcmp(szi, "fire 9") == 0) {
					KEY(FBK_E);
				}
			}
		}

		return 0;
	}

	// Joystick
	nJoyBase = 0x4000;
	nJoyBase |= nDevice << 8;

	if (strcmp(szi, "up") == 0)	{
		KEY(nJoyBase + 0x02)
	}
	if (strcmp(szi, "down") == 0) {
		KEY(nJoyBase + 0x03)
	}
	if (strcmp(szi, "left") == 0)	{
		KEY(nJoyBase + 0x00)
	}
	if (strcmp(szi, "right") == 0) {
		KEY(nJoyBase + 0x01)
	}
	if (strncmp(szi, "fire ", 5) == 0) {
		char *szb = szi + 5;
		INT32 nButton = strtol(szb, NULL, 0);
		if (nButton >= 1) {
			nButton--;
		}
		KEY(nJoyBase + 0x80 + nButton);
	}

	return 0;
}

INT32 GamcPlayerHotRod(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nFlags, INT32 nSlide)
{
	char *szSearch = NULL;
	szSearch = szPlay[nPlayer & 3];
	INT32 k0 = 0, k1 = 0;

	if (_strnicmp(szSearch, szi, 3) != 0) {				// Not our player
		return 1;
	}
	szi += 3;

	if ((nFlags & 1) == 0) {

		// X-Arcade / Hanaho HotRod joystick left side
		if (strcmp(szi, "x-axis") == 0) {
			k0 = FBK_NUMPAD4;
			k1 = FBK_NUMPAD6;
		}
		if (strcmp(szi, "y-axis") == 0) {
			k0 = FBK_NUMPAD8;
			k1 = FBK_NUMPAD2;
		}

		if (strcmp(szi, "up") == 0 || strcmp(szi, "y-axis-neg") == 0) {
			KEY(FBK_NUMPAD8);
		}
		if (strcmp(szi, "down") == 0 || strcmp(szi, "y-axis-pos") == 0) {
			KEY(FBK_NUMPAD2);
		}
		if (strcmp(szi, "left") == 0 || strcmp(szi, "x-axis-neg") == 0) {
			KEY(FBK_NUMPAD4);
		}
		if (strcmp(szi, "right") == 0 || strcmp(szi, "x-axis-pos") == 0) {
			KEY(FBK_NUMPAD6);
		}

		if (nFireButtons == 4) {
			if (strcmp(szi, "fire 1") == 0) {
				KEY(FBK_C);
			}
			if (strcmp(szi, "fire 2") == 0) {
				KEY(FBK_LSHIFT);
			}
			if (strcmp(szi, "fire 3") == 0) {
				KEY(FBK_Z);
			}
			if (strcmp(szi, "fire 4") == 0) {
				KEY(FBK_X);
			}
		} else {
			if (bStreetFighterLayout) {
				// Street Fighter and other Capcom 6-button games
				if (strcmp(szi, "fire 1") == 0) {
					KEY(FBK_LCONTROL);
				}
				if (strcmp(szi, "fire 2") == 0) {
					KEY(FBK_LALT);
				}
				if (strcmp(szi, "fire 3") == 0) {
					KEY(FBK_SPACE);
				}
				if (strcmp(szi, "fire 4") == 0) {
					KEY(FBK_LSHIFT);
				}
				if (strcmp(szi, "fire 5") == 0) {
					KEY(FBK_Z);
				}
				if (strcmp(szi, "fire 6") == 0) {
					KEY(FBK_X);
				}

				// Map the 3x macros on X-Arcade
				if ((nFlags & 0x10)) {
					if (_stricmp(szi, "3× Punch") == 0) {
						MACRO(FBK_C);
					}
					if (_stricmp(szi, "3× Kick") == 0) {
						MACRO(FBK_5);
					}
				}
			} else {
				if (strcmp(szi, "fire 1") == 0) {
					KEY(FBK_LSHIFT);
				}
				if (strcmp(szi, "fire 2") == 0) {
					KEY(FBK_Z);
				}
				if (strcmp(szi, "fire 3") == 0) {
					KEY(FBK_X);
				}
				if (strcmp(szi, "fire 4") == 0) {
					KEY(FBK_LCONTROL);
				}
				if (strcmp(szi, "fire 5") == 0) {
					KEY(FBK_LALT);
				}
				if (strcmp(szi, "fire 6") == 0) {
					KEY(FBK_SPACE);
				}
			}
		}

		// Map remaining buttons as usual
		if (strcmp(szi, "start") == 0) {
			KEY(FBK_1);
		}
		if (strcmp(szi, "coin") == 0 || strcmp(szi, "select") == 0) {
			KEY(FBK_3);
		}

		if (k0 && k1) {
			SetSliderKey(pgi, k0, k1, nSlide);
		}

		return 0;

	} else {
		if ((nFlags & 1) == 1) {
			// X-Arcade / Hanaho HotRod joystick right side

			if (strcmp(szi, "x-axis") == 0) {
				k0 = FBK_D;
				k1 = FBK_G;
			}
			if (strcmp(szi, "y-axis") == 0) {
				k0 = FBK_R;
				k1 = FBK_F;
			}

			if (strcmp(szi, "up") == 0 || strcmp(szi, "y-axis-neg") == 0)	{
				KEY(FBK_R);
			}
			if (strcmp(szi, "down") == 0 || strcmp(szi, "y-axis-pos") == 0) {
				KEY(FBK_F);
			}
			if (strcmp(szi, "left") == 0 || strcmp(szi, "x-axis-neg") == 0) {
				KEY(FBK_D);
			}
			if (strcmp(szi, "right") == 0 || strcmp(szi, "x-axis-pos") == 0) {
				KEY(FBK_G);
			}

			if (nFireButtons == 4) {
				if (strcmp(szi, "fire 1") == 0) {
					KEY(FBK_RBRACKET);
				}
				if (strcmp(szi, "fire 2") == 0) {
					KEY(FBK_W);
				}
				if (strcmp(szi, "fire 3") == 0) {
					KEY(FBK_E);
				}
				if (strcmp(szi, "fire 4") == 0) {
					KEY(FBK_LBRACKET);
				}
			} else {
				if (bStreetFighterLayout) {
					// Street Fighter and other Capcom 6-button games
					if (strcmp(szi, "fire 1") == 0) {
						KEY(FBK_A);
					}
					if (strcmp(szi, "fire 2") == 0) {
						KEY(FBK_S);
					}
					if (strcmp(szi, "fire 3") == 0) {
						KEY(FBK_Q);
					}
					if (strcmp(szi, "fire 4") == 0) {
						KEY(FBK_W);
					}
					if (strcmp(szi, "fire 5") == 0) {
						KEY(FBK_E);
					}
					if (strcmp(szi, "fire 6") == 0) {
						KEY(FBK_LBRACKET);
					}

					// Map the 3x macros on X-Arcade
					if ((nFlags & 0x10)) {
						if (_stricmp(szi, "3× Punch") == 0) {
							MACRO(FBK_RBRACKET);
						}
						if (_stricmp(szi, "3× Kick") == 0) {
							MACRO(FBK_6);
						}
					}
				} else {
					if (strcmp(szi, "fire 1") == 0) {
						KEY(FBK_W);
					}
					if (strcmp(szi, "fire 2") == 0) {
						KEY(FBK_E);
					}
					if (strcmp(szi, "fire 3") == 0) {
						KEY(FBK_LBRACKET);
					}
					if (strcmp(szi, "fire 4") == 0) {
						KEY(FBK_A);
					}
					if (strcmp(szi, "fire 5") == 0) {
						KEY(FBK_S);
					}
					if (strcmp(szi, "fire 6") == 0) {
						KEY(FBK_Q);
					}
				}
			}

			// Map remaining buttons as usual
			if (strcmp(szi, "start") == 0) {
				KEY(FBK_2);
			}
			if (strcmp(szi, "coin") == 0 || strcmp(szi, "select") == 0) {
				KEY(FBK_4);
			}

			if (k0 && k1) {
				SetSliderKey(pgi, k0, k1, nSlide);
			}

			return 0;
		}
	}

	return 1;												// Couldn't map input
}

#undef MACRO
#undef KEY

