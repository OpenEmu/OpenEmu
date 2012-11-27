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


#include "common/debug.h"
#include "common/util.h"

#include "sky/debug.h"
#include "sky/grid.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/struc.h"
#include "sky/compact.h"

namespace Sky {

static const char *const logic_table_names[] = {
	"return",
	"Logic::script",
	"Logic::auto_route",
	"Logic::ar_anim",
	"Logic::ar_turn",
	"Logic::alt",
	"Logic::anim",
	"Logic::turn",
	"Logic::cursor",
	"Logic::talk",
	"Logic::listen",
	"Logic::stopped",
	"Logic::choose",
	"Logic::frames",
	"Logic::pause",
	"Logic::wait_sync",
	"Logic::simple_anim"
};

static const char opcode_par[] = {
	1,
	0,
	1,
	0,
	0,
	1,
	1,
	0,
	0,
	1,
	0,
	2,
	0,
	0,
	1,
	1,
	1,
	0,
	1,
	0,
	0
};

static const char *const opcodes[] = {
	"push_variable",
	"less_than",
	"push_number",
	"not_equal",
	"if_and",
	"skip_zero",
	"pop_var",
	"minus",
	"plus",
	"skip_always",
	"if_or",
	"call_mcode",
	"more_than",
	"script_exit",
	"switch",
	"push_offset",
	"pop_offset",
	"is_equal",
	"skip_nz",
	"script_exit",
	"restart_script"
};

static const char *const mcodes[] = {
	"fn_cache_chip",
	"fn_cache_fast",
	"fn_draw_screen",
	"fn_ar",
	"fn_ar_animate",
	"fn_idle",
	"fn_interact",
	"fn_start_sub",
	"fn_they_start_sub",
	"fn_assign_base",
	"fn_disk_mouse",
	"fn_normal_mouse",
	"fn_blank_mouse",
	"fn_cross_mouse",
	"fn_cursor_right",
	"fn_cursor_left",
	"fn_cursor_down",
	"fn_open_hand",
	"fn_close_hand",
	"fn_get_to",
	"fn_set_to_stand",
	"fn_turn_to",
	"fn_arrived",
	"fn_leaving",
	"fn_set_alternate",
	"fn_alt_set_alternate",
	"fn_kill_id",
	"fn_no_human",
	"fn_add_human",
	"fn_add_buttons",
	"fn_no_buttons",
	"fn_set_stop",
	"fn_clear_stop",
	"fn_pointer_text",
	"fn_quit",
	"fn_speak_me",
	"fn_speak_me_dir",
	"fn_speak_wait",
	"fn_speak_wait_dir",
	"fn_chooser",
	"fn_highlight",
	"fn_text_kill",
	"fn_stop_mode",
	"fn_we_wait",
	"fn_send_sync",
	"fn_send_fast_sync",
	"fn_send_request",
	"fn_clear_request",
	"fn_check_request",
	"fn_start_menu",
	"fn_unhighlight",
	"fn_face_id",
	"fn_foreground",
	"fn_background",
	"fn_new_background",
	"fn_sort",
	"fn_no_sprite_engine",
	"fn_no_sprites_a6",
	"fn_reset_id",
	"fn_toggle_grid",
	"fn_pause",
	"fn_run_anim_mod",
	"fn_simple_mod",
	"fn_run_frames",
	"fn_await_sync",
	"fn_inc_mega_set",
	"fn_dec_mega_set",
	"fn_set_mega_set",
	"fn_move_items",
	"fn_new_list",
	"fn_ask_this",
	"fn_random",
	"fn_person_here",
	"fn_toggle_mouse",
	"fn_mouse_on",
	"fn_mouse_off",
	"fn_fetch_x",
	"fn_fetch_y",
	"fn_test_list",
	"fn_fetch_place",
	"fn_custom_joey",
	"fn_set_palette",
	"fn_text_module",
	"fn_change_name",
	"fn_mini_load",
	"fn_flush_buffers",
	"fn_flush_chip",
	"fn_save_coods",
	"fn_plot_grid",
	"fn_remove_grid",
	"fn_eyeball",
	"fn_cursor_up",
	"fn_leave_section",
	"fn_enter_section",
	"fn_restore_game",
	"fn_restart_game",
	"fn_new_swing_seq",
	"fn_wait_swing_end",
	"fn_skip_intro_code",
	"fn_blank_screen",
	"fn_print_credit",
	"fn_look_at",
	"fn_linc_text_module",
	"fn_text_kill2",
	"fn_set_font",
	"fn_start_fx",
	"fn_stop_fx",
	"fn_start_music",
	"fn_stop_music",
	"fn_fade_down",
	"fn_fade_up",
	"fn_quit_to_dos",
	"fn_pause_fx",
	"fn_un_pause_fx",
	"fn_printf"
};

static const char *const scriptVars[] = {
	"result",
	"screen",
	"logic_list_no",
	"safe_logic_list",
	"low_list_no",
	"high_list_no",
	"mouse_list_no",
	"safe_mouse_list",
	"draw_list_no",
	"second_draw_list",
	"do_not_use",
	"music_module",
	"cur_id",
	"mouse_status",
	"mouse_stop",
	"button",
	"but_repeat",
	"special_item",
	"get_off",
	"safe_click",
	"click_id",
	"player_id",
	"cursor_id",
	"pointer_pen",
	"last_pal",
	"safex",
	"safey",
	"player_x",
	"player_y",
	"player_mood",
	"player_screen",
	"old_x",
	"old_y",
	"joey_x",
	"joey_y",
	"joey_list",
	"flag",
	"hit_id",
	"player_target",
	"joey_target",
	"mega_target",
	"layer_0_id",
	"layer_1_id",
	"layer_2_id",
	"layer_3_id",
	"grid_1_id",
	"grid_2_id",
	"grid_3_id",
	"stop_grid",
	"text_rate",
	"text_speed",
	"the_chosen_one",
	"chosen_anim",
	"text1",
	"anim1",
	"text2",
	"anim2",
	"text3",
	"anim3",
	"text4",
	"anim4",
	"text5",
	"anim5",
	"text6",
	"anim6",
	"text7",
	"anim7",
	"text8",
	"anim8",
	"o0",
	"o1",
	"o2",
	"o3",
	"o4",
	"o5",
	"o6",
	"o7",
	"o8",
	"o9",
	"o10",
	"o11",
	"o12",
	"o13",
	"o14",
	"o15",
	"o16",
	"o17",
	"o18",
	"o19",
	"o20",
	"o21",
	"o22",
	"o23",
	"o24",
	"o25",
	"o26",
	"o27",
	"o28",
	"o29",
	"first_icon",
	"menu_length",
	"scroll_offset",
	"menu",
	"object_held",
	"icon_lit",
	"at_sign",
	"fire_exit_flag",
	"small_door_flag",
	"jobs_greet",
	"lamb_greet",
	"knob_flag",
	"lazer_flag",
	"cupb_flag",
	"jobs_loop",
	"done_something",
	"rnd",
	"jobs_text",
	"jobs_loc1",
	"jobs_loc2",
	"jobs_loc3",
	"id_talking",
	"alarm",
	"alarm_count",
	"clearing_alarm",
	"jobs_friend",
	"joey_born",
	"joey_text",
	"joey_peeved",
	"knows_linc",
	"linc_overmann",
	"reich_entry",
	"seen_lock",
	"wshop_text",
	"knows_firedoor",
	"knows_furnace",
	"jobs_got_spanner",
	"jobs_got_sandwich",
	"jobs_firedoor",
	"knows_transporter",
	"joey_loc1",
	"joey_loc2",
	"joey_loc3",
	"joey_screen",
	"cur_section",
	"old_section",
	"joey_section",
	"lamb_section",
	"knows_overmann",
	"jobs_overmann",
	"jobs_seen_joey",
	"anita_text",
	"anit_loc1",
	"anit_loc2",
	"anit_loc3",
	"lamb_friend",
	"lamb_sick",
	"lamb_crawly",
	"lamb_loc1",
	"lamb_loc2",
	"lamb_loc3",
	"lamb_got_spanner",
	"lamb_text",
	"knows_auditor",
	"lamb_security",
	"lamb_auditor",
	"fore_text",
	"transporter_alive",
	"anita_friend",
	"anita_stop",
	"anita_count",
	"knows_security",
	"fore_loc1",
	"fore_loc2",
	"fore_loc3",
	"fore_friend",
	"knows_dlinc",
	"seen_lift",
	"player_sound",
	"guard_linc",
	"guard_text",
	"guar_loc1",
	"guar_loc2",
	"guar_loc3",
	"guard_talk",
	"lamb_out",
	"guard_warning",
	"wshp_loc1",
	"wshp_loc2",
	"wshp_loc3",
	"jobs_linc",
	"knows_port",
	"jobs_port",
	"joey_overmann",
	"joey_count",
	"knows_pipes",
	"knows_hobart",
	"fore_hobart",
	"fore_overmann",
	"anit_text",
	"seen_eye",
	"anita_dlinc",
	"seen_dis_lift",
	"lamb_move_anita",
	"lamb_stat",
	"machine_stops",
	"guard_stat",
	"guard_hobart",
	"gordon_text",
	"gord_loc1",
	"gord_loc2",
	"gord_loc3",
	"lamb_hobart",
	"anita_loc1",
	"anita_loc2",
	"anita_loc3",
	"knows_elders",
	"anita_elders",
	"anita_overmann",
	"stay_here",
	"joey_pause",
	"knows_break_in",
	"joey_break_in",
	"joey_lift",
	"stair_talk",
	"blown_top",
	"tamper_flag",
	"knows_reich",
	"gordon_reich",
	"open_panel",
	"panel_count",
	"wreck_text",
	"press_button",
	"touch_count",
	"gordon_overmann",
	"lamb_reich",
	"exit_stores",
	"henri_text",
	"henr_loc1",
	"henr_loc2",
	"henr_loc3",
	"got_sponsor",
	"used_deodorant",
	"lob_dad_text",
	"lob_son_text",
	"scan_talk",
	"dady_loc1",
	"dady_loc2",
	"dady_loc3",
	"samm_loc1",
	"samm_loc2",
	"samm_loc3",
	"dirty_card",
	"wrek_loc1",
	"wrek_loc2",
	"wrek_loc3",
	"crushed_nuts",
	"got_port",
	"anita_port",
	"got_jammer",
	"knows_anita",
	"anita_hobart",
	"local_count",
	"lamb_joey",
	"stop_store",
	"knows_suit",
	"joey_box",
	"asked_box",
	"shell_count",
	"got_cable",
	"local_flag",
	"search_flag",
	"rad_count",
	"rad_text",
	"radm_loc1",
	"radm_loc2",
	"radm_loc3",
	"gordon_off",
	"knows_jobsworth",
	"rad_back_flag",
	"lamb_lift",
	"knows_cat",
	"lamb_screwed",
	"tour_flag",
	"foreman_reactor",
	"foreman_anita",
	"burke_text",
	"burk_loc1",
	"burk_loc2",
	"burk_loc3",
	"burke_anchor",
	"jason_text",
	"jaso_loc1",
	"jaso_loc2",
	"helg_loc2",
	"say_to_helga",
	"interest_count",
	"anchor_text",
	"anchor_overmann",
	"anch_loc1",
	"anch_loc2",
	"anch_loc3",
	"anchor_count",
	"lamb_anchor",
	"anchor_port",
	"knows_stephen",
	"knows_ghoul",
	"anchor_talk",
	"joey_hook",
	"joey_done_dir",
	"bios_loc1",
	"bios_loc2",
	"bios_loc3",
	"got_hook",
	"anchor_anita",
	"trev_loc1",
	"trev_loc2",
	"trev_loc3",
	"trevor_text",
	"trev_text",
	"trev_overmann",
	"lamb_smell",
	"art_flag",
	"trev_computer",
	"helga_text",
	"helg_loc1",
	"helg_loc3",
	"bios_loc4",
	"gallagher_text",
	"gall_loc1",
	"gall_loc2",
	"gall_loc3",
	"warn_lamb",
	"open_apts",
	"store_count",
	"foreman_auditor",
	"frozen_assets",
	"read_report",
	"seen_holo",
	"knows_subway",
	"exit_flag",
	"father_text",
	"lamb_fix",
	"read_briefing",
	"seen_shaft",
	"knows_mother",
	"console_type",
	"hatch_selected",
	"seen_walters",
	"joey_fallen",
	"jbel_loc1",
	"lbel_loc1",
	"lbel_loc2",
	"jobsworth_speech",
	"jobs_alert",
	"jobs_alarmed_ref",
	"safe_joey_recycle",
	"safe_joey_sss",
	"safe_joey_mission",
	"safe_trans_mission",
	"safe_slot_mission",
	"safe_corner_mission",
	"safe_joey_logic",
	"safe_gordon_speech",
	"safe_button_mission",
	"safe_dad_speech",
	"safe_son_speech",
	"safe_skorl_speech",
	"safe_uchar_speech",
	"safe_wreck_speech",
	"safe_anita_speech",
	"safe_lamb_speech",
	"safe_foreman_speech",
	"joey_42_mission",
	"joey_junction_mission",
	"safe_welder_mission",
	"safe_joey_weld",
	"safe_radman_speech",
	"safe_link_7_29",
	"safe_link_29_7",
	"safe_lamb_to_3",
	"safe_lamb_to_2",
	"safe_burke_speech",
	"safe_burke_1",
	"safe_burke_2",
	"safe_dr_1",
	"safe_body_speech",
	"joey_bell",
	"safe_anchor_speech",
	"safe_anchor",
	"safe_pc_mission",
	"safe_hook_mission",
	"safe_trevor_speech",
	"joey_fact",
	"safe_helga_speech",
	"helga_mission",
	"gal_bel_speech",
	"safe_glass_mission",
	"safe_lamb_fact_return",
	"lamb_part_2",
	"safe_lamb_bell_return",
	"safe_lamb_bell",
	"safe_cable_mission",
	"safe_foster_tour",
	"safe_lamb_tour",
	"safe_foreman_logic",
	"safe_lamb_leave",
	"safe_lamb_3",
	"safe_lamb_2",
	"into_linc",
	"out_10",
	"out_74",
	"safe_link_28_31",
	"safe_link_31_28",
	"safe_exit_linc",
	"safe_end_game",
	"which_linc",
	"lift_moving",
	"lift_on_screen",
	"barrel_on_screen",
	"convey_on_screen",
	"shades_searched",
	"joey_wiz",
	"slot_slotted",
	"motor_flag",
	"panel_flag",
	"switch_flag",
	"steam_flag",
	"steam_fx_no",
	"factory_flag",
	"power_door_open",
	"left_skull_flag",
	"right_skull_flag",
	"monitor_watching",
	"left_lever_flag",
	"right_lever_flag",
	"lobby_door_flag",
	"weld_stop",
	"cog_flag",
	"sensor_flag",
	"look_through",
	"welder_nut_flag",
	"s7_lift_flag",
	"s29_lift_flag",
	"whos_at_lift_7",
	"whos_at_lift_29",
	"lift_power",
	"whats_joey",
	"seen_box",
	"seen_welder",
	"flap_flag",
	"s15_floor",
	"foreman_friend",
	"locker1_flag",
	"locker2_flag",
	"locker3_flag",
	"whats_in_locker",
	"knows_radsuit",
	"radman_anita",
	"at_anita",
	"coat_flag",
	"dressed_as",
	"s14_take",
	"reactor_door_flag",
	"joey_in_lift",
	"chair_27_flag",
	"at_body_flag",
	"at_gas_flag",
	"anchor_seated",
	"door_23_jam",
	"door_20_jam",
	"reich_door_flag",
	"reich_door_jam",
	"lamb_door_flag",
	"lamb_door_jam",
	"pillow_flag",
	"cat_food_flag",
	"helga_up",
	"got_magazine",
	"trevs_doing",
	"card_status",
	"card_fix",
	"lamb_gallager",
	"locker_11_flag",
	"ever_opened",
	"linc_10_flag",
	"chair_10_flag",
	"skorl_flag",
	"lift_pause",
	"lift_in_use",
	"gordon_back",
	"furnace_door_flag",
	"whos_with_gall",
	"read_news",
	"whos_at_lift_28",
	"s28_lift_flag",
	"mission_state",
	"anita_flag",
	"card_used",
	"gordon_catch",
	"car_flag",
	"first_jobs",
	"jobs_removed",
	"menu_id",
	"tonys_tour_flag",
	"joey_foster_phase",
	"start_info_window",
	"ref_slab_on",
	"ref_up_mouse",
	"ref_down_mouse",
	"ref_left_mouse",
	"ref_right_mouse",
	"ref_disconnect_foster",
	"k0",
	"k1",
	"k2",
	"k3",
	"k4",
	"k5",
	"k6",
	"k7",
	"k8",
	"k9",
	"k10",
	"k11",
	"k12",
	"k13",
	"k14",
	"k15",
	"k16",
	"k17",
	"k18",
	"k19",
	"k20",
	"k21",
	"k22",
	"k23",
	"k24",
	"k25",
	"k26",
	"k27",
	"k28",
	"k29",
	"a0",
	"a1",
	"a2",
	"a3",
	"a4",
	"a5",
	"a6",
	"a7",
	"a8",
	"a9",
	"a10",
	"a11",
	"a12",
	"a13",
	"a14",
	"a15",
	"a16",
	"a17",
	"a18",
	"a19",
	"a20",
	"a21",
	"a22",
	"a23",
	"a24",
	"a25",
	"a26",
	"a27",
	"a28",
	"a29",
	"g0",
	"g1",
	"g2",
	"g3",
	"g4",
	"g5",
	"g6",
	"g7",
	"g8",
	"g9",
	"g10",
	"g11",
	"g12",
	"g13",
	"g14",
	"g15",
	"g16",
	"g17",
	"g18",
	"g19",
	"g20",
	"g21",
	"g22",
	"g23",
	"g24",
	"g25",
	"g26",
	"g27",
	"g28",
	"g29",
	"window_subject",
	"file_text",
	"size_text",
	"auth_text",
	"note_text",
	"id_head_compact",
	"id_file_compact",
	"id_size_compact",
	"id_auth_compact",
	"id_note_compact",
	"pal_no",
	"strikes",
	"char_set_number",
	"eye90_blinded",
	"zap90",
	"eye90_frame",
	"eye91_blinded",
	"zap91",
	"eye91_frame",
	"bag_open",
	"bridge_a_on",
	"bridge_b_on",
	"bridge_c_on",
	"bridge_d_on",
	"bridge_e_on",
	"bridge_f_on",
	"bridge_g_on",
	"bridge_h_on",
	"green_slab",
	"red_slab",
	"foster_slab",
	"circle_slab",
	"slab1_mouse",
	"slab2_mouse",
	"slab3_mouse",
	"slab4_mouse",
	"slab5_mouse",
	"at_guardian",
	"guardian_there",
	"crystal_shattered",
	"virus_taken",
	"fs_command",
	"enter_digits",
	"next_page",
	"linc_digit_0",
	"linc_digit_1",
	"linc_digit_2",
	"linc_digit_3",
	"linc_digit_4",
	"linc_digit_5",
	"linc_digit_6",
	"linc_digit_7",
	"linc_digit_8",
	"linc_digit_9",
	"ref_std_on",
	"ref_std_exit_left_on",
	"ref_std_exit_right_on",
	"ref_advisor_188",
	"ref_shout_action",
	"ref_mega_click",
	"ref_mega_action",
	"ref_walter_speech",
	"ref_joey_medic",
	"ref_joey_med_logic",
	"ref_joey_med_mission72",
	"ref_ken_logic",
	"ref_ken_speech",
	"ref_ken_mission_hand",
	"ref_sc70_iris_opened",
	"ref_sc70_iris_closed",
	"ref_foster_enter_boardroom",
	"ref_father_speech",
	"ref_foster_enter_new_boardroom",
	"ref_hobbins_speech",
	"ref_sc82_jobs_sss",
	"brickwork",
	"door_67_68_flag",
	"crowbar_in_clot",
	"clot_ruptured",
	"clot_repaired",
	"walt_text",
	"walt_loc1",
	"walt_loc2",
	"walt_loc3",
	"walt_count",
	"medic_text",
	"seen_room_72",
	"seen_tap",
	"joey_med_seen72",
	"seen_secure_door",
	"ask_secure_door",
	"sc70_iris_flag",
	"sc70_iris_frame",
	"foster_on_sc70_iris",
	"sc70_grill_flag",
	"sc71_charging_flag",
	"sc72_slime_flag",
	"sc72_witness_sees_foster",
	"sc72_witness_killed",
	"sc73_gallagher_killed",
	"sc73_removed_board",
	"sc73_searched_corpse",
	"door_73_75_flag",
	"sc74_sitting_flag",
	"sc75_crashed_flag",
	"sc75_tissue_infected",
	"sc75_tongs_flag",
	"sc76_cabinet1_flag",
	"sc76_cabinet2_flag",
	"sc76_cabinet3_flag",
	"sc76_board_flag",
	"sc76_ken_prog_flag",
	"sc76_and2_up_flag",
	"ken_text",
	"ken_door_flag",
	"sc77_foster_hand_flag",
	"sc77_ken_hand_flag",
	"door_77_78_flag",
	"sc80_exit_flag",
	"ref_danielle_speech",
	"ref_danielle_go_home",
	"ref_spunky_go_home",
	"ref_henri_speech",
	"ref_buzzer_speech",
	"ref_foster_visit_dani",
	"ref_danielle_logic",
	"ref_jukebox_speech",
	"ref_vincent_speech",
	"ref_eddie_speech",
	"ref_blunt_speech",
	"ref_dani_answer_phone",
	"ref_spunky_see_video",
	"ref_spunky_bark_at_foster",
	"ref_spunky_smells_food",
	"ref_barry_speech",
	"ref_colston_speech",
	"ref_gallagher_speech",
	"ref_babs_speech",
	"ref_chutney_speech",
	"ref_foster_enter_court",
	"dani_text",
	"dani_loc1",
	"dani_loc2",
	"dani_loc3",
	"dani_buff",
	"dani_huff",
	"mother_hobart",
	"foster_id_flag",
	"knows_spunky",
	"dog_fleas",
	"op_flag",
	"chat_up",
	"buzz_loc1",
	"buzz_loc2",
	"blunt_text",
	"blun_loc1",
	"blun_loc2",
	"blun_loc3",
	"blunt_dan_info",
	"vincent_text",
	"vinc_loc1",
	"vinc_loc2",
	"vinc_loc3",
	"eddie_text",
	"eddi_loc1",
	"eddi_loc2",
	"eddi_loc3",
	"knows_dandelions",
	"barry_text",
	"bazz_loc1",
	"bazz_loc2",
	"bazz_loc3",
	"seen_cellar_door",
	"babs_text",
	"babs_loc1",
	"babs_loc2",
	"babs_loc3",
	"colston_text",
	"cols_loc1",
	"cols_loc2",
	"cols_loc3",
	"jukebox",
	"knows_soaking",
	"knows_complaint",
	"dog_bite",
	"new_prints",
	"knows_virus",
	"been_to_court",
	"danielle_target",
	"spunky_target",
	"henri_forward",
	"sc31_lift_flag",
	"sc31_food_on_plank",
	"sc31_spunky_at_plank",
	"dog_in_lake",
	"sc32_lift_flag",
	"sc33_shed_door_flag",
	"gardener_up",
	"babs_x",
	"babs_y",
	"foster_caching",
	"colston_caching",
	"band_playing",
	"colston_at_table",
	"sc36_next_dealer",
	"sc36_door_flag",
	"sc37_door_flag",
	"sc37_lid_loosened",
	"sc37_lid_used",
	"sc37_standing_on_box",
	"sc37_box_broken",
	"sc37_grill_state",
	"got_dog_biscuits",
	"sc38_video_playing",
	"dani_on_phone",
	"sc40_locker_1_flag",
	"sc40_locker_2_flag",
	"sc40_locker_3_flag",
	"sc40_locker_4_flag",
	"sc40_locker_5_flag",
	"seen_anita_corpse",
	"spunky_at_lift",
	"court_text",
	"blunt_knew_jobs",
	"credit_1_text",
	"credit_2_text",
	"id_credit_1",
	"id_credit_2",
	"glass_stolen",
	"foster_at_plank",
	"foster_at_guard",
	"man_talk",
	"man_loc1",
	"man_loc2",
	"man_loc3"
};

void Debug::logic(uint32 logic) {
	debug(6, "LOGIC: %s", logic_table_names[logic]);
}

void Debug::script(uint32 command, uint16 *scriptData) {
	debug(6, "SCRIPT: %s", opcodes[command]);
	if (command == 0 || command == 6)
		debug(6, " %s", scriptVars[(*scriptData)/4]);
	else {
		int i;
		for (i = 0; i < opcode_par[command]; i++) {
			debug(6, " %d", *(scriptData + i));
		}
	}
	debug(6, " ");	// Print an empty line as separator
}

void Debug::mcode(uint32 mcode, uint32 a, uint32 b, uint32 c) {
	debug(6, "MCODE: %s(%d, %d, %d)", mcodes[mcode], a, b, c);
}




Debugger::Debugger(Logic *logic, Mouse *mouse, Screen *screen, SkyCompact *skyCompact)
: GUI::Debugger(), _logic(logic), _mouse(mouse), _screen(screen), _skyCompact(skyCompact), _showGrid(false) {
	DCmd_Register("info",       WRAP_METHOD(Debugger, Cmd_Info));
	DCmd_Register("showgrid",   WRAP_METHOD(Debugger, Cmd_ShowGrid));
	DCmd_Register("reloadgrid", WRAP_METHOD(Debugger, Cmd_ReloadGrid));
	DCmd_Register("compact",    WRAP_METHOD(Debugger, Cmd_ShowCompact));
	DCmd_Register("logiccmd",   WRAP_METHOD(Debugger, Cmd_LogicCommand));
	DCmd_Register("scriptvar",  WRAP_METHOD(Debugger, Cmd_ScriptVar));
	DCmd_Register("section",    WRAP_METHOD(Debugger, Cmd_Section));
	DCmd_Register("logiclist",  WRAP_METHOD(Debugger, Cmd_LogicList));
}

Debugger::~Debugger() {} // we need this here for __SYMBIAN32__

void Debugger::preEnter() {
	::GUI::Debugger::preEnter();
}

void Debugger::postEnter() {
	::GUI::Debugger::postEnter();
	_mouse->resetCursor();
}

bool Debugger::Cmd_ShowGrid(int argc, const char **argv) {
	_showGrid = !_showGrid;
	DebugPrintf("Show grid: %s\n", _showGrid ? "On" : "Off");
	if (!_showGrid)	_screen->forceRefresh();
	return true;
}

bool Debugger::Cmd_ReloadGrid(int argc, const char **argv) {
	_logic->_skyGrid->loadGrids();
	DebugPrintf("Grid reloaded\n");
	return true;
}

static const char *const logicTypes[] = {
	"(none)", "SCRIPT", "AUTOROUTE", "AR_ANIM", "AR_TURNING", "ALT", "MOD_ANIM", "TURNING", "CURSOR", "TALK", "LISTEN",
	"STOPPED", "CHOOSE", "FRAMES", "PAUSE", "WAIT_SYNC", "SIMPLE MOD"
};

static const char *const noYes[] = { "no", "yes" };

void Debugger::dumpCompact(uint16 cptId) {
	uint16 type, size;
	char name[256];
	Compact *cpt = _skyCompact->fetchCptInfo(cptId, &size, &type, name);

	if (type == COMPACT) {
		DebugPrintf("Compact %s: id = %04X, section %d, id %d\n", name, cptId, cptId >> 12, cptId & 0xFFF);
		DebugPrintf("logic      : %04X: %s\n", cpt->logic, (cpt->logic <= 16) ? logicTypes[cpt->logic] : "unknown");
		DebugPrintf("status     : %04X\n", cpt->status);
		DebugPrintf("           : background  : %s\n", noYes[(cpt->status &  ST_BACKGROUND) >> 0]);
		DebugPrintf("           : foreground  : %s\n", noYes[(cpt->status &  ST_FOREGROUND) >> 1]);
		DebugPrintf("           : sort list   : %s\n", noYes[(cpt->status &        ST_SORT) >> 2]);
		DebugPrintf("           : recreate    : %s\n", noYes[(cpt->status &    ST_RECREATE) >> 3]);
		DebugPrintf("           : mouse       : %s\n", noYes[(cpt->status &       ST_MOUSE) >> 4]);
		DebugPrintf("           : collision   : %s\n", noYes[(cpt->status &   ST_COLLISION) >> 5]);
		DebugPrintf("           : logic       : %s\n", noYes[(cpt->status &       ST_LOGIC) >> 6]);
		DebugPrintf("           : on grid     : %s\n", noYes[(cpt->status &   ST_GRID_PLOT) >> 7]);
		DebugPrintf("           : ar priority : %s\n", noYes[(cpt->status & ST_AR_PRIORITY) >> 8]);
		DebugPrintf("sync       : %04X\n", cpt->sync);
		DebugPrintf("screen     : %d\n", cpt->screen);
		_skyCompact->fetchCptInfo(cpt->place, NULL, NULL, name);
		DebugPrintf("place      : %04X: %s\n", cpt->place, name);
		_skyCompact->fetchCptInfo(cpt->getToTableId, NULL, NULL, name);
		DebugPrintf("get to tab : %04X: %s\n", cpt->getToTableId, name);
		DebugPrintf("x/y        : %d/%d\n", cpt->xcood, cpt->ycood);
	} else {
		DebugPrintf("Can't dump binary data\n");
	}
}

bool Debugger::Cmd_ShowCompact(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Example: \"%s foster\" dumps compact \"foster\"\n", argv[0]);
		DebugPrintf("Example: \"%s list 1\" lists all compacts from section 1\n", argv[0]);
		DebugPrintf("Example: \"%s list 1 all\" lists all entities from section 1\n", argv[0]);
		return true;
	}

	if (0 == strcmp(argv[1], "list")) {
		bool showAll = false;
		int sectionNumber = -1;
		if (argc >= 3) {
			sectionNumber = atoi(argv[2]);
			if (sectionNumber >= _skyCompact->giveNumDataLists()) {
				DebugPrintf("Section number %d does not exist\n", sectionNumber);
				return true;
			}
			if ((argc == 4) && (scumm_stricmp(argv[3], "all") == 0))
				showAll = true;
		}
		for (int sec = 0; sec < _skyCompact->giveNumDataLists(); sec++) {
			if ((sectionNumber == -1) || (sectionNumber == sec)) {
				DebugPrintf("Compacts in section %d:\n", sec);
				if (showAll) {
					char line[256];
					char *linePos = line;
					for (int cpt = 0; cpt < _skyCompact->giveDataListLen(sec); cpt++) {
						if (cpt != 0) {
							if ((cpt % 3) == 0) {
								DebugPrintf("%s\n", line);
								linePos = line;
							} else
								linePos += sprintf(linePos, ", ");
						}
						uint16 cptId = (uint16)((sec << 12) | cpt);
						uint16 type, size;
						char name[256];
						_skyCompact->fetchCptInfo(cptId, &size, &type, name);
						linePos += sprintf(linePos, "%04X: %10s %22s", cptId, _skyCompact->nameForType(type), name);
					}
					if (linePos != line)
						DebugPrintf("%s\n", line);
				} else {
					for (int cpt = 0; cpt < _skyCompact->giveDataListLen(sec); cpt++) {
						uint16 cptId = (uint16)((sec << 12) | cpt);
						uint16 type, size;
						char name[256];
						_skyCompact->fetchCptInfo(cptId, &size, &type, name);
						if (type == COMPACT)
							DebugPrintf("%04X: %s\n", cptId, name);
					}
				}
			}
		}
	} else {
		uint16 cptId = _skyCompact->findCptId(argv[1]);
		if (cptId == 0)
			DebugPrintf("Unknown compact: '%s'\n", argv[1]);
		else
			dumpCompact(cptId);
	}
	return true;
}

