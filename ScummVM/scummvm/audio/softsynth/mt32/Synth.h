/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_SYNTH_H
#define MT32EMU_SYNTH_H

//#include <cstdarg>

namespace MT32Emu {

class TableInitialiser;
class Partial;
class PartialManager;
class Part;

/**
 * Methods for emulating the connection between the LA32 and the DAC, which involves
 * some hacks in the real devices for doubling the volume.
 * See also http://en.wikipedia.org/wiki/Roland_MT-32#Digital_overflow
 */
enum DACInputMode {
	// Produces samples at double the volume, without tricks.
	// * Nicer overdrive characteristics than the DAC hacks (it simply clips samples within range)
	// * Higher quality than the real devices
	DACInputMode_NICE,

	// Produces samples that exactly match the bits output from the emulated LA32.
	// * Nicer overdrive characteristics than the DAC hacks (it simply clips samples within range)
	// * Much less likely to overdrive than any other mode.
	// * Half the volume of any of the other modes, meaning its volume relative to the reverb
	//   output when mixed together directly will sound wrong.
	// * Perfect for developers while debugging :)
	DACInputMode_PURE,

	// Re-orders the LA32 output bits as in early generation MT-32s (according to Wikipedia).
	// Bit order at DAC (where each number represents the original LA32 output bit number, and XX means the bit is always low):
	// 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00 XX
	DACInputMode_GENERATION1,

	// Re-orders the LA32 output bits as in later generations (personally confirmed on my CM-32L - KG).
	// Bit order at DAC (where each number represents the original LA32 output bit number):
	// 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00 14
	DACInputMode_GENERATION2
};

enum ReportType {
	// Errors
	ReportType_errorControlROM = 1,
	ReportType_errorPCMROM,
	ReportType_errorSampleRate,

	// Progress
	ReportType_progressInit,

	// HW spec
	ReportType_availableSSE,
	ReportType_available3DNow,
	ReportType_usingSSE,
	ReportType_using3DNow,

	// General info
	ReportType_lcdMessage,
	ReportType_devReset,
	ReportType_devReconfig,
	ReportType_newReverbMode,
	ReportType_newReverbTime,
	ReportType_newReverbLevel
};

enum LoadResult {
	LoadResult_OK,
	LoadResult_NotFound,
	LoadResult_Unreadable,
	LoadResult_Invalid
};

struct SynthProperties {
	// Sample rate to use in mixing
	unsigned int sampleRate;

	// Deprecated - ignored. Use Synth::setReverbEnabled() instead.
	bool useReverb;
	// Deprecated - ignored. Use Synth::setReverbOverridden() instead.
	bool useDefaultReverb;
	// Deprecated - ignored. Use Synth::playSysex*() to configure reverb instead.
	unsigned char reverbType;
	// Deprecated - ignored. Use Synth::playSysex*() to configure reverb instead.
	unsigned char reverbTime;
	// Deprecated - ignored. Use Synth::playSysex*() to configure reverb instead.
	unsigned char reverbLevel;
	// The name of the directory in which the ROM and data files are stored (with trailing slash/backslash)
	// Not used if "openFile" is set. May be NULL in any case.
	const char *baseDir;
	// This is used as the first argument to all callbacks
	void *userData;
	// Callback for reporting various errors and information. May be NULL
	int (*report)(void *userData, ReportType type, const void *reportData);
	// Callback for debug messages, in vprintf() format
	void (*printDebug)(void *userData, const char *fmt, va_list list);
	// Callback for providing an implementation of File, opened and ready for use
	// May be NULL, in which case a default implementation will be used.
	Common::File *(*openFile)(void *userData, const char *filename);
	// Callback for closing a File. May be NULL, in which case the File will automatically be close()d/deleted.
	void (*closeFile)(void *userData, Common::File *file);
};

// This is the specification of the Callback routine used when calling the RecalcWaveforms
// function
typedef void (*recalcStatusCallback)(int percDone);

typedef void (*FloatToBit16sFunc)(Bit16s *target, const float *source, Bit32u len, float outputGain);

const Bit8u SYSEX_MANUFACTURER_ROLAND = 0x41;

const Bit8u SYSEX_MDL_MT32 = 0x16;
const Bit8u SYSEX_MDL_D50 = 0x14;

const Bit8u SYSEX_CMD_RQ1 = 0x11; // Request data #1
const Bit8u SYSEX_CMD_DT1 = 0x12; // Data set 1
const Bit8u SYSEX_CMD_WSD = 0x40; // Want to send data
const Bit8u SYSEX_CMD_RQD = 0x41; // Request data
const Bit8u SYSEX_CMD_DAT = 0x42; // Data set
const Bit8u SYSEX_CMD_ACK = 0x43; // Acknowledge
const Bit8u SYSEX_CMD_EOD = 0x45; // End of data
const Bit8u SYSEX_CMD_ERR = 0x4E; // Communications error
const Bit8u SYSEX_CMD_RJC = 0x4F; // Rejection

const int MAX_SYSEX_SIZE = 512;

const unsigned int CONTROL_ROM_SIZE = 64 * 1024;

struct ControlROMPCMStruct {
	Bit8u pos;
	Bit8u len;
	Bit8u pitchLSB;
	Bit8u pitchMSB;
};

struct ControlROMMap {
	Bit16u idPos;
	Bit16u idLen;
	const char *idBytes;
	Bit16u pcmTable; // 4 * pcmCount bytes
	Bit16u pcmCount;
	Bit16u timbreAMap; // 128 bytes
	Bit16u timbreAOffset;
	bool timbreACompressed;
	Bit16u timbreBMap; // 128 bytes
	Bit16u timbreBOffset;
	bool timbreBCompressed;
	Bit16u timbreRMap; // 2 * timbreRCount bytes
	Bit16u timbreRCount;
	Bit16u rhythmSettings; // 4 * rhythmSettingsCount bytes
	Bit16u rhythmSettingsCount;
	Bit16u reserveSettings; // 9 bytes
	Bit16u panSettings; // 8 bytes
	Bit16u programSettings; // 8 bytes
	Bit16u rhythmMaxTable; // 4 bytes
	Bit16u patchMaxTable; // 16 bytes
	Bit16u systemMaxTable; // 23 bytes
	Bit16u timbreMaxTable; // 72 bytes
};

enum MemoryRegionType {
	MR_PatchTemp, MR_RhythmTemp, MR_TimbreTemp, MR_Patches, MR_Timbres, MR_System, MR_Display, MR_Reset
};

class MemoryRegion {
private:
	Synth *synth;
	Bit8u *realMemory;
	Bit8u *maxTable;
public:
	MemoryRegionType type;
	Bit32u startAddr, entrySize, entries;

	MemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable, MemoryRegionType useType, Bit32u useStartAddr, Bit32u useEntrySize, Bit32u useEntries) {
		synth = useSynth;
		realMemory = useRealMemory;
		maxTable = useMaxTable;
		type = useType;
		startAddr = useStartAddr;
		entrySize = useEntrySize;
		entries = useEntries;
	}
	int lastTouched(Bit32u addr, Bit32u len) const {
		return (offset(addr) + len - 1) / entrySize;
	}
	int firstTouchedOffset(Bit32u addr) const {
		return offset(addr) % entrySize;
	}
	int firstTouched(Bit32u addr) const {
		return offset(addr) / entrySize;
	}
	Bit32u regionEnd() const {
		return startAddr + entrySize * entries;
	}
	bool contains(Bit32u addr) const {
		return addr >= startAddr && addr < regionEnd();
	}
	int offset(Bit32u addr) const {
		return addr - startAddr;
	}
	Bit32u getClampedLen(Bit32u addr, Bit32u len) const {
		if (addr + len > regionEnd())
			return regionEnd() - addr;
		return len;
	}
	Bit32u next(Bit32u addr, Bit32u len) const {
		if (addr + len > regionEnd()) {
			return regionEnd() - addr;
		}
		return 0;
	}
	Bit8u getMaxValue(int off) const {
		if (maxTable == NULL)
			return 0xFF;
		return maxTable[off % entrySize];
	}
	Bit8u *getRealMemory() const {
		return realMemory;
	}
	bool isReadable() const {
		return getRealMemory() != NULL;
	}
	void read(unsigned int entry, unsigned int off, Bit8u *dst, unsigned int len) const;
	void write(unsigned int entry, unsigned int off, const Bit8u *src, unsigned int len, bool init = false) const;
};

class PatchTempMemoryRegion : public MemoryRegion {
public:
	PatchTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_PatchTemp, MT32EMU_MEMADDR(0x030000), sizeof(MemParams::PatchTemp), 9) {}
};
class RhythmTempMemoryRegion : public MemoryRegion {
public:
	RhythmTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_RhythmTemp, MT32EMU_MEMADDR(0x030110), sizeof(MemParams::RhythmTemp), 85) {}
};
class TimbreTempMemoryRegion : public MemoryRegion {
public:
	TimbreTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_TimbreTemp, MT32EMU_MEMADDR(0x040000), sizeof(TimbreParam), 8) {}
};
class PatchesMemoryRegion : public MemoryRegion {
public:
	PatchesMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_Patches, MT32EMU_MEMADDR(0x050000), sizeof(PatchParam), 128) {}
};
class TimbresMemoryRegion : public MemoryRegion {
public:
	TimbresMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_Timbres, MT32EMU_MEMADDR(0x080000), sizeof(MemParams::PaddedTimbre), 64 + 64 + 64 + 64) {}
};
class SystemMemoryRegion : public MemoryRegion {
public:
	SystemMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_System, MT32EMU_MEMADDR(0x100000), sizeof(MemParams::System), 1) {}
};
class DisplayMemoryRegion : public MemoryRegion {
public:
	DisplayMemoryRegion(Synth *useSynth) : MemoryRegion(useSynth, NULL, NULL, MR_Display, MT32EMU_MEMADDR(0x200000), MAX_SYSEX_SIZE - 1, 1) {}
};
class ResetMemoryRegion : public MemoryRegion {
public:
	ResetMemoryRegion(Synth *useSynth) : MemoryRegion(useSynth, NULL, NULL, MR_Reset, MT32EMU_MEMADDR(0x7F0000), 0x3FFF, 1) {}
};

