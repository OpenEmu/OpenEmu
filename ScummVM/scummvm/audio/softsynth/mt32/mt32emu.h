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

#ifndef MT32EMU_MT32EMU_H
#define MT32EMU_MT32EMU_H

// Debugging

// 0: Standard debug output is not stamped with the rendered sample count
// 1: Standard debug output is stamped with the rendered sample count
// NOTE: The "samplestamp" corresponds to the end of the last completed rendering run.
//       This is important to bear in mind for debug output that occurs during a run.
#define MT32EMU_DEBUG_SAMPLESTAMPS 0

// 0: No debug output for initialisation progress
// 1: Debug output for initialisation progress
#define MT32EMU_MONITOR_INIT 0

// 0: No debug output for MIDI events
// 1: Debug output for weird MIDI events
#define MT32EMU_MONITOR_MIDI 0

// 0: No debug output for note on/off
// 1: Basic debug output for note on/off
// 2: Comprehensive debug output for note on/off
#define MT32EMU_MONITOR_INSTRUMENTS 0

// 0: No debug output for partial allocations
// 1: Show partial stats when an allocation fails
// 2: Show partial stats with every new poly
// 3: Show individual partial allocations/deactivations
#define MT32EMU_MONITOR_PARTIALS 0

// 0: No debug output for sysex
// 1: Basic debug output for sysex
#define MT32EMU_MONITOR_SYSEX 0

// 0: No debug output for sysex writes to the timbre areas
// 1: Debug output with the name and location of newly-written timbres
// 2: Complete dump of timbre parameters for newly-written timbres
#define MT32EMU_MONITOR_TIMBRES 0

// 0: No TVA/TVF-related debug output.
// 1: Shows changes to TVA/TVF target, increment and phase.
#define MT32EMU_MONITOR_TVA 0
#define MT32EMU_MONITOR_TVF 0

// The WG algorithm involves dozens of transcendent maths, e.g. exponents and trigonometry.
// Unfortunately, the majority of systems perform such computations inefficiently,
// standard math libs and FPUs make no optimisations for single precision floats,
// and use no LUTs to speedup computing internal taylor series. Though, there're rare exceptions,
// and there's a hope it will become common soon.
// So, this is the crucial point of speed optimisations. We have now eliminated all the transcendent maths
// within the critical path and use LUTs instead.
// Besides, since the LA32 chip is assumed to use similar LUTs inside, the overall emulation accuracy should be better.
// 0: Use LUTs to speedup WG. Most common setting. You can expect about 50% performance boost.
// 1: Use precise float math. Use this setting to achieve more accurate wave generator. If your system performs better with this setting, it is really notable. :)
#define MT32EMU_ACCURATE_WG 0

#define MT32EMU_USE_EXTINT 0

// Configuration
// The maximum number of partials playing simultaneously
#define MT32EMU_MAX_PARTIALS 32
// The maximum number of notes playing simultaneously per part.
// No point making it more than MT32EMU_MAX_PARTIALS, since each note needs at least one partial.
#define MT32EMU_MAX_POLY 32

// If non-zero, deletes reverb buffers that are not in use to save memory.
// If zero, keeps reverb buffers for all modes around all the time to avoid allocating/freeing in the critical path.
#define MT32EMU_REDUCE_REVERB_MEMORY 1

// 0: Use standard Freeverb
// 1: Use AReverb (currently not properly tuned)
#define MT32EMU_USE_AREVERBMODEL 0

namespace MT32Emu
{
// The higher this number, the more memory will be used, but the more samples can be processed in one run -
// various parts of sample generation can be processed more efficiently in a single run.
// A run's maximum length is that given to Synth::render(), so giving a value here higher than render() is ever
// called with will give no gain (but simply waste the memory).
// Note that this value does *not* in any way impose limitations on the length given to render(), and has no effect
// on the generated audio.
// This value must be >= 1.
const unsigned int MAX_SAMPLES_PER_RUN = 4096;

// This determines the amount of memory available for simulating delays.
// If set too low, partials aborted to allow other partials to play will not end gracefully, but will terminate
// abruptly and potentially cause a pop/crackle in the audio output.
// This value must be >= 1.
const unsigned int MAX_PRERENDER_SAMPLES = 1024;
}

#include "Structures.h"
#include "common/file.h"
#include "Tables.h"
#include "Poly.h"
#include "LA32Ramp.h"
#include "TVA.h"
#include "TVP.h"
#include "TVF.h"
#include "Partial.h"
#include "Part.h"
#include "Synth.h"

#endif
