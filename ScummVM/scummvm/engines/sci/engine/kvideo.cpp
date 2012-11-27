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

#include "engines/util.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#include "video/avi_decoder.h"
#include "video/qt_decoder.h"
#include "sci/video/seq_decoder.h"
#ifdef ENABLE_SCI32
#include "video/coktel_decoder.h"
#include "sci/video/robot_decoder.h"
#endif

namespace Sci {

void playVideo(Video::VideoDecoder *videoDecoder, VideoState videoState) {
	if (!videoDecoder)
		return;

	videoDecoder->start();

	byte *scaleBuffer = 0;
	byte bytesPerPixel = videoDecoder->getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 pitch = videoDecoder->getWidth() * bytesPerPixel;
	uint16 screenWidth = g_sci->_gfxScreen->getDisplayWidth();
	uint16 screenHeight = g_sci->_gfxScreen->getDisplayHeight();

	videoState.fileName.toLowercase();
	bool isVMD = videoState.fileName.hasSuffix(".vmd");

	if (screenWidth == 640 && width <= 320 && height <= 240 && ((videoState.flags & kDoubled) || !isVMD)) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		scaleBuffer = new byte[width * height * bytesPerPixel];
	}

	uint16 x, y;

	// Sanity check...
	if (videoState.x > 0 && videoState.y > 0 && isVMD) {
		x = videoState.x;
		y = videoState.y;

		if (x + width > screenWidth || y + height > screenHeight) {
			// Happens in the Lighthouse demo
			warning("VMD video won't fit on screen, centering it instead");
			x = (screenWidth - width) / 2;
			y = (screenHeight - height) / 2;
		}
	} else {
		x = (screenWidth - width) / 2;
		y = (screenHeight - height) / 2;
	}

	bool skipVideo = false;
	EngineState *s = g_sci->getEngineState();

	if (videoDecoder->hasDirtyPalette()) {
		const byte *palette = videoDecoder->getPalette() + s->_vmdPalStart * 3;
		g_system->getPaletteManager()->setPalette(palette, s->_vmdPalStart, s->_vmdPalEnd - s->_vmdPalStart);
	}

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				if (scaleBuffer) {
					// TODO: Probably should do aspect ratio correction in e.g. GK1 Windows
					g_sci->_gfxScreen->scale2x((byte *)frame->pixels, scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight(), bytesPerPixel);
					g_system->copyRectToScreen(scaleBuffer, pitch, x, y, width, height);
				} else {
					g_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, width, height);
				}

				if (videoDecoder->hasDirtyPalette()) {
					const byte *palette = videoDecoder->getPalette() + s->_vmdPalStart * 3;
					g_system->getPaletteManager()->setPalette(palette, s->_vmdPalStart, s->_vmdPalEnd - s->_vmdPalStart);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	delete[] scaleBuffer;
	delete videoDecoder;
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	// Hide the cursor if it's showing and then show it again if it was
	// previously visible.
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	if (reshowCursor)
		g_sci->_gfxCursor->kernelHide();

	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();

	Video::VideoDecoder *videoDecoder = 0;

	if (argv[0].getSegment() != 0) {
		Common::String filename = s->_segMan->getString(argv[0]);

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime
			// The only argument is the string for the video

			// HACK: Switch to 16bpp graphics for Cinepak.
			initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				warning("This video requires >8bpp color to be displayed, but could not switch to RGB color mode");
				return NULL_REG;
			}

			videoDecoder = new Video::QuickTimeDecoder();
			if (!videoDecoder->loadFile(filename))
				error("Could not open '%s'", filename.c_str());
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			// Time is specified as ticks
			videoDecoder = new SEQDecoder(argv[1].toUint16());

			if (!videoDecoder->loadFile(filename)) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			}
		}
	} else {
		// Windows AVI
		// TODO: This appears to be some sort of subop. case 0 contains the string
		// for the video, so we'll just play it from there for now.

#ifdef ENABLE_SCI32
		if (getSciVersion() >= SCI_VERSION_2_1) {
			// SCI2.1 always has argv[0] as 1, the rest of the arguments seem to
			// follow SCI1.1/2.
			if (argv[0].toUint16() != 1)
				error("SCI2.1 kShowMovie argv[0] not 1");
			argv++;
			argc--;
		}
#endif
		switch (argv[0].toUint16()) {
		case 0: {
			Common::String filename = s->_segMan->getString(argv[1]);
			videoDecoder = new Video::AVIDecoder();

			if (filename.equalsIgnoreCase("gk2a.avi")) {
				// HACK: Switch to 16bpp graphics for Indeo3.
				// The only known movie to do use this codec is the GK2 demo trailer
				// If another video turns up that uses Indeo, we may have to add a better
				// check.
				initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

				if (g_system->getScreenFormat().bytesPerPixel == 1) {
					warning("This video requires >8bpp color to be displayed, but could not switch to RGB color mode");
					return NULL_REG;
				}
			}

			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			} else {
				s->_videoState.fileName = filename;
			}
			break;
		}
		default:
			warning("Unhandled SCI kShowMovie subop %d", argv[0].toUint16());
		}
	}

	if (videoDecoder) {
		playVideo(videoDecoder, s->_videoState);

		// HACK: Switch back to 8bpp if we played a true color video.
		// We also won't be copying the screen to the SCI screen...
		if (g_system->getScreenFormat().bytesPerPixel != 1)
			initGraphics(screenWidth, screenHeight, screenWidth > 320);
		else {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette->kernelSyncScreenPalette();
		}
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return s->r_acc;
}

