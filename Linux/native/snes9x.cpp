/* Linux platform adapter derived from SNES9x/SNESGameCore.mm.
 * Copyright (c) 2022, OpenEmu Team.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the OpenEmu Team nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY OpenEmu Team "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL OpenEmu Team BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * The linked core retains its own license at SNES9x/src/LICENSE.
 */
#include "core.h"
#include "snes9x.h"
#include "memmap.h"
#include "gfx.h"
#include "display.h"
#include "ppu.h"
#include "apu/apu.h"
#include "controls.h"
#include "snapshot.h"
#include "conffile.h"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <vector>

struct LinuxSNES {
    std::string directory, battery;
    std::vector<uint32_t> pixels = std::vector<uint32_t>(MAX_SNES_WIDTH * MAX_SNES_HEIGHT);
    std::vector<int16_t> audio;
    unsigned width = 256, height = 224;
    bool memory = false, apu = false, graphics = false, loaded = false;
    bool audio_error = false;
};
// Snes9x has process-global state. The application launches each game in a
// separate helper process; reject overlapping instances inside one process.
static LinuxSNES* current = nullptr;

static void collect_audio(void* data) {
    auto& c = *static_cast<LinuxSNES*>(data);
    int count = S9xGetSampleCount(); // Number of int16 samples, not stereo frames.
    if (count <= 0) return;
    if ((count & 1) || c.audio.size() + count > 16384) {
        c.audio_error = true;
        S9xClearSamples();
        return;
    }
    size_t offset = c.audio.size();
    c.audio.resize(offset + count);
    S9xMixSamples(reinterpret_cast<uint8*>(c.audio.data() + offset), count);
}

