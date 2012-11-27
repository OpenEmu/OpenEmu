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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_RINGWORLD_SPEAKERS_H
#define TSAGE_RINGWORLD_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class SpeakerGText : public Speaker {
public:
	SceneObject _sceneObject;
public:
	SpeakerGText();

	virtual Common::String getClassName() { return "SpeakerGText"; }
	virtual void setText(const Common::String &msg);
	virtual void removeText();
};

class SpeakerPOR : public AnimatedSpeaker {
	class SpeakerAction1 : public SpeakerAction {
	public:
		virtual void signal();
	};

public:
	SceneObject _object3;
	SpeakerAction1 _action2;
public:
	SpeakerPOR();
	virtual Common::String getClassName() { return "SpeakerPOR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerOR : public AnimatedSpeaker {
public:
	SpeakerOR();
	virtual Common::String getClassName() { return "SpeakerOR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerOText : public SpeakerGText {
public:
	SpeakerOText();

	virtual Common::String getClassName() { return "SpeakerOText"; }
};

class SpeakerPOText : public ScreenSpeaker {
public:
	SpeakerPOText();

	virtual Common::String getClassName() { return "SpeakerPOText"; }
};

class SpeakerSText : public ScreenSpeaker {
public:
	SpeakerSText();

	virtual Common::String getClassName() { return "SpeakerSText"; }
};

class SpeakerQText : public ScreenSpeaker {
public:
	SpeakerQText();

	virtual Common::String getClassName() { return "SpeakerQText"; }
};

class SpeakerMText : public ScreenSpeaker {
public:
	SpeakerMText();

	virtual Common::String getClassName() { return "SpeakerMText"; }
};

class SpeakerCText : public ScreenSpeaker {
public:
	SpeakerCText();

	virtual Common::String getClassName() { return "SpeakerCText"; }
};

class SpeakerEText : public ScreenSpeaker {
public:
	SpeakerEText();

	virtual Common::String getClassName() { return "SpeakerEText"; }
};

class SpeakerGR : public AnimatedSpeaker {
public:
	SpeakerGR();

	virtual Common::String getClassName() { return "SpeakerGR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerHText : public ScreenSpeaker {
public:
	SpeakerHText();

	virtual Common::String getClassName() { return "SpeakerHText"; }
};

class SpeakerPText : public ScreenSpeaker {
public:
	SpeakerPText();

	virtual Common::String getClassName() { return "SpeakerPText"; }
};

class SpeakerCHFText : public ScreenSpeaker {
public:
	SpeakerCHFText();

	virtual Common::String getClassName() { return "SpeakerCHFText"; }
};

class SpeakerSKText : public ScreenSpeaker {
public:
	SpeakerSKText();

	virtual Common::String getClassName() { return "SpeakerSKText"; }
};

class SpeakerCDRText : public ScreenSpeaker {
public:
	SpeakerCDRText();

	virtual Common::String getClassName() { return "SpeakerCDRText"; }
};

class SpeakerFLText : public ScreenSpeaker {
public:
	SpeakerFLText();

	virtual Common::String getClassName() { return "SpeakerFLText"; }
};

class SpeakerBatText : public ScreenSpeaker {
public:
	SpeakerBatText();

	virtual Common::String getClassName() { return "SpeakerFLText"; }
};

class SpeakerQR : public AnimatedSpeaker {
public:
	SpeakerQR();

	virtual Common::String getClassName() { return "SpeakerQR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerQU : public AnimatedSpeaker {
public:
	SpeakerQU();

	virtual Common::String getClassName() { return "SpeakerQU"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSKL : public AnimatedSpeaker {
public:
	SpeakerSKL();

	virtual Common::String getClassName() { return "SpeakerQL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerQL : public AnimatedSpeaker {
public:
	SpeakerQL();

	virtual Common::String getClassName() { return "SpeakerQL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSR : public AnimatedSpeaker {
public:
	SceneObject _object3;
public:
	SpeakerSR();

	virtual Common::String getClassName() { return "SpeakerSR"; }
	void setText(const Common::String &msg);
};

class SpeakerSL : public AnimatedSpeaker {
public:
	SpeakerSL();

	virtual Common::String getClassName() { return "SpeakerSL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerCR : public AnimatedSpeaker {
public:
	SpeakerCR();

	virtual Common::String getClassName() { return "SpeakerCR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerMR : public AnimatedSpeaker {
public:
	SpeakerMR();

	virtual Common::String getClassName() { return "SpeakerMR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSAL : public AnimatedSpeaker {
public:
	SpeakerSAL();

	virtual Common::String getClassName() { return "SpeakerSAL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerML : public AnimatedSpeaker {
public:
	SpeakerML();

	virtual Common::String getClassName() { return "SpeakerML"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerCHFL : public AnimatedSpeaker {
public:
	SpeakerCHFL();

	virtual Common::String getClassName() { return "SpeakerCHFL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerCHFR : public AnimatedSpeaker {
public:
	SpeakerCHFR();

	virtual Common::String getClassName() { return "SpeakerCHFR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerPL : public AnimatedSpeaker {
public:
	SceneObject _object3;
	SpeakerAction _speakerAction2;

	SpeakerPL();

	virtual Common::String getClassName() { return "SpeakerPL"; }
	virtual void setText(const Common::String &msg);
	virtual void removeText();
};

class SpeakerPR : public AnimatedSpeaker {
public:
	SceneObject _object3;
	SpeakerAction _speakerAction2;

	SpeakerPR();

	virtual Common::String getClassName() { return "SpeakerPR"; }
	virtual void setText(const Common::String &msg);
	virtual void removeText();
};

class SpeakerCDR : public AnimatedSpeaker {
public:
	SpeakerCDR();

	virtual Common::String getClassName() { return "SpeakerCDR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerCDL : public AnimatedSpeaker {
public:
	SpeakerCDL();

	virtual Common::String getClassName() { return "SpeakerCDL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerFLL : public AnimatedSpeaker {
public:
	SpeakerFLL();

	virtual Common::String getClassName() { return "SpeakerFLL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerBatR : public AnimatedSpeaker {
public:
	SpeakerBatR();

	virtual Common::String getClassName() { return "SpeakerBatR"; }
	virtual void setText(const Common::String &msg);
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
