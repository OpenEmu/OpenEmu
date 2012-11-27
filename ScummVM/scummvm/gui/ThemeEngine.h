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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GUI_THEME_ENGINE_H
#define GUI_THEME_ENGINE_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/str.h"

#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/pixelformat.h"


#define SCUMMVM_THEME_VERSION_STR "SCUMMVM_STX0.8.16"

class OSystem;

namespace Common {
struct Rect;
}

namespace Graphics {
struct DrawStep;
class VectorRenderer;
}

namespace GUI {

struct WidgetDrawData;
struct TextDrawData;
struct TextColorData;
class Dialog;
class GuiObject;
class ThemeEval;
class ThemeItem;
class ThemeParser;

/**
 * DrawData sets enumeration.
 * Each DD set corresponds to the actual looks
 * of a widget in a given state.
 */
enum DrawData {
	kDDMainDialogBackground,
	kDDSpecialColorBackground,
	kDDPlainColorBackground,
	kDDTooltipBackground,
	kDDDefaultBackground,
	kDDTextSelectionBackground,
	kDDTextSelectionFocusBackground,

	kDDWidgetBackgroundDefault,
	kDDWidgetBackgroundSmall,
	kDDWidgetBackgroundEditText,
	kDDWidgetBackgroundSlider,

	kDDButtonIdle,
	kDDButtonHover,
	kDDButtonDisabled,
	kDDButtonPressed,

	kDDSliderFull,
	kDDSliderHover,
	kDDSliderDisabled,

	kDDCheckboxDefault,
	kDDCheckboxDisabled,
	kDDCheckboxSelected,

	kDDRadiobuttonDefault,
	kDDRadiobuttonDisabled,
	kDDRadiobuttonSelected,

	kDDTabActive,
	kDDTabInactive,
	kDDTabBackground,

	kDDScrollbarBase,
	kDDScrollbarButtonIdle,
	kDDScrollbarButtonHover,
	kDDScrollbarHandleIdle,
	kDDScrollbarHandleHover,

	kDDPopUpIdle,
	kDDPopUpHover,
	kDDPopUpDisabled,

	kDDCaret,
	kDDSeparator,
	kDrawDataMAX,
	kDDNone = -1
};

// FIXME: TextData is really a bad name, not conveying what this enum is about.
enum TextData {
	kTextDataNone = -1,
	kTextDataDefault = 0,
	kTextDataButton,
	kTextDataNormalFont,
	kTextDataTooltip,
	kTextDataMAX
};

enum TextColor {
	kTextColorNormal = 0,
	kTextColorNormalInverted,
	kTextColorNormalHover,
	kTextColorNormalDisabled,
	kTextColorAlternative,
	kTextColorAlternativeInverted,
	kTextColorAlternativeHover,
	kTextColorAlternativeDisabled,
	kTextColorButton,
	kTextColorButtonHover,
	kTextColorButtonDisabled,
	kTextColorMAX
};

class ThemeEngine {
protected:
	typedef Common::HashMap<Common::String, Graphics::Surface *> ImagesMap;

	friend class GUI::Dialog;
	friend class GUI::GuiObject;

public:
	/// Vertical alignment of the text.
	enum TextAlignVertical {
		kTextAlignVInvalid,
		kTextAlignVBottom,
		kTextAlignVCenter,
		kTextAlignVTop
	};

	/// Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,            ///< No background at all
		kWidgetBackgroundPlain,         ///< Simple background, this may not include borders
		kWidgetBackgroundBorder,        ///< Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,   ///< Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,      ///< Background used for edit text fields
		kWidgetBackgroundSlider         ///< Background used for sliders
	};

	/// Dialog background type
	enum DialogBackground {
		kDialogBackgroundMain,
		kDialogBackgroundSpecial,
		kDialogBackgroundPlain,
		kDialogBackgroundTooltip,
		kDialogBackgroundDefault
	};

	/// State of the widget to be drawn
	enum State {
		kStateDisabled,     ///< Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,      ///< Indicates that the widget is enabled
		kStateHighlight,    ///< Indicates that the widget is highlighted by the user
		kStatePressed       ///< Indicates that the widget is pressed, currently works for buttons
	};

	typedef State WidgetStateInfo;

