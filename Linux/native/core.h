/* SPDX-License-Identifier: MIT
 * Linux replacement for OpenEmu's OEGameCore platform boundary.
 * Buffers belong to the instance; the caller copies them before the next frame.
 * Each instance must be used from only one thread. Keys use GBA bit order,
 * extended with bit 10 for SNES X and bit 11 for SNES Y. Width and height can
 * change after oe_frame. Snes9x permits one instance per helper process.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void* oe_create(const char* rom, const char* save_dir);
void oe_destroy(void* instance);
void oe_reset(void* instance);
int oe_frame(void* instance, unsigned keys);
const uint32_t* oe_pixels(void* instance);
const int16_t* oe_audio(void* instance);
unsigned oe_width(void* instance);
unsigned oe_height(void* instance);
double oe_fps(void* instance);
unsigned oe_sample_rate(void* instance);
int oe_save_state(void* instance, const char* path);
int oe_load_state(void* instance, const char* path);
void oe_flush(void* instance);
#ifdef __cplusplus
}
#endif