bool Debugger::Cmd_LogicCommand(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Example: %s fn_printf 42\n", argv[0]);
		return true;
	}

	int numMCodes = ARRAYSIZE(mcodes);

	if (0 == strcmp(argv[1], "list")) {
		for (int i = 0; i < numMCodes; ++i) {
			DebugPrintf("%s\n", mcodes[i]);
		}
		return true;
	}

	uint32 arg1 = 0, arg2 = 0, arg3 = 0;

	switch (argc) {
		case  5:
			arg3 = atoi(argv[4]);
		case  4:
			arg2 = atoi(argv[3]);
		case  3:
			arg1 = atoi(argv[2]);
	}

	for (int i = 0; i < numMCodes; ++i) {
		if (0 == strcmp(mcodes[i], argv[1])) {
			_logic->fnExec(i, arg1, arg2, arg3);
			return true;
		}
	}

	DebugPrintf("Unknown function: '%s'\n", argv[1]);

	return true;
}

bool Debugger::Cmd_Info(int argc, const char **argv) {
	DebugPrintf("Beneath a Steel Sky version: 0.0%d\n", SkyEngine::_systemVars.gameVersion);
	DebugPrintf("Speech: %s\n", (SkyEngine::_systemVars.systemFlags & SF_ALLOW_SPEECH) ? "on" : "off");
	DebugPrintf("Text  : %s\n", (SkyEngine::_systemVars.systemFlags & SF_ALLOW_TEXT) ? "on" : "off");
	return true;
}

