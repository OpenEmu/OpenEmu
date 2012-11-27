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

#ifndef SCRIPT_FUNC_H
#define SCRIPT_FUNC_H

#include "common/array.h"
#include "toon/script.h"

namespace Toon {

class ScriptFunc;

typedef Common::Functor1Mem<EMCState *, int32, ScriptFunc> OpcodeV2;

class ScriptFunc {
public:
	ScriptFunc(ToonEngine *vm);
	~ScriptFunc(void);
	Common::Array<const OpcodeV2 *> _opcodes;
	ToonEngine *_vm;

#define SYSFUNC(x)  int32 x(EMCState *)
	SYSFUNC(sys_Cmd_Dummy);
	SYSFUNC(sys_Cmd_Change_Actor_X_And_Y);
	SYSFUNC(sys_Cmd_Init_Talking_Character);
	SYSFUNC(sys_Cmd_Draw_Actor_Standing);
	SYSFUNC(sys_Cmd_Get_Actor_X);
	SYSFUNC(sys_Cmd_Get_Actor_Y);
	SYSFUNC(sys_Cmd_Get_Actor_Facing);
	SYSFUNC(sys_Cmd_Get_Last_Scene);
	SYSFUNC(sys_Cmd_Debug_Print);
	SYSFUNC(sys_Cmd_Flip_Screens);
	SYSFUNC(sys_Cmd_Play_Flic);
	SYSFUNC(sys_Cmd_Force_Facing);
	SYSFUNC(sys_Cmd_Restart_Thread);
	SYSFUNC(sys_Cmd_Walk_Actor_To_Point);
	SYSFUNC(sys_Cmd_Set_Sack_Visible);
	SYSFUNC(sys_Cmd_Set_Actor_Facing);
	SYSFUNC(sys_Cmd_Confiscate_Inventory);
	SYSFUNC(sys_Cmd_Character_Talks);
	SYSFUNC(sys_Cmd_Visited_Scene);
	SYSFUNC(sys_Cmd_Query_Rif_Flag);
	SYSFUNC(sys_Cmd_Query_Scroll);
	SYSFUNC(sys_Cmd_Set_Initial_Location);
	SYSFUNC(sys_Cmd_Make_Line_Non_Walkable);
	SYSFUNC(sys_Cmd_Make_Line_Walkable);
	SYSFUNC(sys_Cmd_Walk_Actor_On_Condition);
	SYSFUNC(sys_Cmd_Set_Actor_Facing_Point);
	SYSFUNC(sys_Cmd_Set_Inventory_Slot);
	SYSFUNC(sys_Cmd_Get_Inventory_Slot);
	SYSFUNC(sys_Cmd_Add_Item_To_Inventory);
	SYSFUNC(sys_Cmd_Set_Actor_RGB_Modifiers);
	SYSFUNC(sys_Cmd_Init_Conversation_AP);
	SYSFUNC(sys_Cmd_Actor_Talks);
	SYSFUNC(sys_Cmd_Say_Lines);
	SYSFUNC(sys_Cmd_Set_Rif_Flag);
	SYSFUNC(sys_Cmd_Empty_Inventory);
	SYSFUNC(sys_Cmd_Set_Anim_Scale_Size);
	SYSFUNC(sys_Cmd_Delete_Item_From_Inventory);
	SYSFUNC(sys_Cmd_Specific_Item_In_Inventory);
	SYSFUNC(sys_Cmd_Run_Script);
	SYSFUNC(sys_Cmd_Query_Game_Flag);
	SYSFUNC(sys_Cmd_Reset_Game_Flag);
	SYSFUNC(sys_Cmd_Set_Game_Flag);
	SYSFUNC(sys_Cmd_Create_Mouse_Item);
	SYSFUNC(sys_Cmd_Destroy_Mouse_Item);
	SYSFUNC(sys_Cmd_Get_Mouse_State);
	SYSFUNC(sys_Cmd_Hide_Mouse);
	SYSFUNC(sys_Cmd_Exit_Conversation);
	SYSFUNC(sys_Cmd_Set_Mouse_Pos);
	SYSFUNC(sys_Cmd_Show_Mouse);
	SYSFUNC(sys_Cmd_In_Close_Up);
	SYSFUNC(sys_Cmd_Set_Scroll_Lock);
	SYSFUNC(sys_Cmd_Fill_Area_Non_Walkable);
	SYSFUNC(sys_Cmd_Set_Scroll_Coords);
	SYSFUNC(sys_Cmd_Hide_Cutaway);
	SYSFUNC(sys_Cmd_Show_Cutaway);
	SYSFUNC(sys_Cmd_Pause_Ticks);
	SYSFUNC(sys_Cmd_In_Conversation);
	SYSFUNC(sys_Cmd_Character_Talking);
	SYSFUNC(sys_Cmd_Set_Flux_Facing_Point);
	SYSFUNC(sys_Cmd_Set_Flux_Facing);
	SYSFUNC(sys_Cmd_Set_Flux_Coords);
	SYSFUNC(sys_Cmd_Set_Flux_Visible);
	SYSFUNC(sys_Cmd_Get_Flux_X);
	SYSFUNC(sys_Cmd_Get_Flux_Y);
	SYSFUNC(sys_Cmd_Get_Flux_Facing);
	SYSFUNC(sys_Cmd_Get_Flux_Flags);
	SYSFUNC(sys_Cmd_Query_Flux_Coords);
	SYSFUNC(sys_Cmd_Have_A_Conversation);
	SYSFUNC(sys_Cmd_Walk_Flux_To_Point);
	SYSFUNC(sys_Cmd_Query_Scene_Anim_Loaded);
	SYSFUNC(sys_Cmd_Play_Flux_Anim);
	SYSFUNC(sys_Cmd_Set_Anim_Priority);
	SYSFUNC(sys_Cmd_Place_Scene_Anim);
	SYSFUNC(sys_Cmd_Update_Scene_Animations);
	SYSFUNC(sys_Cmd_Get_Drew_Scale);
	SYSFUNC(sys_Cmd_Query_Drew_Flags);
	SYSFUNC(sys_Cmd_Set_Music);
	SYSFUNC(sys_Cmd_Query_Speech);
	SYSFUNC(sys_Cmd_Enter_New_Scene);
	SYSFUNC(sys_Cmd_Enter_Same_Scene);
	SYSFUNC(sys_Cmd_Is_Pixel_Walkable);
	SYSFUNC(sys_Cmd_Show_Screen);
	SYSFUNC(sys_Cmd_Hide_Screen);
	SYSFUNC(sys_Cmd_Set_Special_Enter_X_And_Y);
	SYSFUNC(sys_Cmd_Get_Mouse_X);
	SYSFUNC(sys_Cmd_Get_Mouse_Y);
	SYSFUNC(sys_Cmd_Fade_Palette);
	SYSFUNC(sys_Cmd_Music_Enabled);
	SYSFUNC(sys_Cmd_Random);
	SYSFUNC(sys_Cmd_Wait_Key);
	SYSFUNC(sys_Cmd_Draw_Scene_Anim_WSA_Frame_To_Back);
	SYSFUNC(sys_Cmd_Set_Scene_Anim_Wait);
	SYSFUNC(sys_Cmd_Init_Scene_Anim);
	SYSFUNC(sys_Cmd_Set_Scene_Animation_Active_Flag);
	SYSFUNC(sys_Cmd_Draw_Scene_Anim_WSA_Frame);
	SYSFUNC(sys_Cmd_Move_Scene_Anim);
	SYSFUNC(sys_Cmd_Run_Actor_Default_Script);
	SYSFUNC(sys_Cmd_Set_Location_Data);
	SYSFUNC(sys_Cmd_Set_CountDown_Timer);
	SYSFUNC(sys_Cmd_Query_CountDown_Timer);
	SYSFUNC(sys_Cmd_Proceed_To_Next_Chapter);
	SYSFUNC(sys_Cmd_Play_Sfx_Plus);
	SYSFUNC(sys_Cmd_Play_Sfx);
	SYSFUNC(sys_Cmd_Set_Ambient_Sfx);
	SYSFUNC(sys_Cmd_Kill_Ambient_Sfx);
	SYSFUNC(sys_Cmd_Set_Ambient_Sfx_Plus);
	SYSFUNC(sys_Cmd_Set_Ambient_Volume);
	SYSFUNC(sys_Cmd_Freeze_Scene_Animation);
	SYSFUNC(sys_Cmd_Unfreeze_Scene_Animation);
	SYSFUNC(sys_Cmd_Scene_Animation_Frozen);
	SYSFUNC(sys_Cmd_Set_Script_Game_Data_Global);
	SYSFUNC(sys_Cmd_Get_Script_Game_Data_Global);
	SYSFUNC(sys_Cmd_Say_Line);
	SYSFUNC(sys_Cmd_Knight_Puzzle_Get_Coord);
	SYSFUNC(sys_Cmd_Add_Scene_Anim);
	SYSFUNC(sys_Cmd_Remove_Scene_Anim);
	SYSFUNC(sys_Cmd_Disable_Timer);
	SYSFUNC(sys_Cmd_Enable_Timer);
	SYSFUNC(sys_Cmd_Set_Timer);
	SYSFUNC(sys_Cmd_Set_Palette_Color);
	SYSFUNC(sys_Cmd_Number_Of_NPCs);
	SYSFUNC(sys_Cmd_Get_Config_Language);
	SYSFUNC(sys_Cmd_Get_Actor_Final_X);
	SYSFUNC(sys_Cmd_Get_Actor_Final_Y);
};

} // End of namespace Toon

#endif