class ReverbModel {
public:
	virtual ~ReverbModel() {}
	// After construction or a close(), open() will be called at least once before any other call (with the exception of close()).
	virtual void open(unsigned int sampleRate) = 0;
	// May be called multiple times without an open() in between.
	virtual void close() = 0;
	virtual void setParameters(Bit8u time, Bit8u level) = 0;
	virtual void process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples) = 0;
	virtual bool isActive() const = 0;
};

class Synth {
friend class Part;
friend class RhythmPart;
friend class Poly;
friend class Partial;
friend class Tables;
friend class MemoryRegion;
friend class TVA;
friend class TVF;
friend class TVP;
private:
	PatchTempMemoryRegion *patchTempMemoryRegion;
	RhythmTempMemoryRegion *rhythmTempMemoryRegion;
	TimbreTempMemoryRegion *timbreTempMemoryRegion;
	PatchesMemoryRegion *patchesMemoryRegion;
	TimbresMemoryRegion *timbresMemoryRegion;
	SystemMemoryRegion *systemMemoryRegion;
	DisplayMemoryRegion *displayMemoryRegion;
	ResetMemoryRegion *resetMemoryRegion;

	Bit8u *paddedTimbreMaxTable;

	bool isEnabled;

	PCMWaveEntry *pcmWaves; // Array

	const ControlROMMap *controlROMMap;
	Bit8u controlROMData[CONTROL_ROM_SIZE];
	float *pcmROMData;
	int pcmROMSize; // This is in 16-bit samples, therefore half the number of bytes in the ROM

	Bit8s chantable[32];

	Bit32u renderedSampleCount;


	MemParams mt32ram, mt32default;

	ReverbModel *reverbModels[4];
	ReverbModel *reverbModel;
	bool reverbEnabled;
	bool reverbOverridden;

	FloatToBit16sFunc la32FloatToBit16sFunc;
	FloatToBit16sFunc reverbFloatToBit16sFunc;
	float outputGain;
	float reverbOutputGain;

	bool isOpen;

	PartialManager *partialManager;
	Part *parts[9];

	// FIXME: We can reorganise things so that we don't need all these separate tmpBuf, tmp and prerender buffers.
	// This should be rationalised when things have stabilised a bit (if prerender buffers don't die in the mean time).

	float tmpBufPartialLeft[MAX_SAMPLES_PER_RUN];
	float tmpBufPartialRight[MAX_SAMPLES_PER_RUN];
	float tmpBufMixLeft[MAX_SAMPLES_PER_RUN];
	float tmpBufMixRight[MAX_SAMPLES_PER_RUN];
	float tmpBufReverbOutLeft[MAX_SAMPLES_PER_RUN];
	float tmpBufReverbOutRight[MAX_SAMPLES_PER_RUN];

	Bit16s tmpNonReverbLeft[MAX_SAMPLES_PER_RUN];
	Bit16s tmpNonReverbRight[MAX_SAMPLES_PER_RUN];
	Bit16s tmpReverbDryLeft[MAX_SAMPLES_PER_RUN];
	Bit16s tmpReverbDryRight[MAX_SAMPLES_PER_RUN];
	Bit16s tmpReverbWetLeft[MAX_SAMPLES_PER_RUN];
	Bit16s tmpReverbWetRight[MAX_SAMPLES_PER_RUN];

	// These ring buffers are only used to simulate delays present on the real device.
	// In particular, when a partial needs to be aborted to free it up for use by a new Poly,
	// the controller will busy-loop waiting for the sound to finish.
	Bit16s prerenderNonReverbLeft[MAX_PRERENDER_SAMPLES];
	Bit16s prerenderNonReverbRight[MAX_PRERENDER_SAMPLES];
	Bit16s prerenderReverbDryLeft[MAX_PRERENDER_SAMPLES];
	Bit16s prerenderReverbDryRight[MAX_PRERENDER_SAMPLES];
	Bit16s prerenderReverbWetLeft[MAX_PRERENDER_SAMPLES];
	Bit16s prerenderReverbWetRight[MAX_PRERENDER_SAMPLES];
	int prerenderReadIx;
	int prerenderWriteIx;

