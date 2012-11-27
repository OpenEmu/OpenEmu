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

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/list.h" // For OSystem::getSupportedFormats()
#include "graphics/pixelformat.h"

namespace Audio {
class Mixer;
}

namespace Graphics {
struct Surface;
}

namespace Common {
class EventManager;
struct Rect;
class SaveFileManager;
class SearchSet;
class String;
#if defined(USE_TASKBAR)
class TaskbarManager;
#endif
#if defined(USE_UPDATES)
class UpdateManager;
#endif
class TimerManager;
class SeekableReadStream;
class WriteStream;
#ifdef ENABLE_KEYMAPPER
class HardwareInputSet;
class Keymap;
class KeymapperDefaultBindings;
#endif
}

class AudioCDManager;
class FilesystemFactory;
class PaletteManager;

/**
 * A structure describing time and date. This is a clone of struct tm
 * from time.h. We roll our own since not all systems provide time.h.
 * We also do not imitate all files of struct tm, only those we
 * actually need.
 *
 * @note For now, the members are named exactly as in struct tm to ease
 * the transition.
 */
struct TimeDate {
	int tm_sec;     ///< seconds (0 - 60)
	int tm_min;     ///< minutes (0 - 59)
	int tm_hour;    ///< hours (0 - 23)
	int tm_mday;    ///< day of month (1 - 31)
	int tm_mon;     ///< month of year (0 - 11)
	int tm_year;    ///< year - 1900
	int tm_wday;    ///< days since Sunday (0 - 6)
};

namespace LogMessageType {

enum Type {
	kInfo,
	kError,
	kWarning,
	kDebug
};

} // End of namespace LogMessageType

/**
 * Interface for ScummVM backends. If you want to port ScummVM to a system
 * which is not currently covered by any of our backends, this is the place
 * to start. ScummVM will create an instance of a subclass of this interface
 * and use it to interact with the system.
 *
 * In particular, a backend provides a video surface for ScummVM to draw in;
 * methods to create timers, to handle user input events,
 * control audio CD playback, and sound output.
 */
class OSystem : Common::NonCopyable {
protected:
	OSystem();
	virtual ~OSystem();

protected:
	/**
	 * @name Module slots
	 *
	 * For backend authors only, the following pointers (= "slots) to various
	 * subsystem managers / factories / etc. can and should be set to
	 * a suitable instance of the respective type.
	 *
	 * For some of the slots, a default instance is set if your backend
	 * does not do so. For details, please look at the documentation of
	 * each slot.
	 *
	 * A backend may setup slot values in its initBackend() method,
	 * its constructor or somewhere in between. But it must a slot's value
	 * no later than in its initBackend() implementation, because
	 * OSystem::initBackend() will create any default instances if
	 * none has been set yet (and for other slots, will verify that
	 * one has been set; if not, an error may be generated).
	 */
	//@{

	/**
	 * No default value is provided for _audiocdManager by OSystem.
	 * However, BaseBackend::initBackend() does set a default value
	 * if none has been set before.
	 *
	 * @note _audiocdManager is deleted by the OSystem destructor.
	 */
	AudioCDManager *_audiocdManager;

	/**
	 * No default value is provided for _eventManager by OSystem.
	 * However, BaseBackend::initBackend() does set a default value
	 * if none has been set before.
	 *
	 * @note _eventManager is deleted by the OSystem destructor.
	 */
	Common::EventManager *_eventManager;

	/**
	 * No default value is provided for _timerManager by OSystem.
	 *
	 * @note _timerManager is deleted by the OSystem destructor.
	 */
	Common::TimerManager *_timerManager;

	/**
	 * No default value is provided for _savefileManager by OSystem.
	 *
	 * @note _savefileManager is deleted by the OSystem destructor.
	 */
	Common::SaveFileManager *_savefileManager;

#if defined(USE_TASKBAR)
	/**
	 * No default value is provided for _taskbarManager by OSystem.
	 *
	 * @note _taskbarManager is deleted by the OSystem destructor.
	 */
	Common::TaskbarManager *_taskbarManager;
#endif

#if defined(USE_UPDATES)
	/**
	 * No default value is provided for _updateManager by OSystem.
	 *
	 * @note _updateManager is deleted by the OSystem destructor.
	 */
	Common::UpdateManager *_updateManager;
#endif

	/**
	 * No default value is provided for _fsFactory by OSystem.
	 *
	 * Note that _fsFactory is typically required very early on,
	 * so it usually should be set in the backends constructor or shortly
	 * thereafter, and before initBackend() is called.
	 *
	 * @note _fsFactory is deleted by the OSystem destructor.
	 */
	FilesystemFactory *_fsFactory;

	//@}

public:

	/**
	 * The following method is called once, from main.cpp, after all
	 * config data (including command line params etc.) are fully loaded.
	 *
	 * @note Subclasses should always invoke the implementation of their
	 *       parent class. They should do so near the end of their own
	 *       implementation.
	 */
	virtual void initBackend();

