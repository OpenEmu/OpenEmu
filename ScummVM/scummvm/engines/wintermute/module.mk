MODULE := engines/wintermute
 
MODULE_OBJS := \
	ad/ad_actor.o \
	ad/ad_entity.o \
	ad/ad_game.o \
	ad/ad_inventory.o \
	ad/ad_inventory_box.o \
	ad/ad_item.o \
	ad/ad_layer.o \
	ad/ad_node_state.o \
	ad/ad_object.o \
	ad/ad_path.o \
	ad/ad_path_point.o \
	ad/ad_region.o \
	ad/ad_response.o \
	ad/ad_response_box.o \
	ad/ad_response_context.o \
	ad/ad_rot_level.o \
	ad/ad_scale_level.o \
	ad/ad_scene.o \
	ad/ad_scene_node.o \
	ad/ad_scene_state.o \
	ad/ad_sentence.o \
	ad/ad_sprite_set.o \
	ad/ad_talk_def.o \
	ad/ad_talk_holder.o \
	ad/ad_talk_node.o \
	ad/ad_waypoint_group.o \
	base/scriptables/script.o \
	base/scriptables/script_engine.o \
	base/scriptables/script_stack.o \
	base/scriptables/script_value.o \
	base/scriptables/script_ext_array.o \
	base/scriptables/script_ext_date.o \
	base/scriptables/script_ext_file.o \
	base/scriptables/script_ext_math.o \
	base/scriptables/script_ext_object.o \
	base/scriptables/script_ext_mem_buffer.o \
	base/scriptables/script_ext_string.o \
	base/file/base_disk_file.o \
	base/file/base_file.o \
	base/file/base_file_entry.o \
	base/file/base_package.o \
	base/file/base_resources.o \
	base/file/base_save_thumb_file.o \
	base/font/base_font_bitmap.o \
	base/font/base_font_truetype.o \
	base/font/base_font.o \
	base/font/base_font_storage.o \
	base/gfx/base_image.o \
	base/gfx/base_renderer.o \
	base/gfx/base_surface.o \
	base/gfx/osystem/base_surface_osystem.o \
	base/gfx/osystem/base_render_osystem.o \
	base/particles/part_particle.o \
	base/particles/part_emitter.o \
	base/particles/part_force.o \
	base/sound/base_sound.o \
	base/sound/base_sound_buffer.o \
	base/sound/base_sound_manager.o \
	base/base_active_rect.o \
	base/base.o \
	base/base_dynamic_buffer.o \
	base/base_engine.o \
	base/base_fader.o \
	base/base_file_manager.o \
	base/base_frame.o \
	base/base_game.o \
	base/base_keyboard_state.o \
	base/base_named_object.o \
	base/base_object.o \
	base/base_parser.o \
	base/base_persistence_manager.o \
	base/base_point.o \
	base/base_quick_msg.o \
	base/base_region.o \
	base/base_save_thumb_helper.o \
	base/base_scriptable.o \
	base/base_script_holder.o \
	base/base_sprite.o \
	base/base_string_table.o \
	base/base_sub_frame.o \
	base/base_surface_storage.o \
	base/base_transition_manager.o \
	base/base_viewport.o \
	base/saveload.o \
	detection.o \
	graphics/transparent_surface.o \
	math/math_util.o \
	math/matrix4.o \
	math/vector2.o \
	platform_osystem.o \
	system/sys_class.o \
	system/sys_class_registry.o \
	system/sys_instance.o \
	ui/ui_button.o \
	ui/ui_edit.o \
	ui/ui_entity.o \
	ui/ui_object.o \
	ui/ui_text.o \
	ui/ui_tiled_image.o \
	ui/ui_window.o \
	utils/convert_utf.o \
	utils/crc.o \
	utils/path_util.o \
	utils/string_util.o \
	utils/utils.o \
	video/video_player.o \
	video/video_theora_player.o \
	wintermute.o \
	persistent.o
 
MODULE_DIRS += \
	engines/wintermute
 
# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