	SynthProperties myProp;

	bool prerender();
	void copyPrerender(Bit16s *nonReverbLeft, Bit16s *nonReverbRight, Bit16s *reverbDryLeft, Bit16s *reverbDryRight, Bit16s *reverbWetLeft, Bit16s *reverbWetRight, Bit32u pos, Bit32u len);
	void checkPrerender(Bit16s *nonReverbLeft, Bit16s *nonReverbRight, Bit16s *reverbDryLeft, Bit16s *reverbDryRight, Bit16s *reverbWetLeft, Bit16s *reverbWetRight, Bit32u &pos, Bit32u &len);
	void doRenderStreams(Bit16s *nonReverbLeft, Bit16s *nonReverbRight, Bit16s *reverbDryLeft, Bit16s *reverbDryRight, Bit16s *reverbWetLeft, Bit16s *reverbWetRight, Bit32u len);

	void playAddressedSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);
	void readSysex(unsigned char channel, const Bit8u *sysex, Bit32u len) const;
	void initMemoryRegions();
	void deleteMemoryRegions();
	MemoryRegion *findMemoryRegion(Bit32u addr);
	void writeMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, const Bit8u *data);
	void readMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, Bit8u *data);

	LoadResult loadControlROM(const char *filename);
	LoadResult loadPCMROM(const char *filename);

	bool initPCMList(Bit16u mapAddress, Bit16u count);
	bool initTimbres(Bit16u mapAddress, Bit16u offset, int timbreCount, int startTimbre, bool compressed);
	bool initCompressedTimbre(int drumNum, const Bit8u *mem, unsigned int memLen);

	void refreshSystemMasterTune();
	void refreshSystemReverbParameters();
	void refreshSystemReserveSettings();
	void refreshSystemChanAssign(unsigned int firstPart, unsigned int lastPart);
	void refreshSystemMasterVol();
	void refreshSystem();
	void reset();

	unsigned int getSampleRate() const;

	void printPartialUsage(unsigned long sampleOffset = 0);
protected:
	int report(ReportType type, const void *reportData);
	Common::File *openFile(const char *filename);
	void closeFile(Common::File *file);
	void printDebug(const char *fmt, ...);

public:
	static Bit8u calcSysexChecksum(const Bit8u *data, Bit32u len, Bit8u checksum);

	Synth();
	~Synth();

	// Used to initialise the MT-32. Must be called before any other function.
	// Returns true if initialization was sucessful, otherwise returns false.
	bool open(SynthProperties &useProp);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void close(void);

	// Sends a 4-byte MIDI message to the MT-32 for immediate playback
	void playMsg(Bit32u msg);
	void playMsgOnPart(unsigned char part, unsigned char code, unsigned char note, unsigned char velocity);

	// Sends a string of Sysex commands to the MT-32 for immediate interpretation
	// The length is in bytes
	void playSysex(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutFraming(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutHeader(unsigned char device, unsigned char command, const Bit8u *sysex, Bit32u len);
	void writeSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);

	void setReverbEnabled(bool reverbEnabled);
	bool isReverbEnabled() const;
	void setReverbOverridden(bool reverbOverridden);
	bool isReverbOverridden() const;
	void setDACInputMode(DACInputMode mode);

	// Sets output gain factor. Applied to all output samples and unrelated with the synth's Master volume.
	void setOutputGain(float);

	// Sets output gain factor for the reverb wet output. setOutputGain() doesn't change reverb output gain.
	void setReverbOutputGain(float);

	// Renders samples to the specified output stream.
	// The length is in frames, not bytes (in 16-bit stereo,
	// one frame is 4 bytes).
	void render(Bit16s *stream, Bit32u len);

	// Renders samples to the specified output streams (any or all of which may be NULL).
	void renderStreams(Bit16s *nonReverbLeft, Bit16s *nonReverbRight, Bit16s *reverbDryLeft, Bit16s *reverbDryRight, Bit16s *reverbWetLeft, Bit16s *reverbWetRight, Bit32u len);

	// Returns true when there is at least one active partial, otherwise false.
	bool hasActivePartials() const;

	// Returns true if hasActivePartials() returns true, or reverb is (somewhat unreliably) detected as being active.
	bool isActive() const;

	const Partial *getPartial(unsigned int partialNum) const;

	void readMemory(Bit32u addr, Bit32u len, Bit8u *data);

	// partNum should be 0..7 for Part 1..8, or 8 for Rhythm
	const Part *getPart(unsigned int partNum) const;
};

}

#endif
