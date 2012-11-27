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

#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/VectorRenderer.h"

#define VECTOR_RENDERER_FAST_TRIANGLES

namespace Graphics {

/********************************************************************
 * DRAWSTEP handling functions
 ********************************************************************/
void VectorRenderer::drawStep(const Common::Rect &area, const DrawStep &step, uint32 extra) {

	if (step.bgColor.set)
		setBgColor(step.bgColor.r, step.bgColor.g, step.bgColor.b);

	if (step.fgColor.set)
		setFgColor(step.fgColor.r, step.fgColor.g, step.fgColor.b);

	if (step.bevelColor.set)
		setBevelColor(step.bevelColor.r, step.bevelColor.g, step.bevelColor.b);

	if (step.gradColor1.set && step.gradColor2.set)
		setGradientColors(step.gradColor1.r, step.gradColor1.g, step.gradColor1.b,
						  step.gradColor2.r, step.gradColor2.g, step.gradColor2.b);

	setShadowOffset(_disableShadows ? 0 : step.shadow);
	setBevel(step.bevel);
	setGradientFactor(step.factor);
	setStrokeWidth(step.stroke);
	setFillMode((FillMode)step.fillMode);

	_dynamicData = extra;

	(this->*(step.drawingCall))(area, step);
}

int VectorRenderer::stepGetRadius(const DrawStep &step, const Common::Rect &area) {
	int radius = 0;

	if (step.radius == 0xFF)
		radius = MIN(area.width(), area.height()) / 2;
	else
		radius = step.radius;

	if (step.scale != (1 << 16) && step.scale != 0)
		radius = (radius * step.scale) >> 16;

	return radius;
}

void VectorRenderer::stepGetPositions(const DrawStep &step, const Common::Rect &area, uint16 &in_x, uint16 &in_y, uint16 &in_w, uint16 &in_h) {
	if (!step.autoWidth) {
		in_w = step.w == -1 ? area.height() : step.w;

		switch (step.xAlign) {
		case Graphics::DrawStep::kVectorAlignManual:
			if (step.x >= 0)
				in_x = area.left + step.x + step.padding.left;
			else
				in_x = area.left + area.width() + step.x + step.padding.left; // value relative to the opposite corner.
			break;

		case Graphics::DrawStep::kVectorAlignCenter:
			in_x = area.left + (area.width() / 2) - (in_w / 2) + ((step.padding.left + step.padding.right ) / 2);
			break;

		case Graphics::DrawStep::kVectorAlignLeft:
			in_x = area.left  + step.padding.left;
			break;

		case Graphics::DrawStep::kVectorAlignRight:
			in_x = area.left + area.width() - in_w - step.padding.right;
			break;

		default:
			error("Vertical alignment in horizontal data");
		}
	} else {
		in_x = area.left + step.padding.left;
		in_w = area.width();
	}

	if (!step.autoHeight) {
		in_h = step.h == -1 ? area.width() : step.h;

		switch (step.yAlign) {
		case Graphics::DrawStep::kVectorAlignManual:
			if (step.y >= 0)
				in_y = area.top + step.y + step.padding.top;
			else
				in_y = area.top + area.height() + step.y + step.padding.top; // relative
			break;

		case Graphics::DrawStep::kVectorAlignCenter:
			in_y = area.top + (area.height() / 2) - (in_h / 2) + ((step.padding.top + step.padding.bottom ) / 2) ;
			break;

		case Graphics::DrawStep::kVectorAlignTop:
			in_y = area.top + step.padding.top;
			break;

		case Graphics::DrawStep::kVectorAlignBottom:
			in_y = area.top + area.height() - in_h - step.padding.bottom;
			break;

		default:
			error("Horizontal alignment in vertical data");
		}
	} else {
		in_y = area.top + step.padding.top;
		in_h = area.height();
	}

	if (step.scale != (1 << 16) && step.scale != 0) {
		in_x = (in_x * step.scale) >> 16;
		in_y = (in_y * step.scale) >> 16;
		in_w = (in_w * step.scale) >> 16;
		in_h = (in_h * step.scale) >> 16;
	}
}

} // End of namespace Graphics