	/**
	 * Allows the backend to perform engine specific init.
	 * Called just before the engine is run.
	 */
	virtual void engineInit() { }

	/**
	 * Allows the backend to perform engine specific de-init.
	 * Called after the engine finishes.
	 */
	virtual void engineDone() { }

	/** @name Feature flags */
	//@{

	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off. Examples include:
	 *  - fullscreen mode
	 *  - aspect ration correction
	 *  - a virtual keyboard for text entry (on PDAs)
	 *
	 * One has to distinguish between the *availability* of a feature,
	 * which can be checked using hasFeature(), and its *state*.
	 * For example, the SDL backend *has* the kFeatureFullscreenMode,
	 * so hasFeature returns true for it. On the other hand,
	 * fullscreen mode may be active or not; this can be determined
	 * by checking the state via getFeatureState(). Finally, to
	 * switch between fullscreen and windowed mode, use setFeatureState().
	 */
	enum Feature {
		/**
		 * If supported, this feature flag can be used to switch between
		 * windowed and fullscreen mode.
		 */
		kFeatureFullscreenMode,

		/**
		 * Control aspect ratio correction. Aspect ratio correction is used to
		 * correct games running at 320x200 (i.e with an aspect ratio of 8:5),
		 * but which on their original hardware were displayed with the
		 * standard 4:3 ratio (that is, the original graphics used non-square
		 * pixels). When the backend support this, then games running at
		 * 320x200 pixels should be scaled up to 320x240 pixels. For all other
		 * resolutions, ignore this feature flag.
		 * @note Backend implementors can find utility functions in common/scaler.h
		 *       which can be used to implement aspect ratio correction. In
		 *       stretch200To240() can stretch a rect, including (very fast)
		 *       particular, interpolation, and works in-place.
		 */
		kFeatureAspectRatioCorrection,

		/**
		 * Determine whether a virtual keyboard is too be shown or not.
		 * This would mostly be implemented by backends for hand held devices,
		 * like PocketPC, Palms, Symbian phones like the P800, Zaurus, etc.
		 */
		kFeatureVirtualKeyboard,

		/**
		 * Backends supporting this feature allow specifying a custom palette
		 * for the cursor. The custom palette is used if the feature state
		 * is set to true by the client code via setFeatureState().
		 *
		 * It is currently used only by some Macintosh versions of Humongous
		 * Entertainment games. If the backend doesn't implement this feature
		 * then the engine switches to b/w versions of cursors.
		 * The GUI also relies on this feature for mouse cursors.
		 */
		kFeatureCursorPalette,

		/**
		 * A backend have this feature if its overlay pixel format has an alpha
		 * channel which offers at least 3-4 bits of accuracy (as opposed to
		 * just a single alpha bit).
		 *
		 * This feature has no associated state.
		 */
		kFeatureOverlaySupportsAlpha,

		/**
		 * Client code can set the state of this feature to true in order to
		 * iconify the application window.
		 */
		kFeatureIconifyWindow,

		/**
		 * Setting the state of this feature to true tells the backend to disable
		 * all key filtering/mapping, in cases where it would be beneficial to do so.
		 * As an example case, this is used in the AGI engine's predictive dialog.
		 * When the dialog is displayed this feature is set so that backends with
		 * phone-like keypad temporarily unmap all user actions which leads to
		 * comfortable word entry. Conversely, when the dialog exits the feature
		 * is set to false.
		 *
		 * TODO: The word 'beneficial' above is very unclear. Beneficial to
		 * whom and for what??? Just giving an example is not enough.
		 *
		 * TODO: Fingolfin suggests that the way the feature is used can be
		 * generalized in this sense: Have a keyboard mapping feature, which the
		 * engine queries for to assign keys to actions ("Here's my default key
		 * map for these actions, what do you want them set to?").
		 */
		kFeatureDisableKeyFiltering,

		/**
		 * The presence of this feature indicates whether the displayLogFile()
		 * call is supported.
		 *
		 * This feature has no associated state.
		 */
		kFeatureDisplayLogFile
	};

	/**
	 * Determine whether the backend supports the specified feature.
	 */
	virtual bool hasFeature(Feature f) { return false; }

	/**
	 * En-/disable the specified feature. For example, this may be used to
	 * enable fullscreen mode, or to deactivate aspect correction, etc.
	 */
	virtual void setFeatureState(Feature f, bool enable) {}

	/**
	 * Query the state of the specified feature. For example, test whether
	 * fullscreen mode is active or not.
	 */
	virtual bool getFeatureState(Feature f) { return false; }

	//@}