#ifdef ENABLE_SCI32

reg_t kRobot(EngineState *s, int argc, reg_t *argv) {
	int16 subop = argv[0].toUint16();

	switch (subop) {
	case 0: { // init
		int id = argv[1].toUint16();
		reg_t obj = argv[2];
		int16 flag = argv[3].toSint16();
		int16 x = argv[4].toUint16();
		int16 y = argv[5].toUint16();
		warning("kRobot(init), id %d, obj %04x:%04x, flag %d, x=%d, y=%d", id, PRINT_REG(obj), flag, x, y);
		g_sci->_robotDecoder->load(id);
		g_sci->_robotDecoder->start();
		g_sci->_robotDecoder->setPos(x, y);
		}
		break;
	case 1:	// LSL6 hires (startup)
		// TODO
		return NULL_REG;	// an integer is expected
	case 4: {	// start - we don't really have a use for this one
			//int id = argv[1].toUint16();
			//warning("kRobot(start), id %d", id);
		}
		break;
	case 7:	// unknown, called e.g. by Phantasmagoria
		warning("kRobot(%d)", subop);
		break;
	case 8: // sync
		//if (true) {	// debug: automatically skip all robot videos
		if (g_sci->_robotDecoder->endOfVideo()) {
			g_sci->_robotDecoder->close();
			// Signal the engine scripts that the video is done
			writeSelector(s->_segMan, argv[1], SELECTOR(signal), SIGNAL_REG);
		} else {
			writeSelector(s->_segMan, argv[1], SELECTOR(signal), NULL_REG);
		}
		break;
	default:
		warning("kRobot(%d)", subop);
		break;
	}

	return s->r_acc;
}