	/// Text inversion state of the text to be draw
	enum TextInversionState {
		kTextInversionNone, ///< Indicates that the text should not be drawn inverted
		kTextInversion,     ///< Indicates that the text should be drawn inverted, but not focused
		kTextInversionFocus ///< Indicates that the text should be drawn inverted, and focused
	};

	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	/// Font style selector
	enum FontStyle {
		kFontStyleBold = 0,         ///< A bold font. This is also the default font.
		kFontStyleNormal = 1,       ///< A normal font.
		kFontStyleItalic = 2,       ///< Italic styled font.
		kFontStyleFixedNormal = 3,  ///< Fixed size font.
		kFontStyleFixedBold = 4,    ///< Fixed size bold font.
		kFontStyleFixedItalic = 5,  ///< Fixed size italic font.
		kFontStyleTooltip = 6,      ///< Tiny console font
		kFontStyleMax
	};

	/// Font color selector
	enum FontColor {
		kFontColorNormal = 0,       ///< The default color of the theme
		kFontColorAlternate = 1,    ///< Alternative font color
		kFontColorMax
	};

	/// Function used to process areas other than the current dialog
	enum ShadingStyle {
		kShadingNone,       ///< No special post processing
		kShadingDim,        ///< Dimming unused areas
		kShadingLuminance   ///< Converting colors to luminance for unused areas
	};

	// Special image ids for images used in the GUI
	static const char *const kImageLogo;      ///< ScummVM logo used in the launcher
	static const char *const kImageLogoSmall; ///< ScummVM logo used in the GMM
	static const char *const kImageSearch;    ///< Search tool image used in the launcher
	static const char *const kImageEraser;     ///< Clear input image used in the launcher
	static const char *const kImageDelbtn; ///< Delete characters in the predictive dialog
	static const char *const kImageList;      ///< List image used in save/load chooser selection
	static const char *const kImageGrid;      ///< Grid image used in save/load chooser selection

	/**
	 * Graphics mode enumeration.
	 * Each item represents a set of BPP and Renderer modes for a given
	 * surface.
	 */
	enum GraphicsMode {
		kGfxDisabled = 0,   ///< No GFX
		kGfxStandard16bit,  ///< 2BPP with the standard (aliased) renderer.
		kGfxAntialias16bit  ///< 2BPP with the optimized AA renderer.
	};

	/** Constant value to expand dirty rectangles, to make sure they are fully copied */
	static const int kDirtyRectangleThreshold = 1;

	struct Renderer {
		const char *name;
		const char *shortname;
		const char *cfg;
		GraphicsMode mode;
	};

	static const Renderer _rendererModes[];
	static const uint _rendererModesSize;

	static const GraphicsMode _defaultRendererMode;

	static GraphicsMode findMode(const Common::String &cfg);
	static const char *findModeConfigName(GraphicsMode mode);

	/** Default constructor */
	ThemeEngine(Common::String id, GraphicsMode mode);

	/** Default destructor */
	~ThemeEngine();

	bool init();
	void clearAll();

	void refresh();
	void enable();
	void disable();

	/**
	 * Query the set up pixel format.
	 */
	const Graphics::PixelFormat getPixelFormat() const { return _overlayFormat; }

	/**
	 * Implementation of the GUI::Theme API. Called when a
	 * new dialog is opened. Note that the boolean parameter
	 * meaning has been changed.
	 *
	 * @param enableBuffering If set to true, buffering is enabled for
	 *                        drawing this dialog, and will continue enabled
	 *                        until disabled.
	 */
	void openDialog(bool enableBuffering, ShadingStyle shading = kShadingNone);

	/**
	 * The updateScreen() method is called every frame.
	 * It processes all the drawing queues and then copies dirty rects
	 * in the current Screen surface to the overlay.
	 */
	void updateScreen(bool render = true);


	/** @name FONT MANAGEMENT METHODS */
	//@{

	TextData fontStyleToData(FontStyle font) const {
		if (font == kFontStyleNormal)
			return kTextDataNormalFont;
		if (font == kFontStyleTooltip)
			return kTextDataTooltip;
		return kTextDataDefault;
	}

	const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const;

	int getFontHeight(FontStyle font = kFontStyleBold) const;

	int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const;

	int getCharWidth(byte c, FontStyle font = kFontStyleBold) const;

	int getKerningOffset(byte left, byte right, FontStyle font = kFontStyleBold) const;

	//@}


	/** @name WIDGET DRAWING METHODS */
	//@{

	void drawWidgetBackground(const Common::Rect &r, uint16 hints,
	                          WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled);

