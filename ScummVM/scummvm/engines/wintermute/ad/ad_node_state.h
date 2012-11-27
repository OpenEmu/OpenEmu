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

#ifndef WINTERMUTE_ADNODESTATE_H
#define WINTERMUTE_ADNODESTATE_H

namespace Wintermute {

class AdEntity;

class AdNodeState : public BaseClass {
public:
	bool _active;
	bool transferEntity(AdEntity *entity, bool includingSprites, bool saving);
	void setName(const char *name);
	void setFilename(const char *filename);
	void setCursor(const char *filename);
	DECLARE_PERSISTENT(AdNodeState, BaseClass)
	AdNodeState(BaseGame *inGame);
	virtual ~AdNodeState();
	const char *getName() const { return _name; }
private:
	char *_name;
	char *_caption[7];
	void setCaption(const char *caption, int caseVal);
	const char *getCaption(int caseVal);
	uint32 _alphaColor;
	char *_filename;
	char *_cursor;

};

} // end of namespace Wintermute

#endif
