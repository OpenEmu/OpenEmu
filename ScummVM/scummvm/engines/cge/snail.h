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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_COMMANDHANDLER_H
#define CGE_COMMANDHANDLER_H

#include "cge/cge.h"

namespace CGE {

#define kCommandFrameRate 80
#define kCommandFrameDelay (1000 / kCommandFrameRate)
#define kDressed 3

enum CommandType {
	kCmdLabel,  kCmdPause,  kCmdWait,        kCmdLevel,       kCmdHide,
	kCmdSay,    kCmdInf,    kCmdTime,        kCmdCave,        kCmdKill,
	kCmdRSeq,   kCmdSeq,    kCmdSend,        kCmdSwap,        kCmdKeep,
	kCmdGive,   kCmdIf,     kCmdGame,        kCmdSetX0,       kCmdSetY0,
	kCmdSlave,  kCmdSetXY,  kCmdRelX,        kCmdRelY,        kCmdRelZ,
	kCmdSetX,   kCmdSetY,   kCmdSetZ,        kCmdTrans,       kCmdPort,
	kCmdNext,   kCmdNNext,  kCmdTNext,       kCmdRNNext,      kCmdRTNext,
	kCmdRMNear, kCmdRmTake, kCmdFlag,        kCmdSetRef,      kCmdBackPt,
	kCmdFlash,  kCmdLight,  kCmdSetVBarrier, kCmdSetHBarrier, kCmdWalk,
	kCmdReach,  kCmdCover,  kCmdUncover,     kCmdClear,       kCmdTalk,
	kCmdMouse,  kCmdSound,  kCmdCount,       kCmdExec,        kCmdStep,
	kCmdZTrim,  kCmdGhost
};

class CommandHandler {
public:
	struct Command {
		CommandType _commandType;
		int _ref;
		int _val;
		void *_spritePtr;
		CallbackType _cbType;
	} *_commandList;
	static const char *_commandText[];
	bool _talkEnable;

	CommandHandler(CGEEngine *vm, bool turbo);
	~CommandHandler();
	void runCommand();
	void addCommand(CommandType com, int ref, int val, void *ptr);
	void addCallback(CommandType com, int ref, int val, CallbackType cbType);
	void insertCommand(CommandType com, int ref, int val, void *ptr);
	bool idle();
	void reset();
private:
	CGEEngine *_vm;
	bool _turbo;
	uint8 _head;
	uint8 _tail;
	bool _busy;
	bool _textDelay;
	uint32 _timerExpiry;
};

} // End of namespace CGE

#endif
