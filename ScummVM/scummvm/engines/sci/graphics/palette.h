/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_GRAPHICS_PALETTE_H
#define SCI_GRAPHICS_PALETTE_H

#include "common/array.h"
#include "sci/graphics/helpers.h"

namespace Sci {

class ResourceManager;
class GfxScreen;

enum ColorRemappingType {
	kRemappingNone = 0,
	kRemappingByRange = 1,
	kRemappingByPercent = 2
};

/**
 * Palette class, handles palette operations like changing intensity, setting up the palette, merging different palettes
 */
class GfxPalette : public Common::Serializable {
public:
	GfxPalette(ResourceManager *resMan, GfxScreen *screen);
	~GfxPalette();

	bool isMerging();

	void setDefault();
	void createFromData(byte *data, int bytesLeft, Palette *paletteOut);
	bool setAmiga();
	void modifyAmigaPalette(byte *data);
	void setEGA();
	void set(Palette *sciPal, bool force, bool forceRealMerge = false);
	bool insert(Palette *newPalette, Palette *destPalette);
	bool merge(Palette *pFrom, bool force, bool forceRealMerge);
	uint16 matchColor(byte r, byte g, byte b);
	void getSys(Palette *pal);
	uint16 getTotalColorCount() const { return _totalScreenColors; }

	void resetRemapping();
	void setRemappingPercent(byte color, byte percent);
	void setRemappingPercentGray(byte color, byte percent);
	void setRemappingRange(byte color, byte from, byte to, byte base);
	bool isRemapped(byte color) const {
		return _remapOn && (_remappingType[color] != kRemappingNone);
	}
	byte remapColor(byte remappedColor, byte screenColor);

	void setOnScreen();
	void copySysPaletteToScreen();

	void drewPicture(GuiResourceId pictureId);

	bool kernelSetFromResource(GuiResourceId resourceId, bool force);
	void kernelSetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void kernelUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void kernelSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette);
	int16 kernelFindColor(uint16 r, uint16 g, uint16 b);
	bool kernelAnimate(byte fromColor, byte toColor, int speed);
	void kernelAnimateSet();
	reg_t kernelSave();
	void kernelRestore(reg_t memoryHandle);
	void kernelAssertPalette(GuiResourceId resourceId);

	void kernelSyncScreenPalette();

	bool kernelPalVaryInit(GuiResourceId resourceId, uint16 ticks, uint16 stepStop, uint16 direction);
	int16 kernelPalVaryReverse(int16 ticks, uint16 stepStop, int16 direction);
	int16 kernelPalVaryGetCurrentStep();
	int16 kernelPalVaryChangeTarget(GuiResourceId resourceId);
	void kernelPalVaryChangeTicks(uint16 ticks);
	void kernelPalVaryPause(bool pause);
	void kernelPalVaryDeinit();
	void palVaryUpdate();
	void palVaryPrepareForTransition();
	void palVaryProcess(int signal, bool setPalette);

	Palette _sysPalette;

	virtual void saveLoadWithSerializer(Common::Serializer &s);
	void palVarySaveLoadPalette(Common::Serializer &s, Palette *palette);

	byte findMacIconBarColor(byte r, byte g, byte b);
	bool colorIsFromMacClut(byte index);

#ifdef ENABLE_SCI32
	bool loadClut(uint16 clutId);
	byte matchClutColor(uint16 color);
	void unloadClut();
#endif

private:
	void palVaryInit();
	void palVaryInstallTimer();
	void palVaryRemoveTimer();
	bool palVaryLoadTargetPalette(GuiResourceId resourceId);
	static void palVaryCallback(void *refCon);
	void palVaryIncreaseSignal();

	GfxScreen *_screen;
	ResourceManager *_resMan;

	bool _sysPaletteChanged;
	bool _useMerging;

	Common::Array<PalSchedule> _schedules;

	GuiResourceId _palVaryResourceId;
	Palette _palVaryOriginPalette;
	Palette _palVaryTargetPalette;
	int16 _palVaryStep;
	int16 _palVaryStepStop;
	int16 _palVaryDirection;
	uint16 _palVaryTicks;
	int _palVaryPaused;
	int _palVarySignal;
	uint16 _totalScreenColors;

	bool _remapOn;
	ColorRemappingType _remappingType[256];
	byte _remappingByPercent[256];
	byte _remappingByRange[256];
	uint16 _remappingPercentToSet;

	void loadMacIconBarPalette();
	byte *_macClut;

#ifdef ENABLE_SCI32
	byte *_clutTable;
#endif
};

} // End of namespace Sci

#endif