reg_t kPlayVMD(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Video::VideoDecoder *videoDecoder = 0;
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	Common::String warningMsg;

	switch (operation) {
	case 0:	// init
		s->_videoState.reset();
		s->_videoState.fileName = s->_segMan->derefString(argv[1]);

		if (argc > 2 && argv[2] != NULL_REG)
			warning("kPlayVMD: third parameter isn't 0 (it's %04x:%04x - %s)", PRINT_REG(argv[2]), s->_segMan->getObjectName(argv[2]));
		break;
	case 1:
	{
		// Set VMD parameters. Called with a maximum of 6 parameters:
		//
		// x, y, flags, gammaBoost, gammaFirst, gammaLast
		//
		// gammaBoost boosts palette colors in the range gammaFirst to
		// gammaLast, but only if bit 4 in flags is set. Percent value such that
		// 0% = no amplification These three parameters are optional if bit 4 is
		// clear. Also note that the x, y parameters play subtle games if used
		// with subfx 21. The subtleness has to do with creation of temporary
		// planes and positioning relative to such planes.

		uint16 flags = argv[3].getOffset();
		Common::String flagspec;

		if (argc > 3) {
			if (flags & kDoubled)
				flagspec += "doubled ";
			if (flags & kDropFrames)
				flagspec += "dropframes ";
			if (flags & kBlackLines)
				flagspec += "blacklines ";
			if (flags & kUnkBit3)
				flagspec += "bit3 ";
			if (flags & kGammaBoost)
				flagspec += "gammaboost ";
			if (flags & kHoldBlackFrame)
				flagspec += "holdblack ";
			if (flags & kHoldLastFrame)
				flagspec += "holdlast ";
			if (flags & kUnkBit7)
				flagspec += "bit7 ";
			if (flags & kStretch)
				flagspec += "stretch";

			warning("VMDFlags: %s", flagspec.c_str());

			s->_videoState.flags = flags;
		}

		warning("x, y: %d, %d", argv[1].getOffset(), argv[2].getOffset());
		s->_videoState.x = argv[1].getOffset();
		s->_videoState.y = argv[2].getOffset();

		if (argc > 4 && flags & 16)
			warning("gammaBoost: %d%% between palette entries %d and %d", argv[4].getOffset(), argv[5].getOffset(), argv[6].getOffset());
		break;
	}
	case 6:	// Play
		videoDecoder = new Video::AdvancedVMDDecoder();

		if (s->_videoState.fileName.empty()) {
			// Happens in Lighthouse
			warning("kPlayVMD: Empty filename passed");
			return s->r_acc;
		}

		if (!videoDecoder->loadFile(s->_videoState.fileName)) {
			warning("Could not open VMD %s", s->_videoState.fileName.c_str());
			break;
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelHide();

		playVideo(videoDecoder, s->_videoState);

		if (reshowCursor)
			g_sci->_gfxCursor->kernelShow();
		break;
	case 23:	// set video palette range
		s->_vmdPalStart = argv[1].toUint16();
		s->_vmdPalEnd = argv[2].toUint16();
		break;
	case 14:
		// Takes an additional integer parameter (e.g. 3)
	case 16:
		// Takes an additional parameter, usually 0
	case 21:
		// Looks to be setting the video size and position. Called with 4 extra integer
		// parameters (e.g. 86, 41, 235, 106)
	default:
		warningMsg = Common::String::format("PlayVMD - unsupported subop %d. Params: %d (", operation, argc);

		for (int i = 0; i < argc; i++) {
			warningMsg +=  Common::String::format("%04x:%04x", PRINT_REG(argv[i]));
			warningMsg += (i == argc - 1 ? ")" : ", ");
		}

		warning("%s", warningMsg.c_str());
		break;
	}

	return s->r_acc;
}

reg_t kPlayDuck(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Video::VideoDecoder *videoDecoder = 0;
	bool reshowCursor = g_sci->_gfxCursor->isVisible();

	switch (operation) {
	case 1:	// Play
		// 6 params
		s->_videoState.reset();
		s->_videoState.fileName = Common::String::format("%d.duk", argv[1].toUint16());

		videoDecoder = new Video::AVIDecoder();

		if (!videoDecoder->loadFile(s->_videoState.fileName)) {
			warning("Could not open Duck %s", s->_videoState.fileName.c_str());
			break;
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelHide();

		{
		// Duck videos are 16bpp, so we need to change the active pixel format
		int oldWidth = g_system->getWidth();
		int oldHeight = g_system->getHeight();
		Common::List<Graphics::PixelFormat> formats;
		formats.push_back(videoDecoder->getPixelFormat());
		initGraphics(640, 480, true, formats);

		if (g_system->getScreenFormat().bytesPerPixel != videoDecoder->getPixelFormat().bytesPerPixel)
			error("Could not switch screen format for the duck video");

		playVideo(videoDecoder, s->_videoState);

		// Switch back to 8bpp
		initGraphics(oldWidth, oldHeight, oldWidth > 320);
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelShow();
		break;
	default:
		kStub(s, argc, argv);
		break;
	}

	return s->r_acc;
}

#endif

} // End of namespace Sci