	/**
	 * @name Graphics
	 *
	 * The way graphics work in the class OSystem are meant to make
	 * it possible for game frontends to implement all they need in
	 * an efficient manner. The downside of this is that it may be
	 * rather complicated for backend authors to fully understand and
	 * implement the semantics of the OSystem interface.
	 *
	 *
	 * The graphics visible to the user in the end are actually
	 * composed in three layers: the game graphics, the overlay
	 * graphics, and the mouse.
	 *
	 * First, there are the game graphics. The methods in this section
	 * deal with them exclusively. In particular, the size of the game
	 * graphics is defined by a call to initSize(), and
	 * copyRectToScreen() blits the data in the current pixel format
	 * into the game layer. Let W and H denote the width and height of
	 * the game graphics.
	 *
	 * Before the user sees these graphics, the backend may apply some
	 * transformations to it; for example, the may be scaled to better
	 * fit on the visible screen; or aspect ratio correction may be
	 * performed (see kFeatureAspectRatioCorrection). As a result of
	 * this, a pixel of the game graphics may occupy a region bigger
	 * than a single pixel on the screen. We define p_w and p_h to be
	 * the width resp. height of a game pixel on the screen.
	 *
	 * In addition, there is a vertical "shake offset" (as defined by
	 * setShakePos) which is used in some games to provide a shaking
	 * effect. Note that shaking is applied to all three layers, i.e.
	 * also to the overlay and the mouse. We denote the shake offset
	 * by S.
	 *
	 * Putting this together, a pixel (x,y) of the game graphics is
	 * transformed to a rectangle of height p_h and width p_w
	 * appearing at position (p_w * x, p_hw * (y + S)) on the real
	 * screen (in addition, a backend may choose to offset
	 * everything, e.g. to center the graphics on the screen).
	 *
	 *
	 * The next layer is the overlay. It is composed over the game
	 * graphics. Historically the overlay size had always been a
	 * multiple of the game resolution, for example when the game
	 * resolution was 320x200 and the user selected a 2x scaler and did
	 * not enable aspect ratio correction it had a size of 640x400.
	 * An exception was the aspect ratio correction, which did allow
	 * for non multiples of the vertical resolution of the game screen.
	 * Nowadays the overlay size does not need to have any relation to
	 * the game resolution though, for example the overlay resolution
	 * might be the same as the physical screen resolution.
	 * The overlay is forced to a 16bpp mode right now.
	 *
	 * Finally, there is the mouse layer. This layer doesn't have to
	 * actually exist within the backend -- it all depends on how a
	 * backend chooses to implement mouse cursors, but in the default
	 * SDL backend, it really is a separate layer. The mouse can
	 * have a palette of its own, if the backend supports it.
	 *
	 * On a note for OSystem users here. We do not require our graphics
	 * to be thread safe and in fact most/all backends using OpenGL are
	 * not. So do *not* try to call any of these functions from a timer
	 * and/or audio callback (like readBuffer of AudioStreams).
	 */
	//@{

	/**
	 * Description of a graphics mode.
	 */
	struct GraphicsMode {
		/**
		 * The 'name' of the graphics mode. This name is matched when selecting
		 * a mode via the command line, or via the config file.
		 * Examples: "1x", "advmame2x", "hq3x"
		 */
		const char *name;
		/**
		 * Human readable description of the scaler.
		 * Examples: "Normal (no scaling)", "AdvMAME2x", "HQ3x"
		 */
		const char *description;
		/**
		 * ID of the graphics mode. How to use this is completely up to the
		 * backend. This value will be passed to the setGraphicsMode(int)
		 * method by client code.
		 */
		int id;
	};

	/**
	 * Retrieve a list of all graphics modes supported by this backend.
	 * This can be both video modes as well as graphic filters/scalers;
	 * it is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 * @return a list of supported graphics modes
	 */
	virtual const GraphicsMode *getSupportedGraphicsModes() const = 0;

	/**
	 * Return the ID of the 'default' graphics mode. What exactly this means
	 * is up to the backend. This mode is set by the client code when no user
	 * overrides are present (i.e. if no custom graphics mode is selected via
	 * the command line or a config file).
	 *
	 * @return the ID of the 'default' graphics mode
	 */
	virtual int getDefaultGraphicsMode() const = 0;

	/**
	 * Switch to the specified graphics mode. If switching to the new mode
	 * failed, this method returns false.
	 *
	 * @param mode	the ID of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 */
	virtual bool setGraphicsMode(int mode) = 0;

	/**
	 * Switch to the graphics mode with the given name. If 'name' is unknown,
	 * or if switching to the new mode failed, this method returns false.
	 *
	 * @param name	the name of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 * @note This is implemented via the setGraphicsMode(int) method, as well
	 *       as getSupportedGraphicsModes() and getDefaultGraphicsMode().
	 *       In particular, backends do not have to overload this!
	 */
	bool setGraphicsMode(const char *name);

	/**
	 * Determine which graphics mode is currently active.
	 * @return the ID of the active graphics mode
	 */
	virtual int getGraphicsMode() const = 0;

