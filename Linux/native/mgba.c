/* SPDX-License-Identifier: MPL-2.0
 * Adapted from OpenEmu's mGBAGameCore.m (Jeffrey Pfau, 2016).
 * Original attribution and BSD notice: ../THIRD_PARTY_NOTICES.md.
 */
#include "core.h"
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba/gba/core.h>
#include <mgba-util/vfs.h>

const char* const binaryName = "OpenEmu-Linux";
const char* const projectName = "OpenEmu-Linux";
const char* projectVersion = "0.1.0";

struct LinuxGBA {
    struct mCore* core;
    uint32_t video[240 * 160];
    uint32_t rgb[240 * 160];
    int16_t audio[4096 * 2];
};

void* oe_create(const char* rom, const char* save_dir) {
    struct LinuxGBA* c = calloc(1, sizeof(*c));
    if (!c) return NULL;
    c->core = GBACoreCreate();
    if (!c->core) { free(c); return NULL; }
    mCoreInitConfig(c->core, NULL);
    struct mCoreOptions opts = { .useBios = false, .volume = 0x100 };
    mCoreConfigLoadDefaults(&c->core->config, &opts);
    if (!c->core->init(c->core)) { oe_destroy(c); return NULL; }
    c->core->setVideoBuffer(c->core, c->video, 240);
    c->core->setAudioBufferSize(c->core, 4096);
    c->core->dirs.save = VDirOpen(save_dir);
    if (!c->core->dirs.save || !mCoreLoadFile(c->core, rom)) { oe_destroy(c); return NULL; }
    mCoreAutoloadSave(c->core);
    c->core->reset(c->core);
    blip_set_rates(c->core->getAudioChannel(c->core, 0), c->core->frequency(c->core), 48000);
    blip_set_rates(c->core->getAudioChannel(c->core, 1), c->core->frequency(c->core), 48000);
    return c;
}
void oe_destroy(void* p) {
    struct LinuxGBA* c = p;
    if (!c) return;
    mCoreConfigDeinit(&c->core->config);
    c->core->deinit(c->core);
    free(c);
}
void oe_reset(void* p) { struct LinuxGBA* c = p; c->core->reset(c->core); }
int oe_frame(void* p, unsigned keys) {
    struct LinuxGBA* c = p;
    c->core->setKeys(c->core, keys & 1023);
    c->core->runFrame(c->core);
    // mGBA supplies native ABGR; expose XRGB consistently with Gambatte.
    for (size_t i = 0; i < 240 * 160; ++i) {
        uint32_t v = c->video[i];
        c->rgb[i] = ((v & 255) << 16) | (v & 0xFF00) | ((v >> 16) & 255);
    }
    int n = blip_samples_avail(c->core->getAudioChannel(c->core, 0));
    if (n > 4096) n = 4096;
    blip_read_samples(c->core->getAudioChannel(c->core, 0), c->audio, n, true);
    blip_read_samples(c->core->getAudioChannel(c->core, 1), c->audio + 1, n, true);
    return n;
}
const uint32_t* oe_pixels(void* p) { return ((struct LinuxGBA*)p)->rgb; }
const int16_t* oe_audio(void* p) { return ((struct LinuxGBA*)p)->audio; }
unsigned oe_width(void* p) { (void)p; return 240; }
unsigned oe_height(void* p) { (void)p; return 160; }
double oe_fps(void* p) { struct LinuxGBA* c = p; return (double)c->core->frequency(c->core) / c->core->frameCycles(c->core); }
unsigned oe_sample_rate(void* p) { (void)p; return 48000; }
int oe_save_state(void* p, const char* path) {
    struct VFile* vf = VFileOpen(path, O_CREAT | O_TRUNC | O_RDWR);
    if (!vf) return 0;
    int ok = mCoreSaveStateNamed(((struct LinuxGBA*)p)->core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
    vf->close(vf);
    return ok;
}
int oe_load_state(void* p, const char* path) {
    struct VFile* vf = VFileOpen(path, O_RDONLY);
    if (!vf) return 0;
    int ok = mCoreLoadStateNamed(((struct LinuxGBA*)p)->core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
    vf->close(vf);
    return ok;
}
void oe_flush(void* p) {
    // mGBA owns a mapped battery-save file and flushes it during emulation and deinit.
    (void)p;
}
