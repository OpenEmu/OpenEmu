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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_ADRESPONSE_H
#define WINTERMUTE_ADRESPONSE_H


#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/ad/ad_types.h"

namespace Wintermute {
class BaseFont;
class AdResponse : public BaseObject {
public:
	DECLARE_PERSISTENT(AdResponse, BaseObject)
	bool setIcon(const char *filename);
	bool setFont(const char *filename);
	bool setIconHover(const char *filename);
	bool setIconPressed(const char *filename);
	void setText(const char *text);
	int _iD;
	BaseSprite *_icon;
	BaseSprite *_iconHover;
	BaseSprite *_iconPressed;
	BaseFont *_font;
	char *_text;
	char *_textOrig;
	AdResponse(BaseGame *inGame);
	virtual ~AdResponse();
	TResponseType _responseType;

};

} // end of namespace Wintermute

#endif