	/**
	 * Sets the graphics scale factor to x1. Games with large screen sizes
	 * reset the scale to x1 so the screen will not be too big when starting
	 * the game.
	 */
	virtual void resetGraphicsScale() {}

#ifdef USE_RGB_COLOR
	/**
	 * Determine the pixel format currently in use for screen rendering.
	 * @return the active screen pixel format.
	 * @see Graphics::PixelFormat
	 */
	virtual Graphics::PixelFormat getScreenFormat() const = 0;

	/**
	 * Returns a list of all pixel formats supported by the backend.
	 * The first item in the list must be directly supported by hardware,
	 * and provide the largest color space of those formats with direct
	 * hardware support. It is also strongly recommended that remaining
	 * formats should be placed in order of descending preference for the
	 * backend to use.
	 *
	 * EG: a backend that supports 32-bit ABGR and 16-bit 555 BGR in hardware
	 * and provides conversion from equivalent RGB(A) modes should order its list
	 *    1) Graphics::PixelFormat(4, 0, 0, 0, 0, 0, 8, 16, 24)
	 *    2) Graphics::PixelFormat(2, 3, 3, 3, 8, 0, 5, 10, 0)
	 *    3) Graphics::PixelFormat(4, 0, 0, 0, 0, 24, 16, 8, 0)
	 *    4) Graphics::PixelFormat(2, 3, 3, 3, 8, 10, 5, 0, 0)
	 *    5) Graphics::PixelFormat::createFormatCLUT8()
	 *
	 * @see Graphics::PixelFormat
	 *
	 * @note Backends supporting RGB color should accept game data in RGB color
	 *       order, even if hardware uses BGR or some other color order.
	 */
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#else
	inline Graphics::PixelFormat getScreenFormat() const {
		return Graphics::PixelFormat::createFormatCLUT8();
	};

	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	};
