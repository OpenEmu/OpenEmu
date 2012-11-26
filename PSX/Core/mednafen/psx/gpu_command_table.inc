#define POLY_HELPER_SUB(cv, tm, mam)	\
	 &PS_GPU::Command_DrawPolygon<3 + ((cv & 0x8) >> 3), ((cv & 0x10) >> 4), ((cv & 0x4) >> 2), ((cv & 0x2) >> 1) ? BLENDMODE_MAC : -1, ((cv & 1) ^ 1) & ((cv & 0x4) >> 2), tm, mam >

#define POLY_HELPER(cv) { 1 + (3 /*+ ((cv & 0x8) >> 3)*/) * ( 1 + ((cv & 0x4) >> 2) + ((cv & 0x10) >> 4) ) - ((cv & 0x10) >> 4),	\
	1, false, "Polygon", { POLY_HELPER_SUB(cv, ((cv & 0x4) ? 0 : 0), 0), POLY_HELPER_SUB(cv, ((cv & 0x4) ? 1 : 0), 0),	\
		     POLY_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 0), POLY_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 0),	\
		     POLY_HELPER_SUB(cv, ((cv & 0x4) ? 0 : 0), 1), POLY_HELPER_SUB(cv, ((cv & 0x4) ? 1 : 0), 1),	\
		     POLY_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 1), POLY_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 1),	\
		    }	}

//
//

#define SPR_HELPER_SUB(cv, tm, mam) &PS_GPU::Command_DrawSprite<(cv >> 3) & 0x3,	((cv & 0x4) >> 2), ((cv & 0x2) >> 1) ? BLENDMODE_MAC : -1, ((cv & 1) ^ 1) & ((cv & 0x4) >> 2), tm, mam>

#define SPR_HELPER(cv) { 2 + ((cv & 0x4) >> 2) + ((cv & 0x18) ? 0 : 1), 2 | ((cv & 0x4) >> 2) | ((cv & 0x18) ? 0 : 1), false, "Sprite", {	\
		SPR_HELPER_SUB(cv, ((cv & 0x4) ? 0 : 0), 0), SPR_HELPER_SUB(cv, ((cv & 0x4) ? 1 : 0), 0),	\
		SPR_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 0), SPR_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 0),	\
		SPR_HELPER_SUB(cv, ((cv & 0x4) ? 0 : 0), 1), SPR_HELPER_SUB(cv, ((cv & 0x4) ? 1 : 0), 1),	\
		SPR_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 1), SPR_HELPER_SUB(cv, ((cv & 0x4) ? 2 : 0), 1),	\
		 } }

//
//

#define LINE_HELPER_SUB(cv, mam) &PS_GPU::Command_DrawLine<((cv & 0x08) >> 3), ((cv & 0x10) >> 4), ((cv & 0x2) >> 1) ? BLENDMODE_MAC : -1, mam>

#define LINE_HELPER(cv) { 3 + ((cv & 0x10) >> 4), 1, false, "Line",	\
	 { LINE_HELPER_SUB(cv, 0), LINE_HELPER_SUB(cv, 0), LINE_HELPER_SUB(cv, 0), LINE_HELPER_SUB(cv, 0),	\
	   LINE_HELPER_SUB(cv, 1), LINE_HELPER_SUB(cv, 1), LINE_HELPER_SUB(cv, 1), LINE_HELPER_SUB(cv, 1), } }

//
//

#define OTHER_HELPER(arg_cs, arg_fbcs, arg_ss, arg_name, arg_ptr) { arg_cs, arg_fbcs, arg_ss, arg_name, { arg_ptr, arg_ptr, arg_ptr, arg_ptr, arg_ptr, arg_ptr, arg_ptr, arg_ptr } }

