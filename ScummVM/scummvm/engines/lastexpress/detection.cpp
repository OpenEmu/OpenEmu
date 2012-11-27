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

#include "lastexpress/lastexpress.h"
#include "engines/advancedDetector.h"

namespace LastExpress {

static const PlainGameDescriptor lastExpressGames[] = {
	// Games
	{"lastexpress", "The Last Express"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {

	// The Last Express (English) - US Broderbund Release
	//   expressw.exe 1997-02-12 17:24:44
	//   express.exe  1997-02-12 17:29:08
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "2d331459e0e68cf277ef4e4043750413", 29865984},   // 1997-02-10 19:38:19
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},  // 1997-02-10 17:04:40
			{"CD2.HPF", 0, "58aa26e782d10ec5d2231e539d2fe6a2", 669581312},  // 1997-02-10 16:19:30
			{"CD3.HPF", 0, "00554fbf78a2ad391d98578fbbbe1c48", 641128448},  // 1997-02-10 15:44:09
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (English) - UK Broderbund Release
	//   expressw.exe 1997-04-02 14:30:32
	//   express.exe  1997-04-02 15:00:50
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "2d331459e0e68cf277ef4e4043750413", 29865984},   // 1997-04-10 11:03:41
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},  // 1997-04-10 11:03:36
			{"CD2.HPF", 0, "2672348691e1ae22d37d9f46f3683a07", 669509632},  // 1997-04-11 09:48:33
			{"CD3.HPF", 0, "33f5e35f51063cb90f6bed9974475aa6", 641056768},  // 1997-04-11 09:48:55
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (English) - Interplay Release
	//   expressw.exe ???
	//   express.exe  ???
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "bcc32d977f92bb52c060a0b4e8589cac", 30715904},
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},
			{"CD2.HPF", 0, "58aa26e782d10ec5d2231e539d2fe6a2", 669581312},
			{"CD3.HPF", 0, "00554fbf78a2ad391d98578fbbbe1c48", 641128448},
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (Demo - English) - Broderbund
	//   expressw.exe 1997-08-14 14:09:42
	//   express.exe  1997-08-14 14:19:34
	{
		"lastexpress",
		"Demo",
		{
			{"Demo.HPF",  0, "baf3b1f64155d34872896e61c3d3cb78", 58191872},  // 1997-08-14 14:44:26
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (French) - Broderbund Release
	//   expressw.exe 1997-04-02 09:31:24
	//   express.exe  1997-04-02 10:01:12
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "c14c6d685d9bf8705d9f659062e6c5c2", 29505536},   // 1997-04-03 07:53:20
			{"CD1.HPF", 0, "b4277b22bc5cd6ad3b00c2ec04d4645d", 522924032},  // 1997-04-03 07:53:14
			{"CD2.HPF", 0, "8c9610aa4cb707ab51f61c30feb22c1a", 665710592},  // 1997-04-09 12:04:30
			{"CD3.HPF", 0, "411c1bab57b3e8da4fb359c5b40ef5d7", 640884736},  // 1997-04-03 08:21:47
		},
		Common::FR_FRA,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (German)
	//   expressw.exe ???
	//   express.exe  ???
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "7cdd70fc0b1555785f1e9e8d371ea85c", 31301632},   // 1997-04-08 14:33:42
			{"CD1.HPF", 0, "6d74cc861d172466bc745ff8bf0e59c5", 522971136},  // 1997-04-08 13:05:56
			{"CD2.HPF", 0, "b71ac9391de415807c74ff078f4fab22", 655702016},  // 1997-04-08 15:26:14
			{"CD3.HPF", 0, "ee55d4310546dd2a38560b096d1c2771", 641144832},  // 1997-04-05 18:35:50
		},
		Common::DE_DEU,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (Spanish)
	//   expressw.exe 1997-04-02 07:30:32
	//   express.exe  1997-04-02 09:00:50
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "46bed8832f06cf7160883a2aae2d667f", 29657088},   // 1997-09-30 20:43:54
			{"CD1.HPF", 0, "367a3a8581f6f88ddc51af7cde105ba9", 519927808},  // 1997-09-28 19:30:38
			{"CD2.HPF", 0, "af5566df3000472852ec182c9ec57797", 662210560},  // 1997-10-03 05:36:20
			{"CD3.HPF", 0, "0d1901662f4d063a5c250c9fbf64b771", 639504384},  // 1997-09-28 19:55:34
		},
		Common::ES_ESP,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (Italian)
	//   expressw.exe 1997-09-15 12:11:56
	//   express.exe  1997-09-15 12:41:46
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "5539e78fd7eecb70bc858e86b5709fe9", 29562880},   // 1997-12-11 14:11:52
			{"CD1.HPF", 0, "3c1c80b41f2c454b7b89dcb32648796c", 522328064},  // 1997-12-11 14:39:46
			{"CD2.HPF", 0, "ea6414d5a718501cfd55de3884f4431d", 665411584},  // 1997-12-11 15:20:26
			{"CD3.HPF", 0, "a5bd5b58acddbd951d4551f68de22025", 637718528},  // 1997-12-11 15:58:44
		},
		Common::IT_ITA,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// The Last Express (Russian)
	//   expressw.exe 1999-04-05 15:33:56
	//   express.exe  ???
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "a9e915c20f3231c5a1ac4455286971bb", 29908992},   // 1999-04-08 12:43:56
			{"CD1.HPF", 0, "80fbb95c9228353436b7b38e4b5bb64d", 525805568},  // 1999-04-07 13:30:14
			{"CD2.HPF", 0, "a1c8c344754e03eaa86eaabc6024709e", 677289984},  // 1999-04-07 16:19:56
			{"CD3.HPF", 0, "ea5adac447e59ea6d4a1737abad46480", 642584576},  // 1999-04-07 17:26:18
		},
		Common::RU_RUS,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	AD_TABLE_END_MARKER
};


class LastExpressMetaEngine : public AdvancedMetaEngine {
public:
	LastExpressMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), lastExpressGames) {
		_singleid = "lastexpress";
		_guioptions = GUIO2(GUIO_NOSUBTITLES, GUIO_NOSFX);
	}

	const char *getName() const {
		return "Lastexpress";
	}

	const char *getOriginalCopyright() const {
		return "LastExpress Engine (C) 1997 Smoking Car Productions";
	}

protected:
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
};

bool LastExpressMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new LastExpressEngine(syst, (const ADGameDescription *)gd);
	}
	return gd != 0;
}

bool LastExpressEngine::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // End of namespace LastExpress

#if PLUGIN_ENABLED_DYNAMIC(LASTEXPRESS)
	REGISTER_PLUGIN_DYNAMIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#endif