#endif

	/**
	 * Set the size and color format of the virtual screen. Typical sizes include:
	 *  - 320x200 (e.g. for most SCUMM games, and Simon)
	 *  - 320x240 (e.g. for FM-TOWN SCUMM games)
	 *  - 640x480 (e.g. for Curse of Monkey Island)
	 *
	 * This is the resolution for which the client code generates data;
	 * this is not necessarily equal to the actual display size. For example,
	 * a backend may magnify the graphics to fit on screen (see also the
	 * GraphicsMode); stretch the data to perform aspect ratio correction;
	 * or shrink it to fit on small screens (in cell phones).
	 *
	 * Typical formats include:
	 *  CLUT8 (e.g. 256 color, for most games)
	 *  RGB555 (e.g. 16-bit color, for later SCUMM HE games)
	 *  RGB565 (e.g. 16-bit color, for Urban Runner)
	 *
	 * This is the pixel format for which the client code generates data;
	 * this is not necessarily equal to the hardware pixel format. For example,
	 * a backend may perform color lookup of 8-bit graphics before pushing
	 * a screen to hardware, or correct the ARGB color order.
	 *
	 * @param width		the new virtual screen width
	 * @param height	the new virtual screen height
	 * @param format	the new virtual screen pixel format
	 */
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) = 0;

	/**
	 * Return an int value which is changed whenever any screen
	 * parameters (like the resolution) change. That is, whenever a
	 * EVENT_SCREEN_CHANGED would be sent. You can track this value
	 * in your code to detect screen changes in case you do not have
	 * full control over the event loop(s) being used (like the GUI
	 * code).
	 *
	 * @return an integer which can be used to track screen changes
	 *
	 * @note Backends which generate EVENT_SCREEN_CHANGED events MUST
	 *       overload this method appropriately.
	 */
	virtual int getScreenChangeID() const { return 0; }

	/**
	 * Begin a new GFX transaction, which is a sequence of GFX mode changes.
	 * The idea behind GFX transactions is to make it possible to activate
	 * several different GFX changes at once as a "batch" operation. For
	 * example, assume we are running in 320x200 with a 2x scaler (thus using
	 * 640x400 pixels in total). Now, we want to switch to 640x400 with the 1x
	 * scaler. Without transactions, we have to choose whether we want to first
	 * switch the scaler mode, or first to 640x400 mode. In either case,
	 * depending on the backend implementation, some ugliness may result.
	 * E.g. the window might briefly switch to 320x200 or 1280x800.
	 * Using transactions, this can be avoided.
	 *
	 * @note Transaction support is optional, and the default implementations
	 *       of the relevant methods simply do nothing.
	 * @see endGFXTransaction
	 */
	virtual void beginGFXTransaction() {}

	/**
	 * This type is able to save the different errors which can happen while
	 * changing GFX config values inside GFX transactions.
	 *
	 * endGFXTransaction returns a ORed combination of the '*Failed' values
	 * if any problem occures, on success 0.
	 *
	 * @see endGFXTransaction
	 */
	enum TransactionError {
		kTransactionSuccess = 0,					/**< Everything fine (use EQUAL check for this one!) */
		kTransactionAspectRatioFailed = (1 << 0),	/**< Failed switching aspect ratio correction mode */
		kTransactionFullscreenFailed = (1 << 1),	/**< Failed switching fullscreen mode */
		kTransactionModeSwitchFailed = (1 << 2),	/**< Failed switching the GFX graphics mode (setGraphicsMode) */
		kTransactionSizeChangeFailed = (1 << 3),	/**< Failed switching the screen dimensions (initSize) */
		kTransactionFormatNotSupported = (1 << 4)	/**< Failed setting the color format */
	};

	/**
	 * End (and thereby commit) the current GFX transaction.
	 * @see beginGFXTransaction
	 * @see kTransactionError
	 * @return returns a ORed combination of TransactionError values or 0 on success
	 */
	virtual TransactionError endGFXTransaction() { return kTransactionSuccess; }


	/**
	 * Returns the currently set virtual screen height.
	 * @see initSize
	 * @return the currently set virtual screen height
	 */
	virtual int16 getHeight() = 0;

	/**
	 * Returns the currently set virtual screen width.
	 * @see initSize
	 * @return the currently set virtual screen width
	 */
	virtual int16 getWidth() = 0;

	/**
	 * Return the palette manager singleton. For more information, refer
	 * to the PaletteManager documentation.
	 */
	virtual PaletteManager *getPaletteManager() = 0;

	/**
	 * Blit a bitmap to the virtual screen.
	 * The real screen will not immediately be updated to reflect the changes.
	 * Client code has to to call updateScreen to ensure any changes are
	 * visible to the user. This can be used to optimize drawing and reduce
	 * flicker.
	 * If the current pixel format has one byte per pixel, the graphics data
	 * uses 8 bits per pixel, using the palette specified via setPalette.
	 * If more than one byte per pixel is in use, the graphics data uses the
	 * pixel format returned by getScreenFormat.
	 *
	 * @param buf		the buffer containing the graphics data source
	 * @param pitch		the pitch of the buffer (number of bytes in a scanline)
	 * @param x			the x coordinate of the destination rectangle
	 * @param y			the y coordinate of the destination rectangle
	 * @param w			the width of the destination rectangle
	 * @param h			the height of the destination rectangle
	 *
	 * @note The specified destination rectangle must be completly contained
	 *       in the visible screen space, and must be non-empty. If not, a
	 *       backend may or may not perform clipping, trigger an assert or
	 *       silently corrupt memory.
	 *
	 * @see updateScreen
	 * @see getScreenFormat
	 */
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Lock the active screen framebuffer and return a Graphics::Surface
	 * representing it. The caller can then perform arbitrary graphics
	 * transformations on the framebuffer (blitting, scrolling, etc.).
	 * Must be followed by matching call to unlockScreen(). Calling code
	 * should make sure to only lock the framebuffer for the briefest
	 * periods of time possible, as the whole system is potentially stalled
	 * while the lock is active.
	 * Returns 0 if an error occurred. Otherwise a surface with the pixel
	 * format described by getScreenFormat is returned.
	 *
	 * The returned surface must *not* be deleted by the client code.
	 *
	 * @see getScreenFormat
	 */
	virtual Graphics::Surface *lockScreen() = 0;

	/**
	 * Unlock the screen framebuffer, and mark it as dirty (i.e. during the
	 * next updateScreen() call, the whole screen will be updated.
	 */
	virtual void unlockScreen() = 0;

	/**
	 * Fills the screen with a given color value.
	 *
	 * @note We are using uint32 here even though currently
	 * we only support 8bpp indexed mode. Thus the value should
	 * be always inside [0, 255] for now.
	 */
	virtual void fillScreen(uint32 col) = 0;

	/**
	 * Flush the whole screen, that is render the current content of the screen
	 * framebuffer to the display.
	 *
	 * This method could be called very often by engines. Backends are hence
	 * supposed to only perform any redrawing if it is necessary, and otherwise
	 * return immediately. See
	 * <http://wiki.scummvm.org/index.php/HOWTO-Backends#updateScreen.28.29_method>
	 */
	virtual void updateScreen() = 0;

	/**
	 * Set current shake position, a feature needed for some SCUMM screen
	 * effects. The effect causes the displayed graphics to be shifted upwards
	 * by the specified (always positive) offset. The area at the bottom of the
	 * screen which is moved into view by this is filled with black. This does
	 * not cause any graphic data to be lost - that is, to restore the original
	 * view, the game engine only has to call this method again with offset
	 * equal to zero. No calls to copyRectToScreen are necessary.
	 * @param shakeOffset	the shake offset
	 *
	 * @note This is currently used in the SCUMM, QUEEN and KYRA engines.
	 */
	virtual void setShakePos(int shakeOffset) = 0;

	/**
	 * Sets the area of the screen that has the focus.  For example, when a character
	 * is speaking, they will have the focus.  Allows for pan-and-scan style views
	 * where the backend could follow the speaking character or area of interest on
	 * the screen.
	 *
	 * The backend is responsible for clipping the rectangle and deciding how best to
	 * zoom the screen to show any shape and size rectangle the engine provides.
	 *
	 * @param rect A rectangle on the screen to be focused on
	 * @see clearFocusRectangle
	 */
	virtual void setFocusRectangle(const Common::Rect& rect) {}

	/**
	 * Clears the focus set by a call to setFocusRectangle().  This allows the engine
	 * to clear the focus during times when no particular area of the screen has the
	 * focus.
	 * @see setFocusRectangle
	 */
	virtual void clearFocusRectangle() {}

	//@}



	/**
	 * @name Overlay
	 * In order to be able to display dialogs atop the game graphics, backends
	 * must provide an overlay mode.
	 *
	 * The overlay is currently forced at 16 bpp.
	 *
	 * For 'coolness' we usually want to have an overlay which is blended over
	 * the game graphics. On backends which support alpha blending, this is
	 * no issue; but on other systems this needs some trickery.
	 *
	 * Essentially, we fake (alpha) blending on these systems by copying the
	 * current game graphics into the overlay buffer when activating the overlay,
	 * then manually compose whatever graphics we want to show in the overlay.
	 * This works because we assume the game to be "paused" whenever an overlay
	 * is active.
	 */
	//@{

	/** Activate the overlay mode. */
	virtual void showOverlay() = 0;

	/** Deactivate the overlay mode. */
	virtual void hideOverlay() = 0;

	/**
	 * Returns the pixel format description of the overlay.
	 * @see Graphics::PixelFormat
	 */
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;

	/**
	 * Reset the overlay.
	 *
	 * After calling this method while the overlay mode is active, the user
	 * should be seeing only the game graphics. How this is achieved depends
	 * on how the backend implements the overlay. Either it sets all pixels of
	 * the overlay to be transparent (when alpha blending is used).
	 *
	 * Or, in case of fake alpha blending, it might just put a copy of the
	 * current game graphics screen into the overlay.
	 */
	virtual void clearOverlay() = 0;

	/**
	 * Copy the content of the overlay into a buffer provided by the caller.
	 * This is only used to implement fake alpha blending.
	 */
	virtual void grabOverlay(void *buf, int pitch) = 0;

	/**
	 * Blit a graphics buffer to the overlay.
	 * In a sense, this is the reverse of grabOverlay.
	 *
	 * @param buf		the buffer containing the graphics data source
	 * @param pitch		the pitch of the buffer (number of bytes in a scanline)
	 * @param x			the x coordinate of the destination rectangle
	 * @param y			the y coordinate of the destination rectangle
	 * @param w			the width of the destination rectangle
	 * @param h			the height of the destination rectangle
	 *
	 * @see copyRectToScreen
	 * @see grabOverlay
	 */
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Return the height of the overlay.
	 * @see getHeight
	 */
	virtual int16 getOverlayHeight() = 0;

	/**
	 * Return the width of the overlay.
	 * @see getWidth
	 */
	virtual int16 getOverlayWidth() = 0;

	//@}



	/** @name Mouse
	 * This is the lower level implementation as provided by the
	 * backends. The engines should use the Graphics::CursorManager
	 * class instead of using it directly.
	 */
	//@{

	/**
	 * Show or hide the mouse cursor.
	 *
	 * Currently the backend is not required to immediately draw the
	 * mouse cursor on showMouse(true).
	 *
	 * TODO: We might want to reconsider this fact,
	 * check Graphics::CursorManager::showMouse for some details about
	 * this.
	 *
	 * @see Graphics::CursorManager::showMouse
	 */
	virtual bool showMouse(bool visible) = 0;

	/**
	 * Move ("warp") the mouse cursor to the specified position in virtual
	 * screen coordinates.
	 * @param x		the new x position of the mouse
	 * @param y		the new y position of the mouse
	 */
	virtual void warpMouse(int x, int y) = 0;

	/**
	 * Set the bitmap used for drawing the cursor.
	 *
	 * @param buf				the pixmap data to be used
	 * @param w					width of the mouse cursor
	 * @param h					height of the mouse cursor
	 * @param hotspotX			horizontal offset from the left side to the hotspot
	 * @param hotspotY			vertical offset from the top side to the hotspot
	 * @param keycolor			transparency color value. This should not exceed the maximum color value of the specified format.
	 *                          In case it does the behavior is undefined. The backend might just error out or simply ignore the
	 *                          value. (The SDL backend will just assert to prevent abuse of this).
	 * @param dontScale			Whether the cursor should never be scaled. An exception are high ppi displays, where the cursor
	 *                          would be too small to notice otherwise, these are allowed to scale the cursor anyway.
	 * @param format			pointer to the pixel format which cursor graphic uses (0 means CLUT8)
	 */
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) = 0;

	/**
	 * Replace the specified range of cursor the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * Backends which implement it should have kFeatureCursorPalette flag set
	 *
	 * @see setPalette
	 * @see kFeatureCursorPalette
	 */
	virtual void setCursorPalette(const byte *colors, uint start, uint num) {}

	//@}



	/** @name Events and Time */
	//@{

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Get the current time and date, in the local timezone.
	 * Corresponds on many systems to the combination of time()
	 * and localtime().
	 */
	virtual void getTimeAndDate(TimeDate &t) const = 0;

	/**
	 * Return the timer manager singleton. For more information, refer
	 * to the TimerManager documentation.
	 */
	inline Common::TimerManager *getTimerManager() {
		return _timerManager;
	}

	/**
	 * Return the event manager singleton. For more information, refer
	 * to the EventManager documentation.
	 */
	inline Common::EventManager *getEventManager() {
		return _eventManager;
	}