#define NULLCMD() { 1, 1, true, NULL, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } }


 /* 0x00 */
 NULLCMD(),
 OTHER_HELPER(1, 2, false, "Clear Cache", &PS_GPU::Command_ClearCache),
 OTHER_HELPER(3, 3, false, "FB Fill", &PS_GPU::Command_FBFill),

 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0x10 */
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0x20 */
 POLY_HELPER(0x20),
 POLY_HELPER(0x21),
 POLY_HELPER(0x22),
 POLY_HELPER(0x23),
 POLY_HELPER(0x24),
 POLY_HELPER(0x25),
 POLY_HELPER(0x26),
 POLY_HELPER(0x27),
 POLY_HELPER(0x28),
 POLY_HELPER(0x29),
 POLY_HELPER(0x2a),
 POLY_HELPER(0x2b),
 POLY_HELPER(0x2c),
 POLY_HELPER(0x2d),
 POLY_HELPER(0x2e),
 POLY_HELPER(0x2f),
 POLY_HELPER(0x30),
 POLY_HELPER(0x31),
 POLY_HELPER(0x32),
 POLY_HELPER(0x33),
 POLY_HELPER(0x34),
 POLY_HELPER(0x35),
 POLY_HELPER(0x36),
 POLY_HELPER(0x37),
 POLY_HELPER(0x38),
 POLY_HELPER(0x39),
 POLY_HELPER(0x3a),
 POLY_HELPER(0x3b),
 POLY_HELPER(0x3c),
 POLY_HELPER(0x3d),
 POLY_HELPER(0x3e),
 POLY_HELPER(0x3f),

 LINE_HELPER(0x40),
 LINE_HELPER(0x41),
 LINE_HELPER(0x42),
 LINE_HELPER(0x43),
 LINE_HELPER(0x44),
 LINE_HELPER(0x45),
 LINE_HELPER(0x46),
 LINE_HELPER(0x47),
 LINE_HELPER(0x48),
 LINE_HELPER(0x49),
 LINE_HELPER(0x4a),
 LINE_HELPER(0x4b),
 LINE_HELPER(0x4c),
 LINE_HELPER(0x4d),
 LINE_HELPER(0x4e),
 LINE_HELPER(0x4f),
 LINE_HELPER(0x50),
 LINE_HELPER(0x51),
 LINE_HELPER(0x52),
 LINE_HELPER(0x53),
 LINE_HELPER(0x54),
 LINE_HELPER(0x55),
 LINE_HELPER(0x56),
 LINE_HELPER(0x57),
 LINE_HELPER(0x58),
 LINE_HELPER(0x59),
 LINE_HELPER(0x5a),
 LINE_HELPER(0x5b),
 LINE_HELPER(0x5c),
 LINE_HELPER(0x5d),
 LINE_HELPER(0x5e),
 LINE_HELPER(0x5f),

 SPR_HELPER(0x60),
 SPR_HELPER(0x61),
 SPR_HELPER(0x62),
 SPR_HELPER(0x63),
 SPR_HELPER(0x64),
 SPR_HELPER(0x65),
 SPR_HELPER(0x66),
 SPR_HELPER(0x67),
 SPR_HELPER(0x68),
 SPR_HELPER(0x69),
 SPR_HELPER(0x6a),
 SPR_HELPER(0x6b),
 SPR_HELPER(0x6c),
 SPR_HELPER(0x6d),
 SPR_HELPER(0x6e),
 SPR_HELPER(0x6f),
 SPR_HELPER(0x70),
 SPR_HELPER(0x71),
 SPR_HELPER(0x72),
 SPR_HELPER(0x73),
 SPR_HELPER(0x74),
 SPR_HELPER(0x75),
 SPR_HELPER(0x76),
 SPR_HELPER(0x77),
 SPR_HELPER(0x78),
 SPR_HELPER(0x79),
 SPR_HELPER(0x7a),
 SPR_HELPER(0x7b),
 SPR_HELPER(0x7c),
 SPR_HELPER(0x7d),
 SPR_HELPER(0x7e),
 SPR_HELPER(0x7f),

 /* 0x80 */
 OTHER_HELPER(4, 2, false, "FB Copy", &PS_GPU::Command_FBCopy),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0x90 */
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xA0 */
 OTHER_HELPER(3, 2, false, "FB Write", &PS_GPU::Command_FBWrite),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xB0 */
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xC0 */
 OTHER_HELPER(3, 2, false, "FB Read", &PS_GPU::Command_FBRead),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xD0 */
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xE0 */

 NULLCMD(),
 OTHER_HELPER(1, 2, false, "Draw mode settings", &PS_GPU::Command_DrawMode),
 OTHER_HELPER(1, 2, false, "Texture window settings", &PS_GPU::Command_TexWindow),
 OTHER_HELPER(1, 1, true, "Drawing area top left", &PS_GPU::Command_Clip0),
 OTHER_HELPER(1, 1, true, "Drawing area bottom right", &PS_GPU::Command_Clip1),
 OTHER_HELPER(1, 1, true, "Drawing offset", &PS_GPU::Command_DrawingOffset),
 OTHER_HELPER(1, 2, false, "Mask settings",  &PS_GPU::Command_MaskSetting),

 NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

 /* 0xF0 */
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),
 NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(), NULLCMD(),