	void drawButton(const Common::Rect &r, const Common::String &str,
	                WidgetStateInfo state = kStateEnabled, uint16 hints = 0);

	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface,
	                 WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false);

	void drawSlider(const Common::Rect &r, int width,
	                WidgetStateInfo state = kStateEnabled);

	void drawCheckbox(const Common::Rect &r, const Common::String &str,
	                  bool checked, WidgetStateInfo state = kStateEnabled);

	void drawRadiobutton(const Common::Rect &r, const Common::String &str,
	                     bool checked, WidgetStateInfo state = kStateEnabled);

	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth,
	             const Common::Array<Common::String> &tabs, int active, uint16 hints,
	             int titleVPad, WidgetStateInfo state = kStateEnabled);

	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight,
	                   ScrollbarState, WidgetStateInfo state = kStateEnabled);

	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel,
	                     int deltax, WidgetStateInfo state = kStateEnabled, Graphics::TextAlign align = Graphics::kTextAlignLeft);

	void drawCaret(const Common::Rect &r, bool erase,
	               WidgetStateInfo state = kStateEnabled);

	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled);

	void drawDialogBackground(const Common::Rect &r, DialogBackground type, WidgetStateInfo state = kStateEnabled);

	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, Graphics::TextAlign align = Graphics::kTextAlignCenter, TextInversionState inverted = kTextInversionNone, int deltax = 0, bool useEllipsis = true, FontStyle font = kFontStyleBold, FontColor color = kFontColorNormal, bool restore = true);

	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state = kStateEnabled, FontColor color = kFontColorNormal);

	//@}



	/**
	 * Actual implementation of a dirty rect handling.
	 * Dirty rectangles are queued on a list and are later used for the
	 * actual drawing.
	 *
	 * @param r Area of the dirty rect.
	 */
	void addDirtyRect(Common::Rect r);


	/**
	 * Returns the DrawData enumeration value that represents the given string
	 * in the DrawDataDefaults enumeration.
	 * It's slow, but called sparsely.
	 *
	 * @returns The drawdata enum value, or -1 if not found.
	 * @param name The representing name, as found on Theme Description XML files.
	 * @see kDrawDataDefaults[]
	 */
	DrawData parseDrawDataId(const Common::String &name) const;

	TextData getTextData(DrawData ddId) const;
	TextColor getTextColor(DrawData ddId) const;


	/**
	 * Interface for ThemeParser class: Parsed DrawSteps are added via this function.
	 * There is no return type because DrawSteps can always be added, unless something
	 * goes horribly wrong.
	 * The specified step will be added to the Steps list of the given DrawData id.
	 *
	 * @param drawDataId The representing DrawData name, as found on Theme Description XML files.
	 * @param step The actual DrawStep struct to be added.
	 */
	void addDrawStep(const Common::String &drawDataId, const Graphics::DrawStep &step);

	/**
	 * Interface for the ThemeParser class: Parsed DrawData sets are added via this function.
	 * The goal of the function is to initialize each DrawData set before their DrawSteps can
	 * be added, hence this must be called for each DD set before addDrawStep() can be called
	 * for that given set.
	 *
	 * @param data The representing DrawData name, as found on Theme Description XML files.
	 * @param cached Whether this DD set will be cached beforehand.
	 */
	bool addDrawData(const Common::String &data, bool cached);


	/**
	 * Interface for the ThemeParser class: Loads a font to use on the GUI from the given
	 * filename.
	 *
	 * @param fextId            Identifier name for the font.
	 * @param file              Filename of the non-scalable font version.
	 * @param scalableFile      Filename of the scalable version. (Optional)
	 * @param pointsize         Point size for the scalable font. (Optional)
	 */
	bool addFont(TextData textId, const Common::String &file, const Common::String &scalableFile, const int pointsize);

	/**
	 * Interface for the ThemeParser class: adds a text color value.
	 *
	 * @param colorId Identifier for the color type.
	 * @param r, g, b Color of the font.
	 */
	bool addTextColor(TextColor colorId, int r, int g, int b);


	/**
	 * Interface for the ThemeParser class: Loads a bitmap file to use on the GUI.
	 * The filename is also used as its identifier.
	 *
	 * @param filename Name of the bitmap file.
	 */
	bool addBitmap(const Common::String &filename);

	/**
	 * Adds a new TextStep from the ThemeParser. This will be deprecated/removed once the
	 * new Font API is in place. FIXME: Is that so ???
	 */
	bool addTextData(const Common::String &drawDataId, TextData textId, TextColor id, Graphics::TextAlign alignH, TextAlignVertical alignV);

