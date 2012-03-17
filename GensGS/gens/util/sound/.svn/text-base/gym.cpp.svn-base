/**
 * Gens: GYM file handler.
 */

#include "gym.hpp"

#include <string.h>

#include "emulator/g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"

#include "ui/gens_ui.hpp"

#include "macros/file_m.h"


static FILE *GYM_File;
int GYM_Dumping = 0;


/**
 * Start_GYM_Dump(): Start dumping a GYM file.
 * @return 1 on success; 0 on error.
 */
int Start_GYM_Dump(void)
{
	char filename[GENS_PATH_MAX];
	unsigned char YM_Save[0x200], t_buf[4];
	int num, i;
	
	// A game must be loaded in order to dump a GYM.
	if (!Game)
		return 0;
	
	if (GYM_Dumping)
	{
		draw->writeText("GYM sound is already dumping", 1000);
		return 0;
	}
	
	// Build the filename.
	num = -1;
	do
	{
		num++;
		sprintf(filename, "%s%s_%03d.gym", PathNames.Dump_GYM_Dir, ROM_Name, num);
	} while (fileExists(filename));
	
	GYM_File = fopen(filename, "w");
	if (!GYM_File)
		return 0;
	
	// Save the YM2612 registers.
	YM2612_Save(YM_Save);
	
	for (i = 0x30; i < 0x90; i++)
	{
		t_buf[0] = 1;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i];
		fwrite(t_buf, 3, 1, GYM_File);
		
		t_buf[0] = 2;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i + 0x100];
		fwrite(t_buf, 3, 1, GYM_File);
	}
	
	for (i = 0xA0; i < 0xB8; i++)
	{
		t_buf[0] = 1;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i];
		fwrite(t_buf, 3, 1, GYM_File);
		
		t_buf[0] = 2;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i + 0x100];
		fwrite(t_buf, 3, 1, GYM_File);
	}
	
	t_buf[0] = 1;
	t_buf[1] = 0x22;
	t_buf[2] = YM_Save[0x22];
	fwrite(t_buf, 3, 1, GYM_File);
	
	t_buf[0] = 1;
	t_buf[1] = 0x27;
	t_buf[2] = YM_Save[0x27];
	fwrite(t_buf, 3, 1, GYM_File);
	
	t_buf[0] = 1;
	t_buf[1] = 0x28;
	t_buf[2] = YM_Save[0x28];
	fwrite(t_buf, 3, 1, GYM_File);
	
	draw->writeText("Starting to dump GYM sound", 1000);
	GYM_Dumping = 1;
	
	return 1;
}


/**
 * Stop_GYM_Dump(): Stop dumping a GYM file.
 * @return 1 on success; 0 on error.
 */
int Stop_GYM_Dump(void)
{
	if (!GYM_Dumping)
	{
		draw->writeText("Already stopped", 1000);
		return 0;
	}
	
	if (GYM_File)
		fclose(GYM_File);
	audio->clearSoundBuffer();
	GYM_Dumping = 0;
	
	draw->writeText("GYM dump stopped", 1000);
	return 1;
}


/**
 * Start_Play_GYM(): Start playing a GYM file.
 * @return 1 on success; 0 on error.
 */
int Start_Play_GYM(void)
{
	string filename;
	
	if (Game || !audio->enabled())
		return 0;
	
	if (audio->playingGYM())
	{
		draw->writeText("Already playing GYM", 1000);
		return 0;
	}
	
	audio->endSound();
	CPU_Mode = 0;
	
	if (!audio->initSound())
	{
		audio->setEnabled(false);
		draw->writeText("Can't initialize SDL Sound", 1000);
		return 0;
	}
	
	audio->playSound();
	
#if !defined(GENS_UI_OPENEMU)
	filename = GensUI::openFile("Load GYM File", NULL /*Rom_Dir*/, GYMFile);
	if (filename.length() == 0)
		return 0;
	
	// Attempt to open the GYM file.
	GYM_File = fopen(filename.c_str(), "rb");
	if (!GYM_File)
		return 0;
#endif
	YM2612_Init(CLOCK_NTSC / 7, audio->soundRate(), YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, audio->soundRate());
	audio->setPlayingGYM(true);
	
	draw->writeText("Starting to play GYM", 1000);
	return 1;
}


/**
 * Stop_Play_GYM(): Stop playing a GYM file.
 * @return 1 on success; 0 on error.
 */
int Stop_Play_GYM(void)
{
	if (!audio->playingGYM())
	{
		draw->writeText("Already stopped", 1000);
		return 0;
	}
	
	if (GYM_File)
		fclose(GYM_File);
	audio->clearSoundBuffer();
	audio->setPlayingGYM(false);
	
	draw->writeText("Stopped playing GYM", 1000);
	return 1;
}


int GYM_Next(void)
{
	unsigned char c, c2;
	unsigned int l;
	int *buf[2];
	
	if (!audio->playingGYM() || !GYM_File)
		return 0;
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	do
	{
		l = fread(&c, 1, 1, GYM_File);
		if (l == 0)
			return 0;
		
		switch (c)
		{
			case 0:
				PSG_Update(buf, audio->segLength());
				if (YM2612_Enable)
					YM2612_Update(buf, audio->segLength());
				break;
			
			case 1:
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(0, c2);
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(1, c2);
				break;
			
			case 2:
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(2, c2);
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(3, c2);
				break;
			
			case 3:
				fread(&c2, 1, 1, GYM_File);
				PSG_Write(c2);
				break;
		}
	} while (c);
	
	return 1;
}


int Play_GYM(void)
{
	if (!GYM_Next())
	{
		Stop_Play_GYM();
		return 0;
	}
	
	audio->writeSoundBuffer(NULL);
	return 1;
}


int Update_GYM_Dump(int v0, int v1, unsigned char v2)
{
	int bResult;
	char buf_tmp[4];
	unsigned int l;
	
	if (!GYM_Dumping || !GYM_File)
		return 0;
	
	// TODO: Should v1 be an unsigned char instead of an int?
	v1 &= 0xFF;
	
	buf_tmp[0] = v0;
	l = 1;
	
	switch (v0)
	{
		case 1:
		case 2:
			buf_tmp[1] = (unsigned char)v1;
			buf_tmp[2] = v2;
			l = 3;
			break;
		
		case 3:
			buf_tmp[1] = (unsigned char)v1;
			l = 2;
			break;
	}
	
	bResult = fwrite(buf_tmp, l, 1, GYM_File);
	if (!bResult)
		return 0;
	
	return 1;
}
