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

#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/ThemeParser.h"

#include "graphics/VectorRenderer.h"

#include "common/system.h"
#include "common/tokenizer.h"

namespace GUI {

struct TextDataInfo {
	TextData id;
	const char *name;
};

static const TextDataInfo kTextDataDefaults[] = {
	{ kTextDataDefault,			"text_default" },
	{ kTextDataButton,			"text_button" },
	{ kTextDataNormalFont,		"text_normal" },
	{ kTextDataTooltip,			"tooltip_normal" }
};


static TextData parseTextDataId(const Common::String &name) {
	for (int i = 0; i < kTextDataMAX; ++i)
		if (name.compareToIgnoreCase(kTextDataDefaults[i].name) == 0)
			return kTextDataDefaults[i].id;

	return kTextDataNone;
}

struct TextColorDataInfo {
	TextColor id;
	const char *name;
};

static const TextColorDataInfo kTextColorDefaults[] = {
	{ kTextColorNormal,					"color_normal" },
	{ kTextColorNormalInverted,			"color_normal_inverted" },
	{ kTextColorNormalHover,			"color_normal_hover" },
	{ kTextColorNormalDisabled,			"color_normal_disabled" },
	{ kTextColorAlternative,			"color_alternative" },
	{ kTextColorAlternativeInverted,	"color_alternative_inverted" },
	{ kTextColorAlternativeHover,		"color_alternative_hover" },
	{ kTextColorAlternativeDisabled,	"color_alternative_disabled" },
	{ kTextColorButton,					"color_button" },
	{ kTextColorButtonHover,			"color_button_hover" },
	{ kTextColorButtonDisabled,			"color_button_disabled" }
};

static TextColor parseTextColorId(const Common::String &name) {
	for (int i = 0; i < kTextColorMAX; ++i)
		if (name.compareToIgnoreCase(kTextColorDefaults[i].name) == 0)
			return kTextColorDefaults[i].id;

	return kTextColorMAX;
}

static Graphics::TextAlign parseTextHAlign(const Common::String &val) {
	if (val == "left")
		return Graphics::kTextAlignLeft;
	else if (val == "right")
		return Graphics::kTextAlignRight;
	else if (val == "center")
		return Graphics::kTextAlignCenter;
	else
		return Graphics::kTextAlignInvalid;
}

static GUI::ThemeEngine::TextAlignVertical parseTextVAlign(const Common::String &val) {
	if (val == "top")
		return GUI::ThemeEngine::kTextAlignVTop;
	else if (val == "center")
		return GUI::ThemeEngine::kTextAlignVCenter;
	else if (val == "bottom")
		return GUI::ThemeEngine::kTextAlignVBottom;
	else
		return GUI::ThemeEngine::kTextAlignVInvalid;
}


ThemeParser::ThemeParser(ThemeEngine *parent) : XMLParser() {
	_defaultStepGlobal = defaultDrawStep();
	_defaultStepLocal = 0;
	_theme = parent;
}

ThemeParser::~ThemeParser() {
	delete _defaultStepGlobal;
	delete _defaultStepLocal;
}

void ThemeParser::cleanup() {
	delete _defaultStepGlobal;
	delete _defaultStepLocal;

	_defaultStepGlobal = defaultDrawStep();
	_defaultStepLocal = 0;
	_palette.clear();
}

Graphics::DrawStep *ThemeParser::defaultDrawStep() {
	Graphics::DrawStep *step = new Graphics::DrawStep;

	memset(step, 0, sizeof(Graphics::DrawStep));

	step->xAlign = Graphics::DrawStep::kVectorAlignManual;
	step->yAlign = Graphics::DrawStep::kVectorAlignManual;
	step->factor = 1;
	step->autoWidth = true;
	step->autoHeight = true;
	step->fillMode = Graphics::VectorRenderer::kFillDisabled;
	step->scale = (1 << 16);
	step->radius = 0xFF;

	return step;
}

Graphics::DrawStep *ThemeParser::newDrawStep() {
	assert(_defaultStepGlobal);
	Graphics::DrawStep *step = 0; //new DrawStep;

	if (_defaultStepLocal) {
		step = new Graphics::DrawStep(*_defaultStepLocal);
	} else {
		step = new Graphics::DrawStep(*_defaultStepGlobal);
	}

	return step;
}

bool ThemeParser::parserCallback_defaults(ParserNode *node) {
	ParserNode *parentNode = getParentNode(node);
	Graphics::DrawStep *step = 0;

	if (parentNode->name == "render_info") {
		step = _defaultStepGlobal;
	} else if (parentNode->name == "drawdata") {
		if (_defaultStepLocal == 0)
			_defaultStepLocal = new Graphics::DrawStep(*_defaultStepGlobal);

		step = _defaultStepLocal;
	} else {
		return parserError("<default> key out of scope. Must be inside <drawdata> or <render_info> keys.");
	}

	return parseDrawStep(node, step, false);
}

bool ThemeParser::parserCallback_font(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	// Default to a point size of 12.
	int pointsize = 12;
	if (node->values.contains("point_size")) {
		if (sscanf(node->values["point_size"].c_str(), "%d", &pointsize) != 1 || pointsize <= 0)
			return parserError(Common::String::format("Font \"%s\" has invalid point size \"%s\"", node->values["id"].c_str(), node->values["point_size"].c_str()));
	}

	TextData textDataId = parseTextDataId(node->values["id"]);
	if (!_theme->addFont(textDataId, node->values["file"], node->values["scalable_file"], pointsize))
		return parserError("Error loading Font in theme engine.");

	return true;
}

bool ThemeParser::parserCallback_text_color(ParserNode *node) {
	int red, green, blue;

	TextColor colorId = parseTextColorId(node->values["id"]);
	if (colorId == kTextColorMAX)
		return parserError("Error text color is not defined.");

	if (_palette.contains(node->values["color"]))
		getPaletteColor(node->values["color"], red, green, blue);
	else if (!parseIntegerKey(node->values["color"], 3, &red, &green, &blue))
		return parserError("Error parsing color value for text color definition.");

	if (!_theme->addTextColor(colorId, red, green, blue))
		return parserError("Error while adding text color information.");

	return true;
}

bool ThemeParser::parserCallback_fonts(ParserNode *node) {
	return true;
}

bool ThemeParser::parserCallback_cursor(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	int spotx, spoty;

	if (!parseIntegerKey(node->values["hotspot"], 2, &spotx, &spoty))
		return parserError("Error parsing cursor Hot Spot coordinates.");

	if (!_theme->createCursor(node->values["file"], spotx, spoty))
		return parserError("Error creating Bitmap Cursor.");

	return true;
}

bool ThemeParser::parserCallback_bitmap(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	if (!_theme->addBitmap(node->values["filename"]))
		return parserError("Error loading Bitmap file '" + node->values["filename"] + "'");

	return true;
}

bool ThemeParser::parserCallback_text(ParserNode *node) {
	Graphics::TextAlign alignH;
	GUI::ThemeEngine::TextAlignVertical alignV;

	if ((alignH = parseTextHAlign(node->values["horizontal_align"])) == Graphics::kTextAlignInvalid)
		return parserError("Invalid value for text alignment.");

	if ((alignV = parseTextVAlign(node->values["vertical_align"])) == GUI::ThemeEngine::kTextAlignVInvalid)
		return parserError("Invalid value for text alignment.");

	Common::String id = getParentNode(node)->values["id"];
	TextData textDataId = parseTextDataId(node->values["font"]);
	TextColor textColorId = parseTextColorId(node->values["text_color"]);

	if (!_theme->addTextData(id, textDataId, textColorId, alignH, alignV))
		return parserError("Error adding Text Data for '" + id + "'.");

	return true;
}

bool ThemeParser::parserCallback_render_info(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false)
		node->ignore = true;