protected:
	/**
	 * Returns if the Theme is ready to draw stuff on screen.
	 * Must be called instead of just checking _initOk, because
	 * this checks if the renderer is initialized AND if the theme
	 * is loaded.
	 */
	bool ready() const {
		return _initOk && _themeOk;
	}

	/** Load the them from the file with the specified name. */
	void loadTheme(const Common::String &themeid);

	/**
	 * Changes the active graphics mode of the GUI; may be used to either
	 * initialize the GUI or to change the mode while the GUI is already running.
	 */
	void setGraphicsMode(GraphicsMode mode);

public:
	/**
	 * Finishes buffering: widgets from then on will be drawn straight on the screen
	 * without drawing queues.
	 */
	inline void finishBuffering() { _buffering = false; }
	inline void startBuffering() { _buffering = true; }

	inline ThemeEval *getEvaluator() { return _themeEval; }
	inline Graphics::VectorRenderer *renderer() { return _vectorRenderer; }

	inline bool supportsImages() const { return true; }
	inline bool ownCursor() const { return _useCursor; }

	Graphics::Surface *getBitmap(const Common::String &name) {
		return _bitmaps.contains(name) ? _bitmaps[name] : 0;
	}

	const Graphics::Surface *getImageSurface(const Common::String &name) const {
		return _bitmaps.contains(name) ? _bitmaps[name] : 0;
	}

	/**
	 * Interface for the Theme Parser: Creates a new cursor by loading the given
	 * bitmap and sets it as the active cursor.
	 *
	 * @param filename File name of the bitmap to load.
	 * @param hotspotX X Coordinate of the bitmap which does the cursor click.
	 * @param hotspotY Y Coordinate of the bitmap which does the cursor click.
	 */
	bool createCursor(const Common::String &filename, int hotspotX, int hotspotY);

	/**
	 * Wrapper for restoring data from the Back Buffer to the screen.
	 * The actual processing is done in the VectorRenderer.
	 *
	 * @param r Area to restore.
	 */
	void restoreBackground(Common::Rect r);

	const Common::String &getThemeName() const { return _themeName; }
	const Common::String &getThemeId() const { return _themeId; }
	int getGraphicsMode() const { return _graphicsMode; }

protected:
	/**
	 * Initializes the drawing screen surfaces, _screen and _backBuffer.
	 * If the surfaces already exist, they are cleared and re-initialized.
	 *
	 * @param backBuffer Sets whether the _backBuffer surface should be initialized.
	 * @template PixelType C type which specifies the size of each pixel.
	 *                     Defaults to uint16 (2 BPP for the surfaces)
	 */
	template<typename PixelType> void screenInit(bool backBuffer = true);

	/**
	 * Loads the given theme into the ThemeEngine.
	 *
	 * @param themeId Theme identifier.
	 * @returns true if the theme was successfully loaded.
	 */
	bool loadThemeXML(const Common::String &themeId);

	/**
	 * Loads the default theme file (the embedded XML file found
	 * in ThemeDefaultXML.cpp).
	 * Called only when no other themes are available.
	 */
	bool loadDefaultXML();

	/**
	 * Unloads the currently loaded theme so another one can
	 * be loaded.
	 */
	void unloadTheme();

	const Graphics::Font *loadScalableFont(const Common::String &filename, const Common::String &charset, const int pointsize, Common::String &name);
	const Graphics::Font *loadFont(const Common::String &filename, Common::String &name);
	Common::String genCacheFilename(const Common::String &filename) const;
	const Graphics::Font *loadFont(const Common::String &filename, const Common::String &scalableFilename, const Common::String &charset, const int pointsize, const bool makeLocalizedFont);

	/**
	 * Actual Dirty Screen handling function.
	 * Handles all the dirty squares in the list, merges and optimizes
	 * them when possible and draws them to the screen.
	 * Called from updateScreen()
	 */
	void renderDirtyScreen();

	/**
	 * Generates a DrawQueue item and enqueues it so it's drawn to the screen
	 * when the drawing queue is processed.
	 *
	 * If Buffering is enabled, the DrawQueue item will be automatically placed
	 * on its corresponding queue.
	 * If Buffering is disabled, the DrawQueue item will be processed immediately
	 * and drawn to the screen.
	 *
	 * This function is called from all the Widget Drawing methods.
	 */
	void queueDD(DrawData type,  const Common::Rect &r, uint32 dynamic = 0, bool restore = false);
	void queueDDText(TextData type, TextColor color, const Common::Rect &r, const Common::String &text, bool restoreBg,
	                 bool elipsis, Graphics::TextAlign alignH = Graphics::kTextAlignLeft, TextAlignVertical alignV = kTextAlignVTop, int deltax = 0);
	void queueBitmap(const Graphics::Surface *bitmap, const Common::Rect &r, bool alpha);

	/**
	 * DEBUG: Draws a white square and writes some text next to it.
	 */
	void debugWidgetPosition(const char *name, const Common::Rect &r);