#ifdef ENABLE_KEYMAPPER
	/**
	 * Register hardware inputs with keymapper
	 * IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	 * this, please talk to tsoliman and/or LordHoto.
	 *
	 * @return HardwareInputSet with all keys and recommended mappings
	 *
	 * See keymapper documentation for further reference.
	 */
	virtual Common::HardwareInputSet *getHardwareInputSet() { return 0; }

	/**
	 * Return a platform-specific global keymap
	 * IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	 * this, please talk to tsoliman and/or LordHoto.
	 *
	 * @return Keymap with actions appropriate for the platform
	 *
	 * The caller will use and delete the return object.
	 *
	 * See keymapper documentation for further reference.
	 */
	virtual Common::Keymap *getGlobalKeymap() { return 0; }

	/**
	 * Return platform-specific default keybindings
	 * IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	 * this, please talk to tsoliman and/or LordHoto.
	 *
	 * @return KeymapperDefaultBindings populated with keybindings
	 *
	 * See keymapper documentation for further reference.
	 */
	virtual Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() { return 0; }
#endif
	//@}



	/**
	 * @name Mutex handling
	 * Historically, the OSystem API used to have a method which allowed
	 * creating threads. Hence mutex support was needed for thread syncing.
	 * To ease portability, though, we decided to remove the threading API.
	 * Instead, we now use timers (see setTimerCallback() and Common::Timer).
	 * But since those may be implemented using threads (and in fact, that's
	 * how our primary backend, the SDL one, does it on many systems), we
	 * still have to do mutex syncing in our timer callbacks.
	 * In addition, the sound mixer uses a mutex in case the backend runs it
	 * from a dedicated thread (as e.g. the SDL backend does).
	 *
	 * Hence backends which do not use threads to implement the timers simply
	 * can use dummy implementations for these methods.
	 */
	//@{

	typedef struct OpaqueMutex *MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occurred.
	 */
	virtual MutexRef createMutex() = 0;

	/**
	 * Lock the given mutex.
	 *
	 * @note ScummVM code assumes that the mutex implementation supports
	 * recursive locking. That is, a thread may lock a mutex twice w/o
	 * deadlocking. In case of a multilock, the mutex has to be unlocked
	 * as many times as it was locked befored it really becomes unlocked.
	 *
	 * @param mutex	the mutex to lock.
	 */
	virtual void lockMutex(MutexRef mutex) = 0;

	/**
	 * Unlock the given mutex.
	 * @param mutex	the mutex to unlock.
	 */
	virtual void unlockMutex(MutexRef mutex) = 0;

	/**
	 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
	 * If you delete a locked mutex, the behavior is undefined, in particular, your
	 * program may crash.
	 * @param mutex	the mutex to delete.
	 */
	virtual void deleteMutex(MutexRef mutex) = 0;

	//@}



	/** @name Sound */
	//@{

	/**
	 * Return the audio mixer. For more information, refer to the
	 * Audio::Mixer documentation.
	 */
	virtual Audio::Mixer *getMixer() = 0;

	//@}



	/** @name Audio CD */
	//@{

	/**
	 * Return the audio cd manager. For more information, refer to the
	 * AudioCDManager documentation.
	 */
	inline AudioCDManager *getAudioCDManager() {
		return _audiocdManager;
	}

	//@}



	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	 * Signals that a fatal error inside the client code has happened.
	 *
	 * This should quit the application.
	 */
	virtual void fatalError();

	/**
	 * Set a window caption or any other comparable status display to the
	 * given value. The caption must be a pure ISO LATIN 1 string. Passing a
	 * string with a different encoding may lead to unexpected behavior,
	 * even crashes.
	 *
	 * @param caption	the window caption to use, as an ISO LATIN 1 string
	 */
	virtual void setWindowCaption(const char *caption) {}

	/**
	 * Display a message in an 'on screen display'. That is, display it in a
	 * fashion where it is visible on or near the screen (e.g. in a transparent
	 * rectangle over the regular screen content; or in a message box beneath
	 * it; etc.).
	 *
	 * The message is expected to be provided in the current TranslationManager
	 * charset.
	 *
	 * @note There is a default implementation in BaseBackend which uses a
	 *       TimedMessageDialog to display the message. Hence implementing
	 *       this is optional.
	 *
	 * @param msg	the message to display on screen
	 */
	virtual void displayMessageOnOSD(const char *msg) = 0;

	/**
	 * Return the SaveFileManager, used to store and load savestates
	 * and other modifiable persistent game data. For more information,
	 * refer to the SaveFileManager documentation.
	 */
	inline Common::SaveFileManager *getSavefileManager() {
		return _savefileManager;
	}