	return true;
}

bool ThemeParser::parserCallback_layout_info(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false)
		node->ignore = true;

	return true;
}

bool ThemeParser::parserCallback_palette(ParserNode *node) {
	return true;
}

bool ThemeParser::parserCallback_color(ParserNode *node) {
	Common::String name = node->values["name"];

	if (_palette.contains(name))
		return parserError("Color '" + name + "' has already been defined.");

	int red, green, blue;

	if (parseIntegerKey(node->values["rgb"], 3, &red, &green, &blue) == false ||
		red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
		return parserError("Error parsing RGB values for palette color '" + name + "'");

	_palette[name].r = red;
	_palette[name].g = green;
	_palette[name].b = blue;

	return true;
}


static Graphics::DrawingFunctionCallback getDrawingFunctionCallback(const Common::String &name) {

	if (name == "circle")
		return &Graphics::VectorRenderer::drawCallback_CIRCLE;
	if (name == "square")
		return &Graphics::VectorRenderer::drawCallback_SQUARE;
	if (name == "roundedsq")
		return &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	if (name == "bevelsq")
		return &Graphics::VectorRenderer::drawCallback_BEVELSQ;
	if (name == "line")
		return &Graphics::VectorRenderer::drawCallback_LINE;
	if (name == "triangle")
		return &Graphics::VectorRenderer::drawCallback_TRIANGLE;
	if (name == "fill")
		return &Graphics::VectorRenderer::drawCallback_FILLSURFACE;
	if (name == "tab")
		return &Graphics::VectorRenderer::drawCallback_TAB;
	if (name == "void")
		return &Graphics::VectorRenderer::drawCallback_VOID;
	if (name == "bitmap")
		return &Graphics::VectorRenderer::drawCallback_BITMAP;
	if (name == "cross")
		return &Graphics::VectorRenderer::drawCallback_CROSS;

	return 0;
}


bool ThemeParser::parserCallback_drawstep(ParserNode *node) {
	Graphics::DrawStep *drawstep = newDrawStep();

	Common::String functionName = node->values["func"];

	drawstep->drawingCall = getDrawingFunctionCallback(functionName);

	if (drawstep->drawingCall == 0)
		return parserError(functionName + " is not a valid drawing function name");

	if (!parseDrawStep(node, drawstep, true))
		return false;

	_theme->addDrawStep(getParentNode(node)->values["id"], *drawstep);
	delete drawstep;

	return true;
}

bool ThemeParser::parserCallback_drawdata(ParserNode *node) {
	bool cached = false;

	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	if (node->values.contains("cache")) {
		if (!Common::parseBool(node->values["cache"], cached))
			return parserError("'Parsed' value must be either true or false.");
	}

	if (_theme->addDrawData(node->values["id"], cached) == false)
		return parserError("Error adding Draw Data set: Invalid DrawData name.");

	delete _defaultStepLocal;
	_defaultStepLocal = 0;

	return true;
}

bool ThemeParser::parseDrawStep(ParserNode *stepNode, Graphics::DrawStep *drawstep, bool functionSpecific) {
	int red, green, blue, x;
	Common::String val;

/**
 * Helper macro to sanitize and assign an integer value from a key
 * to the draw step.
 *
 * @param struct_name Name of the field of a DrawStep struct that must be
 *                    assigned.
 * @param key_name Name as STRING of the key identifier as it appears in the
 *                 theme description format.
 * @param force Sets if the key is optional or necessary.
 */
#define PARSER_ASSIGN_INT(struct_name, key_name, force) \
	if (stepNode->values.contains(key_name)) { \
		if (!parseIntegerKey(stepNode->values[key_name], 1, &x)) \
			return parserError("Error parsing key value for '" + Common::String(key_name) + "'."); \
		\
		drawstep->struct_name = x; \
	} else if (force) { \
		return parserError("Missing necessary key '" + Common::String(key_name) + "'."); \
	}

/**
 * Helper macro to sanitize and assign a RGB value from a key to the draw
 * step. RGB values have the following syntax: "R, G, B".
 *
 * @param struct_name Name of the field of a DrawStep struct that must be
 *                    assigned.
 * @param key_name Name as STRING of the key identifier as it appears in the
 *                 theme description format.
 */
#define PARSER_ASSIGN_RGB(struct_name, key_name) \
	if (stepNode->values.contains(key_name)) { \
		val = stepNode->values[key_name]; \
		if (_palette.contains(val)) { \
			red = _palette[val].r; \
			green = _palette[val].g; \
			blue = _palette[val].b; \
		} else if (parseIntegerKey(val, 3, &red, &green, &blue) == false || \
			red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) \
			return parserError("Error parsing color struct '" + val + "'");\
		\
		drawstep->struct_name.r = red; \
		drawstep->struct_name.g = green; \
		drawstep->struct_name.b = blue; \
		drawstep->struct_name.set = true; \
	}

	PARSER_ASSIGN_INT(stroke, "stroke", false);
	PARSER_ASSIGN_INT(bevel, "bevel", false);
	PARSER_ASSIGN_INT(shadow, "shadow", false);
	PARSER_ASSIGN_INT(factor, "gradient_factor", false);

	PARSER_ASSIGN_RGB(fgColor, "fg_color");
	PARSER_ASSIGN_RGB(bgColor, "bg_color");
	PARSER_ASSIGN_RGB(gradColor1, "gradient_start");
	PARSER_ASSIGN_RGB(gradColor2, "gradient_end");
	PARSER_ASSIGN_RGB(bevelColor, "bevel_color");

	if (functionSpecific) {
		assert(stepNode->values.contains("func"));
		Common::String functionName = stepNode->values["func"];

		if (functionName == "bitmap") {
			if (!stepNode->values.contains("file"))
				return parserError("Need to specify a filename for Bitmap blitting.");

			drawstep->blitSrc = _theme->getBitmap(stepNode->values["file"]);

			if (!drawstep->blitSrc)
				return parserError("The given filename hasn't been loaded into the GUI.");
		}

		if (functionName == "roundedsq" || functionName == "circle" || functionName == "tab") {
			if (stepNode->values.contains("radius") && stepNode->values["radius"] == "auto") {
				drawstep->radius = 0xFF;
			} else {
				PARSER_ASSIGN_INT(radius, "radius", true);
			}
		}

		if (functionName == "triangle") {
			drawstep->extraData = Graphics::VectorRenderer::kTriangleUp;

			if (stepNode->values.contains("orientation")) {
				val = stepNode->values["orientation"];

				if (val == "top")
					drawstep->extraData = Graphics::VectorRenderer::kTriangleUp;
				else if (val == "bottom")
					drawstep->extraData = Graphics::VectorRenderer::kTriangleDown;
				else if (val == "left")
					drawstep->extraData = Graphics::VectorRenderer::kTriangleLeft;
				else if (val == "right")
					drawstep->extraData = Graphics::VectorRenderer::kTriangleRight;
				else
					return parserError("'" + val + "' is not a valid value for triangle orientation.");
			}
		}

		if (stepNode->values.contains("size")) {
			warning("The <size> keyword has been deprecated. Use <width> and <height> instead");
		}

		if (stepNode->values.contains("width") && stepNode->values["width"] != "auto") {
			drawstep->autoWidth = false;

			val = stepNode->values["width"];
			if (parseIntegerKey(val, 1, &x))
				drawstep->w = x;
			else if (val == "height")
				drawstep->w = -1;
			else return parserError("Invalid value for vector width.");

			if (stepNode->values.contains("xpos")) {
				val = stepNode->values["xpos"];

				if (parseIntegerKey(val, 1, &x))
					drawstep->x = x;
				else if (val == "center")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignCenter;
				else if (val == "left")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignLeft;
				else if (val == "right")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignRight;
				else
					return parserError("Invalid value for X Position");
			} else {
				return parserError("When width is not set to 'auto', a <xpos> tag must be included.");
			}
		}

		if (stepNode->values.contains("height") && stepNode->values["height"] != "auto") {
			drawstep->autoHeight = false;

			val = stepNode->values["height"];
			if (parseIntegerKey(val, 1, &x))
				drawstep->h = x;
			else if (val == "width")
				drawstep->h = -1;
			else return parserError("Invalid value for vector height.");

			if (stepNode->values.contains("ypos")) {
				val = stepNode->values["ypos"];

				if (parseIntegerKey(val, 1, &x))
					drawstep->y = x;
				else if (val == "center")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignCenter;
				else if (val == "top")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignTop;
				else if (val == "bottom")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignBottom;
				else
					return parserError("Invalid value for Y Position");
			} else {
				return parserError("When height is not set to 'auto', a <ypos> tag must be included.");
			}
		}

		if (drawstep->h == -1 && drawstep->w == -1)
			return parserError("Cross-reference in Vector Size: Height is set to width and width is set to height.");
	}

	if (stepNode->values.contains("fill")) {
		val = stepNode->values["fill"];
		if (val == "none")
			drawstep->fillMode = Graphics::VectorRenderer::kFillDisabled;
		else if (val == "foreground")
			drawstep->fillMode = Graphics::VectorRenderer::kFillForeground;
		else if (val == "background")
			drawstep->fillMode = Graphics::VectorRenderer::kFillBackground;
		else if (val == "gradient")
			drawstep->fillMode = Graphics::VectorRenderer::kFillGradient;
		else
			return parserError("'" + stepNode->values["fill"] + "' is not a valid fill mode for a shape.");
	}

	if (stepNode->values.contains("padding")) {
		val = stepNode->values["padding"];
		int pr, pt, pl, pb;
		if (parseIntegerKey(val, 4, &pl, &pt, &pr, &pb))
			drawstep->padding.left = pl,
			drawstep->padding.top = pt,
			drawstep->padding.right = pr,
			drawstep->padding.bottom = pb;
	}

#undef PARSER_ASSIGN_INT
#undef PARSER_ASSIGN_RGB

	return true;
}

bool ThemeParser::parserCallback_def(ParserNode *node) {
	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	Common::String var = "Globals." + node->values["var"];
	int value;

	if (_theme->getEvaluator()->hasVar(node->values["value"]) == true)
		value = _theme->getEvaluator()->getVar(node->values["value"]);

	else if (!parseIntegerKey(node->values["value"], 1, &value))
		return parserError("Invalid definition for '" + var + "'.");

	_theme->getEvaluator()->setVar(var, value);
	return true;
}

bool ThemeParser::parserCallback_widget(ParserNode *node) {
	Common::String var;

	if (getParentNode(node)->name == "globals") {

		if (resolutionCheck(node->values["resolution"]) == false) {
			node->ignore = true;
			return true;
		}

		var = "Globals." + node->values["name"] + ".";
		if (!parseCommonLayoutProps(node, var))
			return parserError("Error parsing Layout properties of '" + var + "'.");

	} else {
		// FIXME: Shouldn't we distinguish the name/id and the label of a widget?
		var = node->values["name"];
		int width = -1;
		int height = -1;
		bool enabled = true;

		if (node->values.contains("enabled")) {
			if (!Common::parseBool(node->values["enabled"], enabled))
				return parserError("Invalid value for Widget enabling (expecting true/false)");
		}

		if (node->values.contains("width")) {
			if (_theme->getEvaluator()->hasVar(node->values["width"]) == true)
				width = _theme->getEvaluator()->getVar(node->values["width"]);

			else if (!parseIntegerKey(node->values["width"], 1, &width))
				return parserError("Corrupted width value in key for " + var);
		}

		if (node->values.contains("height")) {
			if (_theme->getEvaluator()->hasVar(node->values["height"]) == true)
				height = _theme->getEvaluator()->getVar(node->values["height"]);

			else if (!parseIntegerKey(node->values["height"], 1, &height))
				return parserError("Corrupted height value in key for " + var);
		}

		Graphics::TextAlign alignH = Graphics::kTextAlignLeft;

		if (node->values.contains("textalign")) {
			if ((alignH = parseTextHAlign(node->values["textalign"])) == Graphics::kTextAlignInvalid)
				return parserError("Invalid value for text alignment.");
		}

		_theme->getEvaluator()->addWidget(var, width, height, node->values["type"], enabled, alignH);
	}

	return true;
}

bool ThemeParser::parserCallback_dialog(ParserNode *node) {
	Common::String var = "Dialog." + node->values["name"];
	bool enabled = true;
	int inset = 0;

	if (resolutionCheck(node->values["resolution"]) == false) {
		node->ignore = true;
		return true;
	}

	if (node->values.contains("enabled")) {
		if (!Common::parseBool(node->values["enabled"], enabled))
			return parserError("Invalid value for Dialog enabling (expecting true/false)");
	}

	if (node->values.contains("inset")) {
		if (!parseIntegerKey(node->values["inset"], 1, &inset))
			return false;
	}

	_theme->getEvaluator()->addDialog(var, node->values["overlays"], enabled, inset);

	if (node->values.contains("shading")) {
		int shading = 0;
		if (node->values["shading"] == "dim")
			shading = 1;
		else if (node->values["shading"] == "luminance")
			shading = 2;
		else return parserError("Invalid value for Dialog background shading.");

		_theme->getEvaluator()->setVar(var + ".Shading", shading);
	}

	return true;
}

bool ThemeParser::parserCallback_import(ParserNode *node) {

	if (!_theme->getEvaluator()->addImportedLayout(node->values["layout"]))
		return parserError("Error importing external layout");
	return true;
}

bool ThemeParser::parserCallback_layout(ParserNode *node) {
	int spacing = -1;
	bool center = false;

	if (node->values.contains("spacing")) {
		if (!parseIntegerKey(node->values["spacing"], 1, &spacing))
			return false;
	}

	Common::parseBool(node->values["center"], center);

	if (node->values["type"] == "vertical")
		_theme->getEvaluator()->addLayout(GUI::ThemeLayout::kLayoutVertical, spacing, center);
	else if (node->values["type"] == "horizontal")
		_theme->getEvaluator()->addLayout(GUI::ThemeLayout::kLayoutHorizontal, spacing, center);
	else
		return parserError("Invalid layout type. Only 'horizontal' and 'vertical' layouts allowed.");


	if (node->values.contains("padding")) {
		int paddingL, paddingR, paddingT, paddingB;

		if (!parseIntegerKey(node->values["padding"], 4, &paddingL, &paddingR, &paddingT, &paddingB))
			return false;

		_theme->getEvaluator()->addPadding(paddingL, paddingR, paddingT, paddingB);
	}

	return true;
}

bool ThemeParser::parserCallback_space(ParserNode *node) {
	int size = -1;

	if (node->values.contains("size")) {
		if (_theme->getEvaluator()->hasVar(node->values["size"]))
			size = _theme->getEvaluator()->getVar(node->values["size"]);

		else if (!parseIntegerKey(node->values["size"], 1, &size))
			return parserError("Invalid value for Spacing size.");
	}

	_theme->getEvaluator()->addSpace(size);
	return true;
}

bool ThemeParser::closedKeyCallback(ParserNode *node) {
	if (node->name == "layout")
		_theme->getEvaluator()->closeLayout();
	else if (node->name == "dialog")
		_theme->getEvaluator()->closeDialog();

	return true;
}

bool ThemeParser::parseCommonLayoutProps(ParserNode *node, const Common::String &var) {
	if (node->values.contains("size")) {
		int width, height;

		if (!parseIntegerKey(node->values["size"], 2, &width, &height)) {
			Common::StringTokenizer tokenizer(node->values["size"], " ,");
			Common::String wtoken, htoken;
			char *parseEnd;

			wtoken = tokenizer.nextToken();

			if (_theme->getEvaluator()->hasVar(wtoken)) {
				width = _theme->getEvaluator()->getVar(wtoken);
			} else {
				width = strtol(wtoken.c_str(), &parseEnd, 10);

				if (*parseEnd != 0 && !(*parseEnd == '%' && *(parseEnd + 1) == 0))
					return false;

				if (wtoken.lastChar() == '%')
					width = g_system->getOverlayWidth() * width / 100;
			}

			htoken = tokenizer.nextToken();

			if (_theme->getEvaluator()->hasVar(htoken)) {
				height = _theme->getEvaluator()->getVar(htoken);
			} else {
				height = strtol(htoken.c_str(), &parseEnd, 10);

				if (*parseEnd != 0 && !(*parseEnd == '%' && *(parseEnd + 1) == 0))
					return false;

				if (htoken.lastChar() == '%')
					height = g_system->getOverlayHeight() * height / 100;
			}

			if (!tokenizer.empty())
				return false;
		}


		_theme->getEvaluator()->setVar(var + "Width", width);
		_theme->getEvaluator()->setVar(var + "Height", height);
	}

	if (node->values.contains("pos")) {
		int x, y;

		if (!parseIntegerKey(node->values["pos"], 2, &x, &y)) {
			Common::StringTokenizer tokenizer(node->values["pos"], " ,");
			Common::String xpos, ypos;
			char *parseEnd;

			xpos = tokenizer.nextToken();

			if (xpos == "center") {
				if (!_theme->getEvaluator()->hasVar(var + "Width"))
					return false;

				x = (g_system->getOverlayWidth() / 2) - (_theme->getEvaluator()->getVar(var + "Width") / 2);

			} else if (_theme->getEvaluator()->hasVar(xpos)) {
				x = _theme->getEvaluator()->getVar(xpos);
			} else {
				x = strtol(xpos.c_str(), &parseEnd, 10);

				if (*parseEnd != 0 && !(*parseEnd == 'r' && *(parseEnd + 1) == 0))
					return false;

				if (xpos.lastChar() == 'r')
					x = g_system->getOverlayWidth() - x;
			}

			ypos = tokenizer.nextToken();

			if (ypos == "center") {
				if (!_theme->getEvaluator()->hasVar(var + "Height"))
					return false;

				y = (g_system->getOverlayHeight() / 2) - (_theme->getEvaluator()->getVar(var + "Height") / 2);

			} else if (_theme->getEvaluator()->hasVar(ypos)) {
				y = _theme->getEvaluator()->getVar(ypos);
			} else {
				y = strtol(ypos.c_str(), &parseEnd, 10);

				if (*parseEnd != 0 && !(*parseEnd == 'b' && *(parseEnd + 1) == 0))
					return false;

				if (ypos.lastChar() == 'b')
					y = g_system->getOverlayHeight() - y;
			}

			if (!tokenizer.empty())
				return false;
		}

		_theme->getEvaluator()->setVar(var + "X", x);
		_theme->getEvaluator()->setVar(var + "Y", y);
	}

	if (node->values.contains("padding")) {
		int paddingL, paddingR, paddingT, paddingB;

		if (!parseIntegerKey(node->values["padding"], 4, &paddingL, &paddingR, &paddingT, &paddingB))
			return false;

		_theme->getEvaluator()->setVar(var + "Padding.Left", paddingL);
		_theme->getEvaluator()->setVar(var + "Padding.Right", paddingR);
		_theme->getEvaluator()->setVar(var + "Padding.Top", paddingT);
		_theme->getEvaluator()->setVar(var + "Padding.Bottom", paddingB);
	}


	if (node->values.contains("textalign")) {
		Graphics::TextAlign alignH = Graphics::kTextAlignLeft;

		if ((alignH = parseTextHAlign(node->values["textalign"])) == Graphics::kTextAlignInvalid)
			return parserError("Invalid value for text alignment.");

		_theme->getEvaluator()->setVar(var + "Align", alignH);
	}
	return true;
}

bool ThemeParser::resolutionCheck(const Common::String &resolution) {
	if (resolution.empty())
		return true;

	Common::StringTokenizer globTokenizer(resolution, ", ");
	Common::String cur;

	while (!globTokenizer.empty()) {
		cur = globTokenizer.nextToken();

		bool lt;
		int val;

		if (cur.size() < 5) {
			warning("Invalid theme 'resolution' token '%s'", resolution.c_str());
			return false;
		}

		if (cur[0] == 'x') {
			val = g_system->getOverlayWidth();
		} else if (cur[0] == 'y') {
			val = g_system->getOverlayHeight();
		} else {
			warning("Error parsing theme 'resolution' token '%s'", resolution.c_str());
			return false;
		}

		if (cur[1] == '<') {
			lt = true;
		} else if (cur[1] == '>') {
			lt = false;
		} else {
			warning("Error parsing theme 'resolution' token '%s'", resolution.c_str());
			return false;
		}

		int token = atoi(cur.c_str() + 2);

		// check inverse for unfulfilled requirements
		if (lt) {
			if (val >= token)
				return false;
		} else {
			if (val <= token)
				return false;
		}
	}

	return true;
}

} // End of namespace GUI
