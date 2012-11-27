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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_actor.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_inventory.h"
#include "engines/wintermute/ad/ad_inventory_box.h"
#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_node_state.h"
#include "engines/wintermute/ad/ad_object.h"
#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/ad/ad_path_point.h"
#include "engines/wintermute/ad/ad_region.h"
#include "engines/wintermute/ad/ad_response.h"
#include "engines/wintermute/ad/ad_response_box.h"
#include "engines/wintermute/ad/ad_response_context.h"
#include "engines/wintermute/ad/ad_rot_level.h"
#include "engines/wintermute/ad/ad_scale_level.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/ad/ad_scene_state.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_sprite_set.h"
#include "engines/wintermute/ad/ad_talk_def.h"
#include "engines/wintermute/ad/ad_talk_holder.h"
#include "engines/wintermute/ad/ad_talk_node.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/base/base_fader.h"
#include "engines/wintermute/base/font/base_font_bitmap.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font_truetype.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"
#include "engines/wintermute/base/scriptables/script_ext_date.h"
#include "engines/wintermute/base/scriptables/script_ext_file.h"
#include "engines/wintermute/base/scriptables/script_ext_math.h"
#include "engines/wintermute/base/scriptables/script_ext_mem_buffer.h"
#include "engines/wintermute/base/scriptables/script_ext_object.h"
#include "engines/wintermute/base/scriptables/script_ext_string.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_edit.h"
#include "engines/wintermute/ui/ui_entity.h"
#include "engines/wintermute/ui/ui_text.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/system/sys_class.h"

// SystemClass adds these objects to the registry, thus they aren't as leaked as they look
#define REGISTER_CLASS(class_name, persistent_class)\
	new Wintermute::SystemClass(class_name::_className, class_name::persistBuild, class_name::persistLoad, persistent_class);

namespace Wintermute {

// This is done in a separate file, to avoid including the kitchensink in SystemClassRegistry.
void SystemClassRegistry::registerClasses() {
	REGISTER_CLASS(AdActor, false)
	REGISTER_CLASS(AdEntity, false)
	REGISTER_CLASS(AdGame, true)
	REGISTER_CLASS(AdInventory, false)
	REGISTER_CLASS(AdInventoryBox, false)
	REGISTER_CLASS(AdItem, false)
	REGISTER_CLASS(AdLayer, false)
	REGISTER_CLASS(AdNodeState, false)
	REGISTER_CLASS(AdObject, false)
	REGISTER_CLASS(AdPath, false)
	REGISTER_CLASS(AdPathPoint, false)
	REGISTER_CLASS(AdRegion, false)
	REGISTER_CLASS(AdResponse, false)
	REGISTER_CLASS(AdResponseBox, false)
	REGISTER_CLASS(AdResponseContext, false)
	REGISTER_CLASS(AdRotLevel, false)
	REGISTER_CLASS(AdScaleLevel, false)
	REGISTER_CLASS(AdScene, false)
	REGISTER_CLASS(AdSceneNode, false)
	REGISTER_CLASS(AdSceneState, false)
	REGISTER_CLASS(AdSentence, false)
	REGISTER_CLASS(AdSpriteSet, false)
	REGISTER_CLASS(AdTalkDef, false)
	REGISTER_CLASS(AdTalkHolder, false)
	REGISTER_CLASS(AdTalkNode, false)
	REGISTER_CLASS(AdWaypointGroup, false)

	REGISTER_CLASS(BaseFader, false)
	REGISTER_CLASS(BaseFont, false)
	REGISTER_CLASS(BaseFontBitmap, false)
	REGISTER_CLASS(BaseFontStorage, true)
	REGISTER_CLASS(BaseFontTT, false)
	REGISTER_CLASS(BaseFrame, false)
	REGISTER_CLASS(BaseGame, true)
	REGISTER_CLASS(BaseKeyboardState, false)
	REGISTER_CLASS(BaseObject, false)
	REGISTER_CLASS(BasePoint, false)
	REGISTER_CLASS(BaseRegion, false)
	REGISTER_CLASS(BaseScriptable, false)
	REGISTER_CLASS(BaseScriptHolder, false)
	REGISTER_CLASS(BaseSound, false)
	REGISTER_CLASS(BaseSprite, false)
	REGISTER_CLASS(BaseSubFrame, false)

	REGISTER_CLASS(BaseViewport, false)
	REGISTER_CLASS(PartEmitter, false)
	REGISTER_CLASS(ScEngine, true)
	REGISTER_CLASS(ScScript, false)
	REGISTER_CLASS(ScStack, false)
	REGISTER_CLASS(ScValue, false)
	REGISTER_CLASS(SXArray, false)
	REGISTER_CLASS(SXDate, false)
	REGISTER_CLASS(SXFile, false)
	REGISTER_CLASS(SXMath, true)
	REGISTER_CLASS(SXMemBuffer, false)
	REGISTER_CLASS(SXObject, false)
	REGISTER_CLASS(SXString, false)

	REGISTER_CLASS(UIButton, false)
	REGISTER_CLASS(UIEdit, false)
	REGISTER_CLASS(UIEntity, false)
	REGISTER_CLASS(UIObject, false)
	REGISTER_CLASS(UIText, false)
	REGISTER_CLASS(UITiledImage, false)
	REGISTER_CLASS(UIWindow, false)
	REGISTER_CLASS(VideoTheoraPlayer, false)
}

}