void* oe_create(const char* rom, const char* save_dir) {
    if (current) return nullptr;
    try {
        current = new LinuxSNES;
        current->directory = save_dir;
        current->battery = current->directory + "/" + S9xBasenameNoExt(rom) + ".sav";
        current->audio.reserve(16384);
        memset(&Settings, 0, sizeof(Settings));
        Settings.SoundPlaybackRate = 48000;
        Settings.SoundInputRate = 32040;
        Settings.DynamicRateLimit = 5;
        Settings.Transparency = true;
        Settings.DontSaveOopsSnapshot = true;
        Settings.NoPatch = true;
        Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;
        Settings.OneClockCycle = ONE_CYCLE;
        Settings.OneSlowClockCycle = SLOW_ONE_CYCLE;
        Settings.TwoClockCycles = TWO_CYCLES;
        Settings.SuperFXClockMultiplier = 100;
        Settings.BlockInvalidVRAMAccessMaster = true;
        Settings.HDMATimingHack = 100;
        Settings.MaxSpriteTilesPerLine = 34;
        GFX.InfoString.clear();
        GFX.InfoStringTimeout = 0;
        current->memory = Memory.Init();
        if (!current->memory) { oe_destroy(current); return nullptr; }
        current->apu = S9xInitAPU();
        if (!current->apu) { oe_destroy(current); return nullptr; }
        current->graphics = S9xGraphicsInit();
        if (!current->graphics || !S9xInitSound(100)) { oe_destroy(current); return nullptr; }
        S9xSetSamplesAvailableCallback(collect_audio, current);
        S9xUnmapAllControls();
        const char* buttons[] = {"A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "R", "L", "X", "Y"};
        for (unsigned i = 0; i < 12; ++i) {
            std::string name = std::string("Joypad1 ") + buttons[i];
            S9xMapButton(i + 1, S9xGetCommandT(name.c_str()), false);
        }
        if (!Memory.LoadROM(rom)) { oe_destroy(current); return nullptr; }
        // Satellite/Sufami multi-cart BIOS loading needs its own import workflow.
        if (Settings.BS || Multi.cartType) { oe_destroy(current); return nullptr; }
        current->loaded = true;
        S9xSetController(0, CTL_JOYPAD, 0, -1, -1, -1);
        S9xSetController(1, CTL_NONE, -1, -1, -1, -1);
        S9xVerifyControllers();
        Memory.LoadSRAM(current->battery.c_str());
        S9xSetSoundMute(false);
        return current;
    } catch (...) {
        if (current) oe_destroy(current);
        return nullptr;
    }
}

void oe_flush(void* p) {
    auto& c = *static_cast<LinuxSNES*>(p);
    if (!c.loaded || !Memory.SRAMSize) return;
    std::string temp = c.battery + ".tmp";
    if (Memory.SaveSRAM(temp.c_str())) {
        if (std::rename(temp.c_str(), c.battery.c_str()))
            std::fprintf(stderr, "Could not replace SNES battery save: %s\n", c.battery.c_str());
    } else {
        std::fprintf(stderr, "Could not write SNES battery save: %s\n", temp.c_str());
    }
}
void oe_destroy(void* p) {
    auto* c = static_cast<LinuxSNES*>(p);
    if (!c || c != current) return;
    oe_flush(c);
    S9xSetSamplesAvailableCallback(nullptr, nullptr);
    S9xUnmapAllControls();
    if (c->graphics) S9xGraphicsDeinit();
    if (c->apu) S9xDeinitAPU();
    if (c->memory) Memory.Deinit();
    current = nullptr;
    delete c;
}
void oe_reset(void*) { S9xSoftReset(); S9xClearSamples(); }
int oe_frame(void* p, unsigned keys) {
    auto& c = *static_cast<LinuxSNES*>(p);
    c.audio.clear();
    c.audio_error = false;
    for (unsigned i = 0; i < 12; ++i) S9xReportButton(i + 1, (keys & (1u << i)) != 0);
    IPPU.RenderThisFrame = true;
    S9xMainLoop();
    collect_audio(&c);
    if (c.audio_error) return -1;
    // GFX uses a padded 512-pixel stride. Copy only the active image, which can
    // switch between 256/512 columns and normal/overscan/interlaced heights.
    for (unsigned y = 0; y < c.height; ++y) {
        for (unsigned x = 0; x < c.width; ++x) {
            uint16 v = GFX.Screen[y * GFX.RealPPL + x];
            unsigned r = (v >> 11) & 31, g = (v >> 5) & 63, b = v & 31;
            c.pixels[y * c.width + x] = ((r * 255 / 31) << 16) | ((g * 255 / 63) << 8) | (b * 255 / 31);
        }
    }
    return static_cast<int>(c.audio.size() / 2);
}
const uint32_t* oe_pixels(void* p) { return static_cast<LinuxSNES*>(p)->pixels.data(); }
const int16_t* oe_audio(void* p) { return static_cast<LinuxSNES*>(p)->audio.data(); }
unsigned oe_width(void* p) { return static_cast<LinuxSNES*>(p)->width; }
unsigned oe_height(void* p) { return static_cast<LinuxSNES*>(p)->height; }
double oe_fps(void*) { return Settings.PAL ? 50.0 : 60.098; }
unsigned oe_sample_rate(void*) { return 48000; }
int oe_save_state(void*, const char* path) {
    uint32 size = S9xFreezeSize();
    std::vector<uint8> state(size);
    if (!S9xFreezeGameMem(state.data(), size)) return 0;
    FILE* f = std::fopen(path, "wb");
    if (!f) return 0;
    bool ok = std::fwrite(state.data(), 1, size, f) == size;
    return std::fclose(f) == 0 && ok;
}
int oe_load_state(void*, const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (!f) return 0;
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::rewind(f);
    if (size <= 0 || size > 32 * 1024 * 1024) { std::fclose(f); return 0; }
    std::vector<uint8> state(size);
    bool ok = std::fread(state.data(), 1, size, f) == static_cast<size_t>(size);
    std::fclose(f);
    if (!ok || S9xUnfreezeGameMem(state.data(), size) != SUCCESS) return 0;
    S9xClearSamples();
    return 1;
}

bool8 S9xOpenSnapshotFile(const char* name, bool8 read_only, STREAM* file) {
    *file = OPEN_STREAM(name, read_only ? "rb" : "wb");
    return *file != nullptr;
}
void S9xCloseSnapshotFile(STREAM file) { CLOSE_STREAM(file); }
void S9xMessage(int, int, const char* message) { std::fprintf(stderr, "Snes9x: %s\n", message); }
std::string S9xGetDirectory(enum s9x_getdirtype) { return current ? current->directory : "."; }
void S9xAutoSaveSRAM() { if (current) oe_flush(current); }
void S9xSyncSpeed() { IPPU.RenderThisFrame = true; }
bool8 S9xOpenSoundDevice() { return true; }
bool8 S9xInitUpdate() { return true; }
bool8 S9xDeinitUpdate(int width, int height) {
    if (!current || width < 1 || height < 1 || width > MAX_SNES_WIDTH || height > MAX_SNES_HEIGHT) return false;
    current->width = width;
    current->height = height;
    return true;
}
bool8 S9xContinueUpdate(int width, int height) { return S9xDeinitUpdate(width, height); }
const char* S9xStringInput(const char*) { return nullptr; }
std::string S9xGetFilenameInc(std::string, enum s9x_getdirtype) { return ""; }
bool S9xPollButton(uint32, bool* pressed) { *pressed = false; return false; }
bool S9xPollPointer(uint32, int16* x, int16* y) { *x = *y = 0; return false; }
bool S9xPollAxis(uint32, int16* value) { *value = 0; return false; }
bool8 S9xDoScreenshot(int, int) { return false; }
void S9xToggleSoundChannel(int) {}
void S9xExit() {}
void S9xHandlePortCommand(s9xcommand_t, int16, int16) {}
void S9xExtraUsage() {}
void S9xParseArg(char**, int&, int) {}
void S9xParsePortConfig(ConfigFile&, int) {}