bool Debugger::Cmd_ScriptVar(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Example: %s lamb_friend <value>\n", argv[0]);
		return true;
	}

	int numScriptVars = ARRAYSIZE(scriptVars);

	if (0 == strcmp(argv[1], "list")) {
		for (int i = 0; i < numScriptVars; ++i) {
			DebugPrintf("%s\n", scriptVars[i]);
		}
		return true;
	}

	for (int i = 0; i < numScriptVars; ++i) {
		if (0 == strcmp(scriptVars[i], argv[1])) {
			if (argc == 3) {
				Logic::_scriptVariables[i] = atoi(argv[2]);
			}
			DebugPrintf("%s = %d\n", argv[1], Logic::_scriptVariables[i]);

			return true;
		}
	}

	DebugPrintf("Unknown ScriptVar: '%s'\n", argv[1]);

	return true;
}

bool Debugger::Cmd_Section(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Example: %s 4\n", argv[0]);
		return true;
	}

	const int baseId[] = { START_ONE, START_S6, START_29, START_SC31, START_SC66, START_SC90, START_SC81 };
	int section = atoi(argv[1]);

	if (section >= 0 && section <= 6) {
		_logic->fnEnterSection(section == 6 ? 4 : section, 0, 0);
		_logic->fnAssignBase(ID_FOSTER, baseId[section], 0);
		_skyCompact->fetchCpt(ID_FOSTER)->megaSet = 0;
	} else {
		DebugPrintf("Unknown section '%s'\n", argv[1]);
	}

	return true;
}

bool Debugger::Cmd_LogicList(int argc, const char **argv) {
	if (argc != 1)
		DebugPrintf("%s does not expect any parameters\n", argv[0]);

	char cptName[256];
	uint16 numElems, type;
	uint16 *logicList = (uint16 *)_skyCompact->fetchCptInfo(Logic::_scriptVariables[LOGIC_LIST_NO], &numElems, &type, cptName);
	DebugPrintf("Current LogicList: %04X (%s)\n", Logic::_scriptVariables[LOGIC_LIST_NO], cptName);
	while (*logicList != 0) {
		if (*logicList == 0xFFFF) {
			uint16 newList = logicList[1];
			logicList = (uint16 *)_skyCompact->fetchCptInfo(newList, &numElems, &type, cptName);
			DebugPrintf("New List: %04X (%s)\n", newList, cptName);
		} else {
			_skyCompact->fetchCptInfo(*logicList, &numElems, &type, cptName);
			DebugPrintf(" Cpt %04X (%s) (%s)\n", *logicList, cptName, _skyCompact->nameForType(type));
			logicList++;
		}
	}
	return true;
}

} // End of namespace Sky
