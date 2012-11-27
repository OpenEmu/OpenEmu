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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FBase.h>
#include <FApp.h>
#include <FSystem.h>

#include "backends/platform/bada/portdefs.h"
#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"
#include "backends/platform/bada/application.h"

using namespace Osp::Base;
using namespace Osp::Base::Collection;

C_LINKAGE_BEGIN

_EXPORT_ int OspMain(int argc, char *pArgv[]);

/**
 * The entry function of bada application called by the operating system.
 */
int OspMain(int argc, char *pArgv[]) {
	result r = E_SUCCESS;

	AppLog("Application started.");
	ArrayList *pArgs = new ArrayList();
	pArgs->Construct();

	for (int i = 0; i < argc; i++) {
		pArgs->Add(*(new String(pArgv[i])));
	}

	r = Osp::App::Application::Execute(BadaScummVM::createInstance, pArgs);
	if (IsFailed(r)) {
		r &= 0x0000FFFF;
	}

	pArgs->RemoveAll(true);
	delete pArgs;
	AppLog("Application finished.");

	return static_cast<int>(r);
}

C_LINKAGE_END


