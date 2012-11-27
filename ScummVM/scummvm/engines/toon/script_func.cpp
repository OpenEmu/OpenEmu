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

#include "common/debug.h"
#include "common/system.h"

#include "toon/script_func.h"
#include "toon/script.h"
#include "toon/state.h"
#include "toon/toon.h"
#include "toon/anim.h"
#include "toon/hotspot.h"
#include "toon/drew.h"
#include "toon/flux.h"

namespace Toon {

#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &ScriptFunc::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

ScriptFunc::ScriptFunc(ToonEngine *vm) {
	Common::Array<const OpcodeV2 *> *table = 0;

	_vm = vm;
	_opcodes.reserve(176);
	SetOpcodeTable(_opcodes);

	Opcode(sys_Cmd_Dummy);                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Change_Actor_X_And_Y);                   //     dd offset sub_2A710
	Opcode(sys_Cmd_Init_Talking_Character);             //     dd offset sub_2A4E0
	Opcode(sys_Cmd_Draw_Actor_Standing),                    //     dd offset sub_2A650
	Opcode(sys_Cmd_Get_Actor_X),                            //     dd offset sub_2ADC0
	Opcode(sys_Cmd_Get_Actor_Y),                            //     dd offset sub_2ADD0
	Opcode(sys_Cmd_Get_Actor_Facing),                       //     dd offset sub_2A790
	Opcode(sys_Cmd_Get_Last_Scene),                     //     dd offset sub_29F80
	Opcode(sys_Cmd_Debug_Print),                            //     dd offset sub_2A510
	Opcode(sys_Cmd_Flip_Screens),                           //     dd offset sub_2A180
	Opcode(sys_Cmd_Play_Flic),                              //     dd offset sub_2A080
	Opcode(sys_Cmd_Force_Facing),                           //     dd offset sub_29F90
	Opcode(sys_Cmd_Restart_Thread),                     //     dd offset sub_29F30
	Opcode(sys_Cmd_Walk_Actor_To_Point),                    //     dd offset sub_2A440
	Opcode(sys_Cmd_Set_Sack_Visible),                       //     dd offset sub_29920
	Opcode(sys_Cmd_Set_Actor_Facing),                       //     dd offset sub_2AD60
	Opcode(sys_Cmd_Confiscate_Inventory),                   //     dd offset sub_29EB0
	Opcode(sys_Cmd_Character_Talks),                        //     dd offset sub_29F00
	Opcode(sys_Cmd_Visited_Scene),                          //     dd offset sub_29E80
	Opcode(sys_Cmd_Query_Rif_Flag),                     //     dd offset sub_29D20
	Opcode(sys_Cmd_Query_Scroll),                           //     dd offset sub_29D60
	Opcode(sys_Cmd_Set_Initial_Location),                   //     dd offset sub_2AD80
	Opcode(sys_Cmd_Make_Line_Non_Walkable),             //     dd offset sub_29FC0
	Opcode(sys_Cmd_Make_Line_Walkable),                 //     dd offset sub_2A050
	Opcode(sys_Cmd_Walk_Actor_On_Condition),                //     dd offset sub_29D70
	Opcode(sys_Cmd_Set_Actor_Facing_Point),             //     dd offset sub_29E60
	Opcode(sys_Cmd_Set_Inventory_Slot),                 //     dd offset sub_2B0D0
	Opcode(sys_Cmd_Get_Inventory_Slot),                 //     dd offset sub_2B0F0
	Opcode(sys_Cmd_Add_Item_To_Inventory),                  //     dd offset sub_2AE50
	Opcode(sys_Cmd_Set_Actor_RGB_Modifiers),                //     dd offset sub_29CA0
	Opcode(sys_Cmd_Init_Conversation_AP),                   //     dd offset sub_2B130
	Opcode(sys_Cmd_Actor_Talks),                            //     dd offset sub_2ADA0
	Opcode(sys_Cmd_Say_Lines),                              //     dd offset sub_29B20
	Opcode(sys_Cmd_Set_Rif_Flag),                           //     dd offset sub_2A320
	Opcode(sys_Cmd_Empty_Inventory),                        //     dd offset sub_2AE10
	Opcode(sys_Cmd_Set_Anim_Scale_Size),                    //     dd offset sub_29BD0
	Opcode(sys_Cmd_Delete_Item_From_Inventory),         //     dd offset sub_2AE70
	Opcode(sys_Cmd_Specific_Item_In_Inventory),         //     dd offset sub_2A740
	Opcode(sys_Cmd_Run_Script),                         //     dd offset sub_29AF0
	Opcode(sys_Cmd_Query_Game_Flag),                        //     dd offset sub_2A3E0
	Opcode(sys_Cmd_Reset_Game_Flag),                        //     dd offset sub_2A420
	Opcode(sys_Cmd_Set_Game_Flag),                          //     dd offset sub_2A400
	Opcode(sys_Cmd_Create_Mouse_Item),                      //     dd offset sub_2A4B0
	Opcode(sys_Cmd_Destroy_Mouse_Item),                 //     dd offset sub_2A4D0
	Opcode(sys_Cmd_Get_Mouse_State),                        //     dd offset sub_2A860
	Opcode(sys_Cmd_Hide_Mouse),                         //     dd offset sub_2A5D0
	Opcode(sys_Cmd_Exit_Conversation),                      //     dd offset sub_29AE0
	Opcode(sys_Cmd_Set_Mouse_Pos),                          //     dd offset sub_2A810
	Opcode(sys_Cmd_Show_Mouse),                         //     dd offset sub_2A5F0
	Opcode(sys_Cmd_In_Close_Up),                            //     dd offset sub_29FB0
	Opcode(sys_Cmd_Set_Scroll_Lock),                        //     dd offset sub_298B0
	Opcode(sys_Cmd_Fill_Area_Non_Walkable),             //     dd offset sub_29FF0
	Opcode(sys_Cmd_Set_Scroll_Coords),                      //     dd offset sub_298D0
	Opcode(sys_Cmd_Hide_Cutaway),                           //     dd offset sub_2A0F0
	Opcode(sys_Cmd_Show_Cutaway),                           //     dd offset sub_2A100
	Opcode(sys_Cmd_Pause_Ticks),                            //     dd offset sub_2A360
	Opcode(sys_Cmd_In_Conversation),                        //     dd offset sub_29C60
	Opcode(sys_Cmd_Character_Talking),                      //     dd offset sub_29C70
	Opcode(sys_Cmd_Set_Flux_Facing_Point),                  //     dd offset sub_29980
	Opcode(sys_Cmd_Set_Flux_Facing),                        //     dd offset sub_299A0
	Opcode(sys_Cmd_Set_Flux_Coords),                        //     dd offset sub_299C0
	Opcode(sys_Cmd_Set_Flux_Visible),                       //     dd offset sub_299F0
	Opcode(sys_Cmd_Get_Flux_X),                         //     dd offset sub_29A40
	Opcode(sys_Cmd_Get_Flux_Y),                         //     dd offset sub_29A50
	Opcode(sys_Cmd_Get_Flux_Facing),                        //     dd offset sub_29A60
	Opcode(sys_Cmd_Get_Flux_Flags),                     //     dd offset sub_29A70
	Opcode(sys_Cmd_Query_Flux_Coords),                      //     dd offset sub_29A90
	Opcode(sys_Cmd_Have_A_Conversation),                    //     dd offset sub_2B110
	Opcode(sys_Cmd_Walk_Flux_To_Point),                 //     dd offset sub_29AC0
	Opcode(sys_Cmd_Get_Actor_Final_X),                      //     dd offset sub_29940
	Opcode(sys_Cmd_Get_Actor_Final_Y),                      //     dd offset sub_29960
	Opcode(sys_Cmd_Query_Scene_Anim_Loaded),                //     dd offset sub_29870
	Opcode(sys_Cmd_Play_Flux_Anim),                     //     dd offset sub_29820
	Opcode(sys_Cmd_Set_Anim_Priority),                      //     dd offset sub_29790
	Opcode(sys_Cmd_Place_Scene_Anim),                       //     dd offset sub_2A7A0
	Opcode(sys_Cmd_Update_Scene_Animations),                //     dd offset sub_2AE30
	Opcode(sys_Cmd_Get_Drew_Scale),                     //     dd offset sub_297E0
	Opcode(sys_Cmd_Query_Drew_Flags),                       //     dd offset sub_29800
	Opcode(sys_Cmd_Set_Music),                              //     dd offset sub_29720
	Opcode(sys_Cmd_Query_Speech),                           //     dd offset sub_296D0
	Opcode(sys_Cmd_Enter_New_Scene),                        //     dd offset sub_2A550
	Opcode(sys_Cmd_Enter_Same_Scene),                       //     dd offset sub_2ADE0
	Opcode(sys_Cmd_Is_Pixel_Walkable),                      //     dd offset sub_2A4F0
	Opcode(sys_Cmd_Show_Screen),                            //     dd offset sub_2A0C0
	Opcode(sys_Cmd_Hide_Screen),                            //     dd offset sub_2A0F0
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_295D0
	Opcode(sys_Cmd_Set_Special_Enter_X_And_Y),              //     dd offset sub_2A590
	Opcode(sys_Cmd_Get_Mouse_X),                            //     dd offset sub_296B0
	Opcode(sys_Cmd_Get_Mouse_Y),                            //     dd offset sub_296C0
	Opcode(sys_Cmd_Fade_Palette),                           //     dd offset sub_29650
	Opcode(sys_Cmd_Music_Enabled),                          //     dd offset sub_29620
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_295F0
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_29610
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Random),                             //     dd offset sub_2A600
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Wait_Key),                               //     dd offset sub_2AE20
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Draw_Scene_Anim_WSA_Frame_To_Back),      //     dd offset sub_2A940
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Set_Scene_Anim_Wait),                    //     dd offset sub_2A870
	Opcode(sys_Cmd_Init_Scene_Anim),                        //     dd offset sub_2AC60
	Opcode(sys_Cmd_Set_Scene_Animation_Active_Flag),        //     dd offset sub_2AB10
	Opcode(sys_Cmd_Draw_Scene_Anim_WSA_Frame),              //     dd offset sub_2A8D0
	Opcode(sys_Cmd_Move_Scene_Anim),                        //     dd offset sub_2AA90
	Opcode(sys_Cmd_Run_Actor_Default_Script),               //     dd offset sub_2A4E0
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Set_Location_Data),                      //     dd offset sub_2AE90
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Set_CountDown_Timer),                    //     dd offset sub_2AFC0
	Opcode(sys_Cmd_Query_CountDown_Timer),                  //     dd offset sub_2AFE0
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Proceed_To_Next_Chapter),                //     dd offset sub_2AFF0
	Opcode(sys_Cmd_Play_Sfx_Plus),                          //     dd offset sub_2A1D0
	Opcode(sys_Cmd_Play_Sfx),                               //     dd offset sub_2A1A0
	Opcode(sys_Cmd_Set_Ambient_Sfx),                        //     dd offset sub_2A260
	Opcode(sys_Cmd_Kill_Ambient_Sfx),                       //     dd offset sub_2A300
	Opcode(sys_Cmd_Set_Ambient_Sfx_Plus),                   //     dd offset sub_2A290
	Opcode(sys_Cmd_Set_Ambient_Volume),                 //     dd offset sub_2A240
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Freeze_Scene_Animation),             //     dd offset sub_2AB90
	Opcode(sys_Cmd_Unfreeze_Scene_Animation),               //     dd offset sub_2ABB0
	Opcode(sys_Cmd_Scene_Animation_Frozen),             //     dd offset sub_2ABD0
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Set_Script_Game_Data_Global),            //     dd offset sub_2ABF0
	Opcode(sys_Cmd_Get_Script_Game_Data_Global),            //     dd offset sub_2AC30
	Opcode(sys_Cmd_Say_Line),                               //     dd offset loc_2A190
	Opcode(sys_Cmd_Knight_Puzzle_Get_Coord),                //     dd offset sub_2A110
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Add_Scene_Anim),                     //     dd offset sub_2AC60
	Opcode(sys_Cmd_Remove_Scene_Anim),                      //     dd offset sub_2ACE0
	Opcode(sys_Cmd_Disable_Timer),                          //     dd offset sub_2AD00
	Opcode(sys_Cmd_Enable_Timer),                           //     dd offset sub_2AD20
	Opcode(sys_Cmd_Set_Timer),                              //     dd offset sub_2AD40
	Opcode(sys_Cmd_Set_Palette_Color),                      //     dd offset sub_2B020
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Number_Of_NPCs),                     //     dd offset loc_2A190
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Dummy),                                  //     dd offset sub_2B160
	Opcode(sys_Cmd_Get_Config_Language),                    //     dd offset sub_2B0C0
	Opcode(sys_Cmd_Dummy);                                  //     dd offset sub_2B160
}

