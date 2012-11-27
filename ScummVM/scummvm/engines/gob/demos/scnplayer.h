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

#ifndef GOB_SCNPLAYER_H
#define GOB_SCNPLAYER_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "gob/demos/demoplayer.h"

namespace Gob {

class SCNPlayer : public DemoPlayer {
public:
	SCNPlayer(GobEngine *vm);
	virtual ~SCNPlayer();

protected:
	virtual bool playStream(Common::SeekableReadStream &scn);

private:
	typedef Common::HashMap<Common::String, int32, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> LabelMap;

	bool readLabels(Common::SeekableReadStream &scn, LabelMap &labels);

	void gotoLabel(Common::SeekableReadStream &scn, const LabelMap &labels, const char *label);
};

} // End of namespace Gob

#endif // GOB_SCNPLAYER_H
