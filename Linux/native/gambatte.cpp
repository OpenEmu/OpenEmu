/* SPDX-License-Identifier: GPL-2.0-only
 * Adapted from OpenEmu's Gambatte/GBGameCore.mm (OpenEmu Team, 2015).
 * Original attribution and BSD notice: ../THIRD_PARTY_NOTICES.md.
 */
#include "core.h"
#include "gambatte.h"
#include "resampler.h"
#include "resamplerinfo.h"
#include <memory>
#include <vector>

struct LinuxGB : gambatte::InputGetter {
    gambatte::GB gb;
    unsigned keys = 0;
    uint32_t video[160 * 144] = {};
    uint32_t input_audio[4128] = {};
    std::unique_ptr<Resampler> resampler;
    std::vector<int16_t> audio;
    unsigned operator()() override { return keys; }
    LinuxGB() : resampler(ResamplerInfo::get(2).create(2097152, 48000, 4128)) {
        gb.setInputGetter(this);
        const unsigned palette[] = {0xE0F8CF, 0x86C06C, 0x306850, 0x071821};
        for (int p = 0; p < 3; ++p)
            for (int i = 0; i < 4; ++i) gb.setDmgPaletteColor(p, i, palette[i]);
    }
};

void* oe_create(const char* rom, const char* save_dir) {
    try {
        std::unique_ptr<LinuxGB> c(new LinuxGB);
        c->gb.setSaveDir(save_dir);
        if (c->gb.load(rom) != 0) return nullptr;
        return c.release();
    } catch (...) { return nullptr; }
}
void oe_destroy(void* p) { if (p) { oe_flush(p); delete static_cast<LinuxGB*>(p); } }
void oe_reset(void* p) { static_cast<LinuxGB*>(p)->gb.reset(); }
int oe_frame(void* p, unsigned keys) {
    auto& c = *static_cast<LinuxGB*>(p);
    c.keys = keys & 255;
    c.audio.clear();
    // Gambatte may overshoot each request by 2064 samples, as in GBGameCore.mm.
    for (int attempts = 0; attempts < 128; ++attempts) {
        size_t count = 2064;
        auto frame = c.gb.runFor(c.video, 160, c.input_audio, count);
        std::vector<int16_t> out(c.resampler->maxOut(count) * 2);
        auto n = c.resampler->resample(out.data(), reinterpret_cast<int16_t*>(c.input_audio), count);
        c.audio.insert(c.audio.end(), out.begin(), out.begin() + n * 2);
        if (frame >= 0) return static_cast<int>(c.audio.size() / 2);
    }
    return -1;
}
const uint32_t* oe_pixels(void* p) { return static_cast<LinuxGB*>(p)->video; }
const int16_t* oe_audio(void* p) { return static_cast<LinuxGB*>(p)->audio.data(); }
unsigned oe_width(void*) { return 160; }
unsigned oe_height(void*) { return 144; }
double oe_fps(void*) { return 4194304.0 / 70224.0; }
unsigned oe_sample_rate(void*) { return 48000; }
int oe_save_state(void* p, const char* path) { return static_cast<LinuxGB*>(p)->gb.saveState(nullptr, 0, path); }
int oe_load_state(void* p, const char* path) { return static_cast<LinuxGB*>(p)->gb.loadState(path); }
void oe_flush(void* p) { static_cast<LinuxGB*>(p)->gb.saveSavedata(); }
