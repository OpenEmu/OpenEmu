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

#ifndef RIVEN_EXTERNAL_H
#define RIVEN_EXTERNAL_H

#include "mohawk/riven.h"

namespace Mohawk {

#define COMMAND(x) _externalCommands.push_back(new RivenExternalCmd(#x, &RivenExternal::x))

class RivenExternal {
public:
	RivenExternal(MohawkEngine_Riven *vm);
	~RivenExternal();

	void runCommand(uint16 argc, uint16 *argv);
	uint16 getComboDigit(uint32 correctCombo, uint32 digit);
	uint32 getDomeSliderState() { return _sliderState; }
	void setDomeSliderState(uint32 state) { _sliderState = state; }
	void checkYtramCatch(bool playSound);

private:
	MohawkEngine_Riven *_vm;
	uint32 _sliderState;
	Common::Array<Common::Rect> _marbleBaseHotspots;

	typedef void (RivenExternal::*ExternalCmd)(uint16 argc, uint16 *argv);

	struct RivenExternalCmd {
		RivenExternalCmd(const char *d, ExternalCmd p) : desc(d), proc(p) {}
		const char *desc;
		ExternalCmd proc;
	};

	Common::Array<RivenExternalCmd *> _externalCommands;
	void setupCommands();

	// Supplementary Functions
	int jspitElevatorLoop();
	void runDemoBoundaryDialog();
	void runEndGame(uint16 video, uint32 delay);
	void runCredits(uint16 video, uint32 delay);
	void runDomeCheck();
	void runDomeButtonMovie();
	void resetDomeSliders(uint16 soundId, uint16 startHotspot);
	void checkDomeSliders(uint16 resetSlidersHotspot, uint16 openDomeHotspot);
	void checkSliderCursorChange(uint16 startHotspot);
	void dragDomeSlider(uint16 soundId, uint16 resetSlidersHotspot, uint16 openDomeHotspot, uint16 startHotspot);
	void drawDomeSliders(uint16 startHotspot);
	void drawMarbles();
	void setMarbleHotspots();
	void redrawWharkNumberPuzzle(uint16 overlay, uint16 number);
	void lowerPins();

