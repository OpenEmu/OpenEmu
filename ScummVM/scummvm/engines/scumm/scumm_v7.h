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

#ifndef SCUMM_SCRIPT_V7_H
#define SCUMM_SCRIPT_V7_H

#ifdef ENABLE_SCUMM_7_8

#include "scumm/scumm_v6.h"

namespace Scumm {

class Insane;
class SmushMixer;
class SmushPlayer;

class ScummEngine_v7 : public ScummEngine_v6 {
	friend class SmushPlayer;
	friend class Insane;
public:
	ScummEngine_v7(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v7();


protected:
	int _smushFrameRate;

	/**
	 * Flag which signals that the SMUSH video playback should end now
	 * (e.g. because it was aborted by the user or it's simply finished).
	 */
	bool _smushVideoShouldFinish;

	bool _smushActive;

	Insane *_insane;

public:
	SmushMixer *_smixer;
	SmushPlayer *_splayer;


	struct LangIndexNode {
		char tag[12+1];
		int32 offset;
	};

protected:
	int _verbLineSpacing;
	bool _existLanguageFile;
	char *_languageBuffer;
	LangIndexNode *_languageIndex;
	int _languageIndexSize;
	char _lastStringTag[12+1];

#if defined(__SYMBIAN32__) || defined(_WIN32_WCE) // for some reason VC6 cannot find the base class TextObject
	struct SubtitleText {
		int16 xpos, ypos;
		byte color;
		byte charset;
		byte text[256];
		bool actorSpeechMsg;
	};
#else
	struct SubtitleText : TextObject {
		bool actorSpeechMsg;
	};
#endif

	int _subtitleQueuePos;
	SubtitleText _subtitleQueue[20];

public:
	void processSubtitleQueue();
	void addSubtitleToQueue(const byte *text, const Common::Point &pos, byte color, byte charset);
	void clearSubtitleQueue();
	virtual void CHARSET_1();
	bool isSmushActive() { return _smushActive; }

protected:
	virtual int runDialog(Dialog &dialog);

	virtual void scummLoop_handleSound();
	virtual void scummLoop_handleDrawing();
	virtual void processInput();
	virtual void processKeyboard(Common::KeyState lastKeyHit);

	virtual void setupScumm();

	virtual void setupScummVars();
	virtual void resetScummVars();

	virtual void akos_processQueue();

	virtual void saveOrLoad(Serializer *s);

	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();
	virtual void readIndexBlock(uint32 blocktype, uint32 itemsize);

	virtual void setCameraAt(int pos_x, int pos_y);
	virtual void setCameraFollows(Actor *a, bool setCamera = false);
	virtual void moveCamera();
	virtual void panCameraTo(int x, int y);

	virtual int getObjectIdFromOBIM(const byte *obim);

	virtual void actorTalk(const byte *msg);
	virtual void translateText(const byte *text, byte *trans_buff);
	virtual void loadLanguageBundle();
	void playSpeech(const byte *ptr);

	virtual void drawVerb(int verb, int mode);


	virtual void o6_kernelSetFunctions();
};


} // End of namespace Scumm

#endif // ENABLE_SCUMM_7_8

#endif