#if defined(USE_TASKBAR)
	/**
	 * Returns the TaskbarManager, used to handle progress bars,
	 * icon overlay, tasks and recent items list on the taskbar.
	 *
	 * @return the TaskbarManager for the current architecture
	 */
	virtual Common::TaskbarManager *getTaskbarManager() {
		return _taskbarManager;
	}
#endif

#if defined(USE_UPDATES)
	/**
	 * Returns the UpdateManager, used to handle auto-updating,
	 * and updating of ScummVM in general.
	 *
	 * @return the UpdateManager for the current architecture
	 */
	virtual Common::UpdateManager *getUpdateManager() {
		return _updateManager;
	}
#endif

	/**
	 * Returns the FilesystemFactory object, depending on the current architecture.
	 *
	 * @return the FSNode factory for the current architecture
	 */
	virtual FilesystemFactory *getFilesystemFactory();

	/**
	 * Add system specific Common::Archive objects to the given SearchSet.
	 * E.g. on Unix the dir corresponding to DATA_PATH (if set), or on
	 * Mac OS X the 'Resource' dir in the app bundle.
	 *
	 * @todo Come up with a better name. This one sucks.
	 *
	 * @param s		the SearchSet to which the system specific dirs, if any, are added
	 * @param priority	the priority with which those dirs are added
	 */
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) {}

	/**
	 * Open the default config file for reading, by returning a suitable
	 * ReadStream instance. It is the callers responsiblity to delete
	 * the stream after use.
	 */
	virtual Common::SeekableReadStream *createConfigReadStream();

	/**
	 * Open the default config file for writing, by returning a suitable
	 * WriteStream instance. It is the callers responsiblity to delete
	 * the stream after use.
	 *
	 * May return 0 to indicate that writing to config file is not possible.
	 */
	virtual Common::WriteStream *createConfigWriteStream();

	/**
	 * Get the default file name (or even path) where the user configuration
	 * of ScummVM will be saved.
	 * Note that not all ports may use this.
	 */
	virtual Common::String getDefaultConfigFileName();

	/**
	 * Logs a given message.
	 *
	 * It is up to the backend where to log the different messages.
	 * The backend should aim at using a non-buffered output for it
	 * so that no log data is lost in case of a crash.
	 *
	 * The default implementation outputs them on stdout/stderr.
	 *
	 * @param type    the type of the message
	 * @param message the message itself
	 */
	virtual void logMessage(LogMessageType::Type type, const char *message) = 0;

	/**
	 * Open the log file in a way that allows the user to review it,
	 * and possibly email it (or parts of it) to the ScummVM team,
	 * e.g. as part of a bug report.
	 *
	 * On a desktop operating system, this would typically launch
	 * some kind of (external) text editor / viewer.
	 * On a phone, it might also cause a context switch to another
	 * application. Finally, on some ports, it might not be supported
	 * at all, and so do nothing.
	 *
	 * The kFeatureDisplayLogFile feature flag can be used to
	 * test whether this call has been implemented by the active
	 * backend.
	 *
	 * @return true if all seems to have gone fine, false if an error occurred
	 *
	 * @note An error could mean that the log file did not exist,
	 * or the editor could not launch. However, a return value of true does
	 * not guarantee that the user actually will see the log file.
	 *
	 * @note It is up to the backend to ensure that the system is in a state
	 * that allows the user to actually see the displayed log files. This
	 * might for example require leaving fullscreen mode.
	 */
	virtual bool displayLogFile() { return false; }

	/**
	 * Returns the locale of the system.
	 *
	 * This returns the currently set up locale of the system, on which
	 * ScummVM is run.
	 *
	 * The format of the locale is language_country. These should match
	 * the POSIX locale values.
	 *
	 * For information about POSIX locales read here:
	 * http://en.wikipedia.org/wiki/Locale#POSIX-type_platforms
	 *
	 * The default implementation returns "en_US".
	 *
	 * @return locale of the system
	 */
	virtual Common::String getSystemLanguage() const;

	//@}
};


/** The global OSystem instance. Initialized in main(). */
extern OSystem *g_system;

#endif