	// -----------------------------------------------------
	// aspit (Main Menu, Books, Setup) external commands
	// Main Menu
	void xastartupbtnhide(uint16 argc, uint16 *argv);
	void xasetupcomplete(uint16 argc, uint16 *argv);
	// Atrus' Journal
	void xaatrusopenbook(uint16 argc, uint16 *argv);
	void xaatrusbookback(uint16 argc, uint16 *argv);
	void xaatrusbookprevpage(uint16 argc, uint16 *argv);
	void xaatrusbooknextpage(uint16 argc, uint16 *argv);
	// Catherine's Journal
	void xacathopenbook(uint16 argc, uint16 *argv);
	void xacathbookback(uint16 argc, uint16 *argv);
	void xacathbookprevpage(uint16 argc, uint16 *argv);
	void xacathbooknextpage(uint16 argc, uint16 *argv);
	// Trap Book
	void xtrapbookback(uint16 argc, uint16 *argv);
	void xatrapbookclose(uint16 argc, uint16 *argv);
	void xatrapbookopen(uint16 argc, uint16 *argv);
	// aspit DVD-specific commands
	void xarestoregame(uint16 argc, uint16 *argv);
	// aspit Demo-specific commands
	void xadisablemenureturn(uint16 argc, uint16 *argv);
	void xaenablemenureturn(uint16 argc, uint16 *argv);
	void xalaunchbrowser(uint16 argc, uint16 *argv);
	void xadisablemenuintro(uint16 argc, uint16 *argv);
	void xaenablemenuintro(uint16 argc, uint16 *argv);
	void xademoquit(uint16 argc, uint16 *argv);
	void xaexittomain(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// bspit (Boiler Island) external commands
	// Gehn's Lab Journal
	void xblabopenbook(uint16 argc, uint16 *argv);
	void xblabbooknextpage(uint16 argc, uint16 *argv);
	void xblabbookprevpage(uint16 argc, uint16 *argv);
	// Boiler Puzzle
	void xsoundplug(uint16 argc, uint16 *argv);
	void xbchangeboiler(uint16 argc, uint16 *argv);
	void xbupdateboiler(uint16 argc, uint16 *argv);
	// Frog Trap
	void xbsettrap(uint16 argc, uint16 *argv);
	void xbcheckcatch(uint16 argc, uint16 *argv);
	void xbait(uint16 argc, uint16 *argv);
	void xbfreeytram(uint16 argc, uint16 *argv);
	void xbaitplate(uint16 argc, uint16 *argv);
	// Dome
	void xbisland190_opencard(uint16 argc, uint16 *argv);
	void xbisland190_resetsliders(uint16 argc, uint16 *argv);
	void xbisland190_slidermd(uint16 argc, uint16 *argv);
	void xbisland190_slidermw(uint16 argc, uint16 *argv);
	void xbscpbtn(uint16 argc, uint16 *argv);
	void xbisland_domecheck(uint16 argc, uint16 *argv);
	// Water Control
	void xvalvecontrol(uint16 argc, uint16 *argv);
	// Run the Wood Chipper
	void xbchipper(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// gspit (Garden Island) external commands
	// Pins
	void xgresetpins(uint16 argc, uint16 *argv);
	void xgrotatepins(uint16 argc, uint16 *argv);
	void xgpincontrols(uint16 argc, uint16 *argv);
	// Dome
	void xgisland25_opencard(uint16 argc, uint16 *argv);
	void xgisland25_resetsliders(uint16 argc, uint16 *argv);
	void xgisland25_slidermd(uint16 argc, uint16 *argv);
	void xgisland25_slidermw(uint16 argc, uint16 *argv);
	void xgscpbtn(uint16 argc, uint16 *argv);
	void xgisland1490_domecheck(uint16 argc, uint16 *argv);
	// Mapping
	void xgplateau3160_dopools(uint16 argc, uint16 *argv);
	// Scribe Taking the Tram
	void xgwt200_scribetime(uint16 argc, uint16 *argv);
	void xgwt900_scribe(uint16 argc, uint16 *argv);
	// Periscope/Prison Viewer
	void xgplaywhark(uint16 argc, uint16 *argv);
	void xgrviewer(uint16 argc, uint16 *argv);
	void xgwharksnd(uint16 argc, uint16 *argv);
	void xglview_prisonoff(uint16 argc, uint16 *argv);
	void xglview_villageoff(uint16 argc, uint16 *argv);
	void xglviewer(uint16 argc, uint16 *argv);
	void xglview_prisonon(uint16 argc, uint16 *argv);
	void xglview_villageon(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// jspit (Jungle Island) external commands
	// Rebel Tunnel Puzzle
	void xreseticons(uint16 argc, uint16 *argv);
	void xicon(uint16 argc, uint16 *argv);
	void xcheckicons(uint16 argc, uint16 *argv);
	void xtoggleicon(uint16 argc, uint16 *argv);
	void xjtunnel103_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel104_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel105_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel106_pictfix(uint16 argc, uint16 *argv);
	// Lower the gallows carriage
	void xvga1300_carriage(uint16 argc, uint16 *argv);
	// Dome
	void xjdome25_resetsliders(uint16 argc, uint16 *argv);
	void xjdome25_slidermd(uint16 argc, uint16 *argv);
	void xjdome25_slidermw(uint16 argc, uint16 *argv);
	void xjscpbtn(uint16 argc, uint16 *argv);
	void xjisland3500_domecheck(uint16 argc, uint16 *argv);
	// Whark Elevator
	void xhandlecontroldown(uint16 argc, uint16 *argv);
	void xhandlecontrolmid(uint16 argc, uint16 *argv);
	void xhandlecontrolup(uint16 argc, uint16 *argv);
	// Beetle
	void xjplaybeetle_550(uint16 argc, uint16 *argv);
	void xjplaybeetle_600(uint16 argc, uint16 *argv);
	void xjplaybeetle_950(uint16 argc, uint16 *argv);
	void xjplaybeetle_1050(uint16 argc, uint16 *argv);
	void xjplaybeetle_1450(uint16 argc, uint16 *argv);
	// Creatures in the Lagoon
	void xjlagoon700_alert(uint16 argc, uint16 *argv);
	void xjlagoon800_alert(uint16 argc, uint16 *argv);
	void xjlagoon1500_alert(uint16 argc, uint16 *argv);
	// Play the Whark Game
	void xschool280_playwhark(uint16 argc, uint16 *argv);
	void xjschool280_resetleft(uint16 argc, uint16 *argv); // DVD only
	void xjschool280_resetright(uint16 argc, uint16 *argv); // DVD only
	// jspit Demo-specific commands
	void xjatboundary(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// ospit (233rd Age / Gehn's Office) external commands
	// Death!
	void xorollcredittime(uint16 argc, uint16 *argv);
	// Trap Book Puzzle
	void xbookclick(uint16 argc, uint16 *argv); // Four params -- movie_sref, start_time, end_time, u0
	// Blank Linking Book
	void xooffice30_closebook(uint16 argc, uint16 *argv);
	// Gehn's Journal
    void xobedroom5_closedrawer(uint16 argc, uint16 *argv);
    void xogehnopenbook(uint16 argc, uint16 *argv);
    void xogehnbookprevpage(uint16 argc, uint16 *argv);
    void xogehnbooknextpage(uint16 argc, uint16 *argv);
	// Elevator Combination
    void xgwatch(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// pspit (Prison Island) external commands
	// Prison Elevator
	void xpisland990_elevcombo(uint16 argc, uint16 *argv);	// Param1: button
	// Dome
	void xpscpbtn(uint16 argc, uint16 *argv);
	void xpisland290_domecheck(uint16 argc, uint16 *argv);
	void xpisland25_opencard(uint16 argc, uint16 *argv);
	void xpisland25_resetsliders(uint16 argc, uint16 *argv);
	void xpisland25_slidermd(uint16 argc, uint16 *argv);
	void xpisland25_slidermw(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// rspit (Rebel Age / Tay) external commands
	void xrcredittime(uint16 argc, uint16 *argv);
	void xrshowinventory(uint16 argc, uint16 *argv);
	void xrhideinventory(uint16 argc, uint16 *argv);
	void xrwindowsetup(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// tspit (Temple Island) external commands
	// Telescope
	void xtexterior300_telescopedown(uint16 argc, uint16 *argv);
	void xtexterior300_telescopeup(uint16 argc, uint16 *argv);
	// Called when clicking the telescope cover buttons. button is the button number (1...5).
	void xtisland390_covercombo(uint16 argc, uint16 *argv);	// Param1: button
	// Atrus' Journal and Trap Book are added to inventory
	void xtatrusgivesbooks(uint16 argc, uint16 *argv);
	// Trap Book is removed from inventory
	void xtchotakesbook(uint16 argc, uint16 *argv);
	void xthideinventory(uint16 argc, uint16 *argv);
	// Marble Puzzle
	void xt7500_checkmarbles(uint16 argc, uint16 *argv);
	void xt7600_setupmarbles(uint16 argc, uint16 *argv);
	void xt7800_setup(uint16 argc, uint16 *argv);
	void xdrawmarbles(uint16 argc, uint16 *argv);
	void xtakeit(uint16 argc, uint16 *argv);
	// Dome
	void xtscpbtn(uint16 argc, uint16 *argv);
	void xtisland4990_domecheck(uint16 argc, uint16 *argv);
	void xtisland5056_opencard(uint16 argc, uint16 *argv);
	void xtisland5056_resetsliders(uint16 argc, uint16 *argv);
	void xtisland5056_slidermd(uint16 argc, uint16 *argv);
	void xtisland5056_slidermw(uint16 argc, uint16 *argv);
	// tspit Demo-specific commands
	void xtatboundary(uint16 argc, uint16 *argv);

	// -----------------------------------------------------
	// Common external commands
	void xflies(uint16 argc, uint16 *argv); // Start the "flies" realtime effect. u0 seems always 0, u1 is a small number (< 10).
};

} // End of namespace Mohawk

#endif