public:
	struct ThemeDescriptor {
		Common::String name;
		Common::String id;
		Common::String filename;
	};

	/**
	 * Lists all theme files useable.
	 */
	static void listUsableThemes(Common::List<ThemeDescriptor> &list);
private:
	static bool themeConfigUsable(const Common::FSNode &node, Common::String &themeName);
	static bool themeConfigUsable(const Common::ArchiveMember &member, Common::String &themeName);
	static bool themeConfigParseHeader(Common::String header, Common::String &themeName);

	static Common::String getThemeFile(const Common::String &id);
	static Common::String getThemeId(const Common::String &filename);
	static void listUsableThemes(const Common::FSNode &node, Common::List<ThemeDescriptor> &list, int depth = -1);
	static void listUsableThemes(Common::Archive &archive, Common::List<ThemeDescriptor> &list);

protected:
	OSystem *_system; /** Global system object. */

	/** Vector Renderer object, does the actual drawing on screen */
	Graphics::VectorRenderer *_vectorRenderer;

	/** XML Parser, does the Theme parsing instead of the default parser */
	GUI::ThemeParser *_parser;

	/** Theme getEvaluator (changed from GUI::Eval to add functionality) */
	GUI::ThemeEval *_themeEval;

	/** Main screen surface. This is blitted straight into the overlay. */
	Graphics::Surface _screen;

	/** Backbuffer surface. Stores previous states of the screen to blit back */
	Graphics::Surface _backBuffer;

	/** Sets whether the current drawing is being buffered (stored for later
	    processing) or drawn directly to the screen. */
	bool _buffering;

	/** Bytes per pixel of the Active Drawing Surface (i.e. the screen) */
	int _bytesPerPixel;

	/** Current graphics mode */
	GraphicsMode _graphicsMode;

	/** Font info. */
	Common::String _fontName;
	const Graphics::Font *_font;

	/**
	 * Array of all the DrawData elements than can be drawn to the screen.
	 * Must be full so the renderer can work.
	 */
	WidgetDrawData *_widgets[kDrawDataMAX];

	/** Array of all the text fonts that can be drawn. */
	TextDrawData *_texts[kTextDataMAX];

	/** Array of all font colors available. */
	TextColorData *_textColors[kTextColorMAX];

	ImagesMap _bitmaps;
	Graphics::PixelFormat _overlayFormat;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _cursorFormat;
#endif

	/** List of all the dirty screens that must be blitted to the overlay. */
	Common::List<Common::Rect> _dirtyScreen;

	/** Queue with all the drawing that must be done to the Back Buffer */
	Common::List<ThemeItem *> _bufferQueue;

	/** Queue with all the drawing that must be done to the screen */
	Common::List<ThemeItem *> _screenQueue;

	bool _initOk;  ///< Class and renderer properly initialized
	bool _themeOk; ///< Theme data successfully loaded.
	bool _enabled; ///< Whether the Theme is currently shown on the overlay

	Common::String _themeName; ///< Name of the currently loaded theme
	Common::String _themeId;
	Common::String _themeFile;
	Common::Archive *_themeArchive;
	Common::SearchSet _themeFiles;

	bool _useCursor;
	int _cursorHotspotX, _cursorHotspotY;
	enum {
		MAX_CURS_COLORS = 255
	};
	byte *_cursor;
	bool _needPaletteUpdates;
	uint _cursorWidth, _cursorHeight;
	byte _cursorPal[3 * MAX_CURS_COLORS];
	byte _cursorPalSize;
};

} // End of namespace GUI.

#endif