ScriptFunc::~ScriptFunc(void) {
	while (!_opcodes.empty()) {
		const OpcodeV2 *temp = _opcodes.back();
		_opcodes.pop_back();
		delete temp;
	}
}

char *GetText(int32 i, EMCState *state) {
	short stack = stackPos(i);
	unsigned short textoffset = READ_BE_UINT16(&((unsigned short *)(state->dataPtr->text))[stack]);
	char *text = (char *)&state->dataPtr->text[textoffset];
	return text;
}

int32 ScriptFunc::sys_Cmd_Dummy(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Change_Actor_X_And_Y(EMCState *state) {
	_vm->getDrew()->forcePosition(stackPos(0), stackPos(1));
	//_vm->getDrew()->resetScale();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Init_Talking_Character(EMCState *state) {
	// really does nothing in original
	return 0;
}

int32 ScriptFunc::sys_Cmd_Draw_Actor_Standing(EMCState *state) {

	int32 arg1 = stackPos(0);
	int32 arg2 = stackPos(1);
	int32 arg3 = stackPos(2);

	// WORKAROUND: In scene 19 (transformed barn), Drew disappears when it shouldn't. It seems like a script bug
	//				even if the game works correctly at this point
	//				We need a special case for it then.
	if (_vm->state()->_currentScene == 19 && arg3 == 1 && arg1 < 0) {
		arg1 = 1;
	}

	if (arg2 > -1)
		_vm->getDrew()->forceFacing(arg2);

	if (arg1 < 0) {
		_vm->getDrew()->setVisible(false);
	} else {
		_vm->getDrew()->setVisible(true);
		_vm->getDrew()->playStandingAnim();
	}
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Actor_X(EMCState *state) {
	return _vm->getDrew()->getX();
}

int32 ScriptFunc::sys_Cmd_Get_Actor_Y(EMCState *state) {
	return _vm->getDrew()->getY();
}

int32 ScriptFunc::sys_Cmd_Get_Actor_Final_X(EMCState *state) {
	return _vm->getDrew()->getFinalX();
}

int32 ScriptFunc::sys_Cmd_Get_Actor_Final_Y(EMCState *state) {
	return _vm->getDrew()->getFinalY();
}

int32 ScriptFunc::sys_Cmd_Get_Actor_Facing(EMCState *state) {
	return _vm->getDrew()->getFacing();
}

int32 ScriptFunc::sys_Cmd_Get_Last_Scene(EMCState *state) {
	return _vm->state()->_lastVisitedScene;
}

int32 ScriptFunc::sys_Cmd_Debug_Print(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Flip_Screens(EMCState *state) {
	_vm->flipScreens();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Play_Flic(EMCState *state) {

	Common::String name;

	// workaround for the video of the beginning
	if (strstr(GetText(0, state), "209"))
		name = GetText(0, state);
	else
		name = _vm->createRoomFilename(GetText(0, state));

	int32 stopMusic = stackPos(2);
	_vm->getMoviePlayer()->play(name, stopMusic);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Force_Facing(EMCState *state) {
	_vm->getDrew()->setFacing(stackPos(0));
	_vm->getDrew()->playStandingAnim();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Restart_Thread(EMCState *state) {

	int32 sceneId = stackPos(0);
	_vm->getScript()->init(&_vm->getSceneAnimationScript(sceneId)->_state, _vm->getSceneAnimationScript(sceneId)->_data);
	_vm->getScript()->start(&_vm->getSceneAnimationScript(sceneId)->_state, 9 + sceneId);

	if (!stackPos(1))
		_vm->setSceneAnimationScriptUpdate(false);

	return 0;
}

int32 ScriptFunc::sys_Cmd_Walk_Actor_To_Point(EMCState *state) {
	return _vm->getDrew()->walkTo(stackPos(0), stackPos(1));
}

int32 ScriptFunc::sys_Cmd_Set_Sack_Visible(EMCState *state) {
	_vm->state()->_sackVisible = stackPos(0) > 0;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Actor_Facing(EMCState *state) {
	_vm->getDrew()->forceFacing(stackPos(0));
	_vm->getDrew()->playStandingAnim();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Confiscate_Inventory(EMCState *state) {
	for (int32 i = 0; i < _vm->state()->_numInventoryItems; i++) {
		_vm->state()->_confiscatedInventory[_vm->state()->_numConfiscatedInventoryItems] = _vm->state()->_inventory[i];
		_vm->state()->_numConfiscatedInventoryItems++;
	}
	_vm->state()->_numInventoryItems = 0;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Character_Talks(EMCState *state) {
	_vm->characterTalk(stackPos(0), false);
	//_vm->characterTalk(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Visited_Scene(EMCState *state) {
	return _vm->state()->_locations[stackPos(0)]._visited ? 1 : 0;
}

int32 ScriptFunc::sys_Cmd_Query_Rif_Flag(EMCState *state) {

	int32 hs = _vm->getHotspots()->FindBasedOnCorner(stackPos(0), stackPos(1));
	if (hs >= 0)
		return _vm->getHotspots()->Get(hs)->getData(stackPos(2));

	return 0;
}

int32 ScriptFunc::sys_Cmd_Query_Scroll(EMCState *state) {
	return _vm->state()->_currentScrollValue;
}

int32 ScriptFunc::sys_Cmd_Set_Initial_Location(EMCState *state) {
	int32 initialLocation = stackPos(0);
	_vm->state()->_currentScene = initialLocation;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Make_Line_Non_Walkable(EMCState *state) {
	_vm->makeLineNonWalkable(stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	// we have to store some info for savegame
	_vm->getSaveBufferStream()->writeSint16BE(2); // 2 = sys_Cmd_Make_Line_Non_Walkable
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(0));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(1));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(2));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Make_Line_Walkable(EMCState *state) {
	_vm->makeLineWalkable(stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	// we have to store some info for savegame
	_vm->getSaveBufferStream()->writeSint16BE(3); // 3 = sys_Cmd_Make_Line_Walkable
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(0));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(1));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(2));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Walk_Actor_On_Condition(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Actor_Facing_Point(EMCState *state) {
	int32 fx = stackPos(0);
	int32 fy = stackPos(1);
	_vm->getDrew()->setFacing(_vm->getDrew()->getFacingFromDirection(fx - _vm->getDrew()->getX(), fy - _vm->getDrew()->getY()));
	return 1;
}

int32 ScriptFunc::sys_Cmd_Set_Inventory_Slot(EMCState *state) {
	_vm->state()->_inventory[stackPos(1)] = stackPos(0);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Inventory_Slot(EMCState *state) {
	return _vm->state()->_inventory[stackPos(0)];
}

int32 ScriptFunc::sys_Cmd_Add_Item_To_Inventory(EMCState *state) {
	_vm->addItemToInventory(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Actor_RGB_Modifiers(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Init_Conversation_AP(EMCState *state) {
	_vm->initCharacter(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Actor_Talks(EMCState *state) {
	_vm->characterTalk(stackPos(0), false);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Say_Lines(EMCState *state) {

	// WORKAROUND: In the scene 4 (Castle), if you click twice on the closed door, Drew disappears
	//				the script makes him disappear for the custom animation and not reappear.
	if (_vm->state()->_currentScene == 4 && stackPos(1) == 562) {
		_vm->getDrew()->setVisible(true);
	}

	_vm->sayLines(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Rif_Flag(EMCState *state) {
	int32 hs = _vm->getHotspots()->FindBasedOnCorner(stackPos(0), stackPos(1));
	if (hs >= 0)
		_vm->getHotspots()->Get(hs)->setData(stackPos(2), stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Empty_Inventory(EMCState *state) {

	for (int32 i = 0; i < _vm->state()->_numInventoryItems; i++)
		_vm->state()->_inventory[i] = 0;

	_vm->state()->_numInventoryItems = 0;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Anim_Scale_Size(EMCState *state) {
	int32 animID = stackPos(0);
	int32 scale = stackPos(1);

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animID);
	if (sceneAnim) {
		sceneAnim->_animInstance->setUseMask(true);
		sceneAnim->_animInstance->setScale(scale, true);
	}
	return 0;
}
int32 ScriptFunc::sys_Cmd_Delete_Item_From_Inventory(EMCState *state) {
	for (int32 i = 0; i < _vm->state()->_numInventoryItems; i++) {
		if (stackPos(0) == _vm->state()->_inventory[i])
			_vm->state()->_inventory[i] = 0;
	}
	_vm->rearrangeInventory();
	return 0;
}
int32 ScriptFunc::sys_Cmd_Specific_Item_In_Inventory(EMCState *state) {
	for (int32 i = 0; i < _vm->state()->_numInventoryItems; i++) {
		if (_vm->state()->_inventory[i] == stackPos(0))
			return 1;
	}
	if (_vm->state()->_mouseState == stackPos(0))
		return 1;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Run_Script(EMCState *state) {
	return _vm->runEventScript(_vm->getMouseX(), _vm->getMouseY(), 2, stackPos(0), 0);
}

int32 ScriptFunc::sys_Cmd_Query_Game_Flag(EMCState *state) {
	int32 arg = stackPos(0);
	return (_vm->state()->_gameFlag[arg >> 3] & (1 << (arg & 7))) != 0;
}

int32 ScriptFunc::sys_Cmd_Reset_Game_Flag(EMCState *state) {
	int32 arg = stackPos(0);
	_vm->state()->_gameFlag[arg >> 3] &= ~(1 << (arg & 7));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Game_Flag(EMCState *state) {
	int32 arg = stackPos(0);
	_vm->state()->_gameFlag[arg >> 3] |= (1 << (arg & 7));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Create_Mouse_Item(EMCState *state) {
	_vm->createMouseItem(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Destroy_Mouse_Item(EMCState *state) {
	_vm->deleteMouseItem();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Mouse_State(EMCState *state) {
	return _vm->state()->_mouseState;
}

int32 ScriptFunc::sys_Cmd_Hide_Mouse(EMCState *state) {
	_vm->state()->_mouseHidden = true;
	//if (Game.MouseHiddenCount > 0) Game.MouseHiddenCount = 1;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Exit_Conversation(EMCState *state) {
	_vm->state()->_exitConversation = true;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Mouse_Pos(EMCState *state) {
	if (_vm->state()->_inCloseUp) {
		_vm->_system->warpMouse(stackPos(0), stackPos(1));
	} else {
		_vm->_system->warpMouse(stackPos(0) - _vm->state()->_currentScrollValue, stackPos(1));
	}
	return 0;
}

int32 ScriptFunc::sys_Cmd_Show_Mouse(EMCState *state) {
	_vm->state()->_mouseHidden = false;
	//if (Game.MouseHiddenCount < 0) Game.MouseHiddenCount = 0;
	return 0;
}

int32 ScriptFunc::sys_Cmd_In_Close_Up(EMCState *state) {
	return _vm->state()->_inCloseUp;
}

int32 ScriptFunc::sys_Cmd_Set_Scroll_Lock(EMCState *state) {
	_vm->state()->_currentScrollLock = stackPos(0) > 0;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Fill_Area_Non_Walkable(EMCState *state) {
	_vm->getMask()->floodFillNotWalkableOnMask(stackPos(0), stackPos(1));

	// we have to store some info for savegame
	_vm->getSaveBufferStream()->writeSint16BE(4); // 4 = sys_Cmd_Make_Line_Walkable
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(0));
	_vm->getSaveBufferStream()->writeSint16BE(stackPos(1));

	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Scroll_Coords(EMCState *state) {
	_vm->state()->_currentScrollValue = stackPos(0);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Hide_Cutaway(EMCState *state) {
	_vm->hideCutaway();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Show_Cutaway(EMCState *state) {
	_vm->showCutaway("");
	return 0;
}

int32 ScriptFunc::sys_Cmd_Pause_Ticks(EMCState *state) {

	if (!_vm->isUpdatingSceneAnimation() || _vm->getScriptRegionNested() > 0) {
		if (stackPos(1))
			_vm->waitTicks(stackPos(0), true);
		else
			_vm->waitTicks(stackPos(0), false);
	} else {
		uint32 sceneId = _vm->getCurrentUpdatingSceneAnimation();
		uint32 waitTicks = stackPos(0);
		if (waitTicks < 1) waitTicks = 1;

		waitTicks *= _vm->getTickLength();

		if (sceneId < 40) {
			int32 nextTicks = waitTicks + _vm->getSceneAnimationScript(sceneId)->_lastTimer;
			if (nextTicks < _vm->getOldMilli())
				_vm->getSceneAnimationScript(sceneId)->_lastTimer = _vm->getOldMilli() + waitTicks;
			else
				_vm->getSceneAnimationScript(sceneId)->_lastTimer = nextTicks;
		}
		return 0;
	}

	return 0;
}

int32 ScriptFunc::sys_Cmd_In_Conversation(EMCState *state) {
	return _vm->state()->_inConversation;
}

int32 ScriptFunc::sys_Cmd_Character_Talking(EMCState *state) {
	int32 characterId = stackPos(0);
	Character *character = _vm->getCharacterById(characterId);
	if (character)
		return character->isTalking();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Flux_Facing_Point(EMCState *state) {
	int32 fx = stackPos(0);
	int32 fy = stackPos(1);
	_vm->getFlux()->setFacing(_vm->getFlux()->getFacingFromDirection(fx - _vm->getFlux()->getX(), fy - _vm->getFlux()->getY()));
	if (_vm->getFlux()->getFlag() == 0)  // don't reset the animation unless Flux is in idle mode
		_vm->getFlux()->playStandingAnim();
	return 1;
}

int32 ScriptFunc::sys_Cmd_Set_Flux_Facing(EMCState *state) {
	_vm->getFlux()->forceFacing(stackPos(0));
	if (_vm->getFlux()->getFlag() == 0) // don't reset the animation unless Flux is in idle mode
		_vm->getFlux()->playStandingAnim();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Flux_Coords(EMCState *state) {
	_vm->getFlux()->stopWalk();
	_vm->getFlux()->forcePosition(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Flux_Visible(EMCState *state) {
	_vm->getFlux()->setVisible(stackPos(0) > 0);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Flux_X(EMCState *state) {
	return _vm->getFlux()->getX();
}

int32 ScriptFunc::sys_Cmd_Get_Flux_Y(EMCState *state) {
	return _vm->getFlux()->getY();
}

int32 ScriptFunc::sys_Cmd_Get_Flux_Facing(EMCState *state) {
	return _vm->getFlux()->getFacing();
}

int32 ScriptFunc::sys_Cmd_Get_Flux_Flags(EMCState *state) {
	return (_vm->getFlux()->getFlag() & stackPos(0)) != 0;
}

int32 ScriptFunc::sys_Cmd_Query_Flux_Coords(EMCState *state) {
	return (stackPos(0) == _vm->getFlux()->getX()) && (stackPos(1) == _vm->getFlux()->getY());
}

int32 ScriptFunc::sys_Cmd_Have_A_Conversation(EMCState *state) {
	_vm->haveAConversation(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Walk_Flux_To_Point(EMCState *state) {
	_vm->getFlux()->walkTo(stackPos(0), stackPos(1));
	return 1;
}

int32 ScriptFunc::sys_Cmd_Query_Scene_Anim_Loaded(EMCState *state) {
	return _vm->getSceneAnimation(stackPos(0))->_active;
}

int32 ScriptFunc::sys_Cmd_Play_Flux_Anim(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Anim_Priority(EMCState *state) {

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(stackPos(0));
	sceneAnim->_animInstance->setLayerZ(stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Place_Scene_Anim(EMCState *state) {
	int32 sceneId = stackPos(0);
	int32 x = stackPos(1);
	int32 y = stackPos(2);
	int32 frame = stackPos(5);

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(sceneId);
	sceneAnim->_animInstance->setPosition(x, y, sceneAnim->_animInstance->getZ(), false);
	sceneAnim->_animInstance->forceFrame(frame);
	_vm->setSceneAnimationScriptUpdate(false);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Update_Scene_Animations(EMCState *state) {
	//debugC(0, 0xfff, "UpdateAnimations");
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Drew_Scale(EMCState *state) {
	int32 scale = _vm->getDrew()->getScale();
	if (!scale)
		return 1024;
	return scale;
}

int32 ScriptFunc::sys_Cmd_Query_Drew_Flags(EMCState *state) {
	return (_vm->getDrew()->getFlag() & stackPos(0)) != 0;
}

int32 ScriptFunc::sys_Cmd_Set_Music(EMCState *state) {

	char *newMus = GetText(0, state);
	_vm->getAudioManager()->playMusic(_vm->state()->_locations[_vm->state()->_currentScene]._name, newMus);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Query_Speech(EMCState *state) {
	if (_vm->getAudioManager()->voiceStillPlaying())
		return 1;
	else
		return 0;
}

int32 ScriptFunc::sys_Cmd_Enter_New_Scene(EMCState *state) {
	_vm->exitScene();
	_vm->getDrew()->forceFacing(stackPos(1));
	_vm->loadScene(stackPos(0));
	_vm->setSceneAnimationScriptUpdate(false);

	return 0;
}

int32 ScriptFunc::sys_Cmd_Enter_Same_Scene(EMCState *state) {
	_vm->exitScene();
	_vm->loadScene(_vm->state()->_currentScene);
	_vm->setSceneAnimationScriptUpdate(false);

	return 0;
}

int32 ScriptFunc::sys_Cmd_Is_Pixel_Walkable(EMCState *state) {
	return (_vm->getMask()->getData(stackPos(0), stackPos(1)) & 0x1f) > 0;
}

int32 ScriptFunc::sys_Cmd_Show_Screen(EMCState *state) {
	_vm->showCutaway(GetText(0, state));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Hide_Screen(EMCState *state) {
	_vm->hideCutaway();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Special_Enter_X_And_Y(EMCState *state) {
	_vm->state()->_nextSpecialEnterX = stackPos(0);
	_vm->state()->_nextSpecialEnterY = stackPos(1);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Mouse_X(EMCState *state) {
	return _vm->getMouseX();
}

int32 ScriptFunc::sys_Cmd_Get_Mouse_Y(EMCState *state) {
	return _vm->getMouseY();
}

int32 ScriptFunc::sys_Cmd_Fade_Palette(EMCState *state) {
	debugC(0, 0xfff, "fadePalette %d %d", stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Music_Enabled(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Random(EMCState *state) {
	int32 randMin = stackPos(0);
	int32 randMax = stackPos(1);
	int32 t = 0;

	if (randMin > randMax) {
		t = randMin;
		randMin = randMax;
		randMax = t;
	}
	return _vm->randRange(randMin, randMax);
}

int32 ScriptFunc::sys_Cmd_Wait_Key(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Draw_Scene_Anim_WSA_Frame_To_Back(EMCState *state) {
	// draw the frame in the backbuffer (picture then)

	int32 animId = stackPos(0);
	int32 frame = stackPos(1);

	if (frame < 0)
		return 0;

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animId);

	if (sceneAnim->_active) {
		sceneAnim->_animInstance->setFrame(frame);
		sceneAnim->_animInstance->setAnimationRange(frame, frame);
		sceneAnim->_animInstance->stopAnimation();
		sceneAnim->_animInstance->renderOnPicture();

		// we have to store some info for savegame
		_vm->getSaveBufferStream()->writeSint16BE(1); // 1 = Draw_Scene_Anim_WSA_Frame_To_Back
		_vm->getSaveBufferStream()->writeSint16BE(frame);
		_vm->getSaveBufferStream()->writeSint16BE(strlen(sceneAnim->_animInstance->getAnimation()->_name) + 1);
		_vm->getSaveBufferStream()->write(sceneAnim->_animInstance->getAnimation()->_name, strlen(sceneAnim->_animInstance->getAnimation()->_name) + 1);
		_vm->getSaveBufferStream()->writeSint16BE(sceneAnim->_animInstance->getX());
		_vm->getSaveBufferStream()->writeSint16BE(sceneAnim->_animInstance->getY());
		_vm->getSaveBufferStream()->writeSint16BE(sceneAnim->_animInstance->getZ());
		_vm->getSaveBufferStream()->writeSint16BE(sceneAnim->_animInstance->getLayerZ());

	}
	return 1;
}

int32 ScriptFunc::sys_Cmd_Set_Scene_Anim_Wait(EMCState *state) {
	int32 sceneId = stackPos(0);
	int32 waitTicks = stackPos(1);
	if (waitTicks < 1) waitTicks = 1;

	// WORKAROUND : To fix the timing problem in the Gift-O-Matic
	//              The animation was too fast and the player was unable to click fast enough to get objects
	//              Here we slow down the animation
	if (_vm->state()->_currentScene == 24) {
		if (_vm->getCurrentUpdatingSceneAnimation() == 6) {
			if (waitTicks == 1) {
				waitTicks = 10;
				_vm->setSceneAnimationScriptUpdate(false);
			}
		}
	}

	// WORKAROUND : In Wolf place, the animation to move the pot was too fast and the player was unable to
	//              progress into the game.
	if (_vm->state()->_currentScene == 29) {
		if (_vm->getCurrentUpdatingSceneAnimation() == 8 || _vm->getCurrentUpdatingSceneAnimation() == 7) {
			if (waitTicks == 1) {
				waitTicks = 5;
				_vm->setSceneAnimationScriptUpdate(false);
			}
		}
	}

	// WORKAROUND : In transformed hangar, everything is too fast..
	if (_vm->state()->_currentScene == 19) {
		waitTicks = 10;
		_vm->setSceneAnimationScriptUpdate(false);
	}

	// WORKAROUND : Slow down just a little the guards dance animation so that the voices don't cut
	if (_vm->state()->_currentScene == 2 && (sceneId == 2 || sceneId == 3)) {
		waitTicks = 7;
		_vm->setSceneAnimationScriptUpdate(false);
	}

	waitTicks *= _vm->getTickLength();

	if (sceneId >= 0 && sceneId < 40) {
		int32 nextTicks = waitTicks + _vm->getSceneAnimationScript(sceneId)->_lastTimer;
		if (nextTicks < _vm->getOldMilli())
			_vm->getSceneAnimationScript(sceneId)->_lastTimer = _vm->getOldMilli() + waitTicks;
		else
			_vm->getSceneAnimationScript(sceneId)->_lastTimer = nextTicks;
	}
	return 0;
}

int32 ScriptFunc::sys_Cmd_Init_Scene_Anim(EMCState *state) {
	int32 animId = stackPos(0);
	int32 flags = stackPos(1);

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animId);
	if (sceneAnim->_active)
		return 0;

	delete sceneAnim->_animation;
	delete sceneAnim->_animInstance;

	sceneAnim->_animation = new Animation(_vm);
	sceneAnim->_animation->loadAnimation(GetText(12, state));
	sceneAnim->_animInstance = _vm->getAnimationManager()->createNewInstance(kAnimationScene);
	sceneAnim->_originalAnimInstance = sceneAnim->_animInstance;
	sceneAnim->_animInstance->setAnimation(sceneAnim->_animation);
	sceneAnim->_animInstance->setVisible((flags & 1) != 0);
	sceneAnim->_animInstance->setAnimationRange(stackPos(11), stackPos(11));
	sceneAnim->_animInstance->setFrame(stackPos(11));

	debugC(0, 0xfff, "Init Anim %s %d %d %d %d %d %d %d %d %d %d %d %d %d\n", GetText(12, state), stackPos(0), stackPos(1), stackPos(2), stackPos(3),
	       stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10), stackPos(11),  stackPos(12));

	int32 dx = stackPos(4);
	int32 dy = stackPos(5);
	int32 x = stackPos(2);
	int32 layerZ = stackPos(3);

	if (dx == -2)
		sceneAnim->_animInstance->moveRelative(TOON_SCREEN_WIDTH, 0, 0);
	else if (dx < 0) {
		dx = sceneAnim->_animation->_x1;
	}
	else if (dx >= 0)
		sceneAnim->_animInstance->setX(dx);

	if (dy >= 0)
		sceneAnim->_animInstance->setY(dy);
	else
		dy = sceneAnim->_animation->_y1;

	if (flags & 0x20) {
		sceneAnim->_animInstance->setZ(_vm->getLayerAtPoint(x, layerZ));
		sceneAnim->_animInstance->setUseMask(true);
	}

	if (layerZ >= 0) {
		sceneAnim->_animInstance->setLayerZ(layerZ);
	} else {
		dy = dy + sceneAnim->_animation->_y2 - sceneAnim->_animation->_y1 - 1;
		sceneAnim->_animInstance->setLayerZ(dy);
	}

	sceneAnim->_animInstance->setId(stackPos(0));
	sceneAnim->_active = true;

	// add the animation to the list only if it's already visible
	if (flags & 1) {
		_vm->getAnimationManager()->addInstance(sceneAnim->_animInstance);
	}
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Scene_Animation_Active_Flag(EMCState *state) {
	int32 animId = stackPos(0);
	int32 activeFlag = stackPos(1);

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animId);

	if (sceneAnim->_active) {
		sceneAnim->_animInstance->setVisible(activeFlag > 0);

		if (activeFlag) {
			_vm->getAnimationManager()->addInstance(sceneAnim->_animInstance);
		}
	}

	return 0;
}

int32 ScriptFunc::sys_Cmd_Draw_Scene_Anim_WSA_Frame(EMCState *state) {
	int32 animId = stackPos(0);
	int32 frame = stackPos(1);

	if (frame < 0)
		return 0;

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animId);

	if (sceneAnim->_active) {
		sceneAnim->_animInstance->setAnimation(sceneAnim->_animation);
		sceneAnim->_animInstance->setFrame(frame);
		sceneAnim->_animInstance->setAnimationRange(frame, frame);
		sceneAnim->_animInstance->stopAnimation();
	}
	_vm->setSceneAnimationScriptUpdate(false);

	// WORKAROUND : Too fast animations...
	if (_vm->state()->_currentScene == 26 && animId == 22)
		_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 3);

	if (_vm->state()->_currentScene == 14) {
		if (animId == 3 || animId == 2 || animId == 4)
			_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 2);
		else if (animId == 20 || animId == 15 || animId == 21 || animId == 16 || animId == 17 || animId == 18)
			_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 1);
		else if (animId == 9) {
			_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 3);
		}
	}

	if (_vm->state()->_currentScene == 19 && _vm->getCurrentUpdatingSceneAnimation() == 0 ) {
		_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 6);
	}

	if (_vm->state()->_currentScene == 29) {
		if (animId == 16 || animId == 26 || animId == 36)
			_vm->pauseSceneAnimationScript(_vm->getCurrentUpdatingSceneAnimation(), 2);
	}

	return 0;
}

int32 ScriptFunc::sys_Cmd_Move_Scene_Anim(EMCState *state) {
	int32 animId = stackPos(0);

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(animId);
	sceneAnim->_animInstance->moveRelative(stackPos(1), stackPos(2), 0);
	_vm->setSceneAnimationScriptUpdate(false);
	return 0;
}

int32 ScriptFunc::sys_Cmd_Run_Actor_Default_Script(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Location_Data(EMCState *state) {
	// initial setup of locations
	int32 locationId = stackPos(0);
	debugC(0, 0, "setlocationdata(%d) %s %x %s %s %d %d", locationId, GetText(1, state), stackPos(2), GetText(3, state), GetText(4, state), stackPos(5), stackPos(6));
	strcpy(_vm->state()->_locations[locationId]._name, GetText(1, state));
	strcpy(_vm->state()->_locations[locationId]._music, GetText(3, state));
	strcpy(_vm->state()->_locations[locationId]._cutaway, GetText(4, state));
	_vm->state()->_locations[locationId]._flags = stackPos(2);
	_vm->state()->_locations[locationId]._visited = false;
	_vm->state()->_locations[locationId]._numSceneAnimations = stackPos(5);

	return 0;

}

int32 ScriptFunc::sys_Cmd_Set_CountDown_Timer(EMCState *state) {
	// game timer is in ticks
	_vm->state()->_gameTimer = stackPos(0) * _vm->getTickLength();
	return 0;
}

int32 ScriptFunc::sys_Cmd_Query_CountDown_Timer(EMCState *state) {
	return _vm->state()->_gameTimer;
}

int32 ScriptFunc::sys_Cmd_Proceed_To_Next_Chapter(EMCState *state) {

	_vm->state()->_currentChapter = stackPos(0);
	_vm->exitScene();
	_vm->loadScene(stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Play_Sfx_Plus(EMCState *state) {
	//debugC(0, 0xfff, "playSfx ( %d , %d, %d, %d, %d )", stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_vm->playSFX(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Play_Sfx(EMCState *state) {
	//debugC(0, 0xfff, "playSfx ( %d , %d)", stackPos(0), stackPos(1));
	_vm->playSFX(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Ambient_Sfx(EMCState *state) {
	//debug("Ambient Sfx : %d %d %d %d", stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_vm->getAudioManager()->startAmbientSFX(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Kill_Ambient_Sfx(EMCState *state) {
	//debug("Kill Sfx : %d", stackPos(0));
	_vm->getAudioManager()->killAmbientSFX(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Ambient_Sfx_Plus(EMCState *state) {
	//debug("Ambient Sfx Plus: %d %d %d %d %d %d %d %d", stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	_vm->getAudioManager()->startAmbientSFX(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Ambient_Volume(EMCState *state) {
	//debug("Ambient Volume : %d %d", stackPos(0), stackPos(1));
	_vm->getAudioManager()->setAmbientSFXVolume(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Freeze_Scene_Animation(EMCState *state) {

	_vm->getSceneAnimationScript(stackPos(0))->_frozen = true;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Unfreeze_Scene_Animation(EMCState *state) {
	_vm->getSceneAnimationScript(stackPos(0))->_frozen = false;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Scene_Animation_Frozen(EMCState *state) {
	return _vm->getSceneAnimationScript(stackPos(0))->_frozen ? 1 : 0;
}

int32 ScriptFunc::sys_Cmd_Set_Script_Game_Data_Global(EMCState *state) {
	_vm->state()->_gameGlobalData[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int32 ScriptFunc::sys_Cmd_Get_Script_Game_Data_Global(EMCState *state) {
	return _vm->state()->_gameGlobalData[stackPos(0)];
}

int32 ScriptFunc::sys_Cmd_Say_Line(EMCState *state) {
	_vm->sayLines(1 , stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Knight_Puzzle_Get_Coord(EMCState *state) {
	static const uint16 knightCoords[] = {
		0x327, 0x0fa, 0x354, 0x0f8, 0x383, 0x0f4, 0x3a4, 0x0f0, 0x3d3, 0x0ed,
		0x3fd, 0x0ef, 0x2fe, 0x12d, 0x2fe, 0x12d, 0x310, 0x109, 0x349, 0x103,
		0x378, 0x103, 0x3a4, 0x102, 0x3d5, 0x102, 0x403, 0x103, 0x2fe, 0x12d,
		0x2fe, 0x12d, 0x2fe, 0x12d, 0x2fe, 0x12d, 0x369, 0x123, 0x3a4, 0x123,
		0x3d8, 0x121, 0x410, 0x124, 0x2fe, 0x12d, 0x2fe, 0x12d, 0x2fe, 0x12d,
		0x2fe, 0x12d, 0x35d, 0x14f, 0x3a2, 0x149, 0x3db, 0x149, 0x415, 0x14a,
		0x2fe, 0x12d, 0x2fe, 0x12d
	};

	return knightCoords[stackPos(2) + 2 * (8 * stackPos(1) + stackPos(0))];
}

int32 ScriptFunc::sys_Cmd_Add_Scene_Anim(EMCState *state) {
	return sys_Cmd_Init_Scene_Anim(state);
}

int32 ScriptFunc::sys_Cmd_Remove_Scene_Anim(EMCState *state) {
	int32 sceneId = stackPos(0);

	if (!_vm->getSceneAnimation(sceneId)->_active)
		return 0;

	SceneAnimation *sceneAnim = _vm->getSceneAnimation(sceneId);
	sceneAnim->_active = false;
	_vm->getAnimationManager()->removeInstance(sceneAnim->_animInstance);
	delete sceneAnim->_animation;
	sceneAnim->_animation = NULL;

	// see if one character shares this instance
	for (int32 c = 0; c < 32; c++) {
		if (_vm->getCharacter(c) && _vm->getCharacter(c)->getAnimationInstance() == sceneAnim->_originalAnimInstance) {
			_vm->getCharacter(c)->setAnimationInstance(NULL);
		}
	}
	delete sceneAnim->_originalAnimInstance;
	sceneAnim->_originalAnimInstance = NULL;
	sceneAnim->_animInstance = NULL;
	return 0;
}

int32 ScriptFunc::sys_Cmd_Disable_Timer(EMCState *state) {
	_vm->disableTimer(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Enable_Timer(EMCState *state) {
	_vm->enableTimer(stackPos(0));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Timer(EMCState *state) {
	_vm->setTimer(stackPos(0), stackPos(1));
	return 0;
}

int32 ScriptFunc::sys_Cmd_Set_Palette_Color(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Number_Of_NPCs(EMCState *state) {
	return 0;
}

int32 ScriptFunc::sys_Cmd_Get_Config_Language(EMCState *state) {
	return 0;
}

} // End of namespace Toon
