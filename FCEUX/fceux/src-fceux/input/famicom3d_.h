/****************************************************************************
 * This file handles the Stereoscopic 3D shutter-glasses of Famicom 3D System
 * They worked the way a modern 3D TV works, but at a much lower refresh rate
 * All we do here is set a global variable to the state of the OUT1 pin
 ****************************************************************************/

uint8 shutter_3d;

static void Famicom3DWrite(uint8 V)
{
	shutter_3d = (V >> 1) & 1;
}

static INPUTCFC Famicom3D={0,Famicom3DWrite,0,0,0};

INPUTCFC *FCEU_InitFamicom3D(void)
{
	return(&Famicom3D);
}

