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

#include "ScummVMapp.h"
#include <scummvm.rsg>
#include <apgcli.h>
#include <eikdll.h>
#include <apgtask.h>

EXPORT_C CApaApplication *NewApplication() {
	return (new CScummVM);
}

CScummVM::CScummVM() {
}

CScummVM::~CScummVM() {
}

CApaDocument *CScummVM::CreateDocumentL() {
	return new (ELeave)CScummVMDoc(*this);
}

TUid CScummVM::AppDllUid() const {
	return TUid::Uid(0x101f9b57);
}

CScummVMDoc::CScummVMDoc(CEikApplication &aApp) : CEikDocument(aApp) {
}

CScummVMDoc::~CScummVMDoc() {
}

CEikAppUi *CScummVMDoc::CreateAppUiL() {
	return new (ELeave)CScummVMUi;
}

void CScummVMUi::HandleForegroundEventL(TBool aForeground) {
	if (aForeground) {
		BringUpEmulatorL();
	}
}

CScummVMUi::CScummVMUi() {
}

CScummVMUi::~CScummVMUi() {
	if (iWatcher) {
		iThreadWatch.LogonCancel(iWatcher->iStatus);
		iWatcher->Cancel();
	}

	delete iWatcher;

	iThreadWatch.Close();
}

void CScummVMUi::ConstructL() {
	BaseConstructL();
	TBuf<128> startFile;
	startFile = iEikonEnv->EikAppUi()->Application()->AppFullName();
	TParse parser;
	parser.Set(startFile,NULL,NULL);

	startFile = parser.DriveAndPath();
#ifndef __WINS__
	startFile.Append( _L("ScummVM.exe"));
#else
	startFile.Append( _L("ScummVM.dll"));
#endif
	CApaCommandLine *cmdLine = CApaCommandLine::NewLC(startFile);
	RApaLsSession lsSession;

	lsSession.Connect();
	CleanupClosePushL(lsSession);
	lsSession.StartApp(*cmdLine, iThreadId);

	CleanupStack::PopAndDestroy();//close lsSession
	CleanupStack::PopAndDestroy(cmdLine);

	User::After(500000);// Let the application start

	TApaTaskList taskList(iEikonEnv->WsSession());

	TApaTask myTask = taskList.FindApp(TUid::Uid(0x101f9b57));
	myTask.SendToBackground();

	TApaTask exeTask = taskList.FindByPos(0);

	iExeWgId=exeTask.WgId();
	exeTask.BringToForeground();

	if (iExeWgId == myTask.WgId()) { // Should n't be the same
		Exit();
	}
	if (iThreadWatch.Open(iThreadId) == KErrNone) {
		iWatcher = new (ELeave)CScummWatcher;
		iWatcher->iAppUi = this;
		iThreadWatch.Logon(iWatcher->iStatus);
	}
}

CScummWatcher::CScummWatcher() : CActive(EPriorityStandard) {
	CActiveScheduler::Add(this);

	iStatus = KRequestPending;
	SetActive();
}

CScummWatcher::~CScummWatcher() {
}

void CScummWatcher::DoCancel() {
}

void CScummWatcher::RunL() {
	iAppUi->HandleCommandL(EEikCmdExit);
}

void CScummVMUi::BringUpEmulatorL() {
	RThread thread;

	if (thread.Open(iThreadId) == KErrNone) {
		thread.Close();
		TApaTask apaTask(iEikonEnv->WsSession());
		apaTask.SetWgId(iExeWgId);
		apaTask.BringToForeground();
	} else {
		iExeWgId = -1;
		Exit();
	}
}

void CScummVMUi::HandleCommandL(TInt aCommand) {
	switch (aCommand) {
	case EEikCmdExit:
		{
			RThread thread;
			if (thread.Open(iThreadId) == KErrNone) {
				thread.Terminate(0);
				thread.Close();
			}
			Exit();
		}
		break;
	}
}

GLDEF_C  TInt E32Dll(TDllReason) {
	return KErrNone;
}
