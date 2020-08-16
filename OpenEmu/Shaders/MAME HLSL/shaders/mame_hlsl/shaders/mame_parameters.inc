layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
// effect toggles and multi
//   float bloomtoggle;
   float ntscsignal;
   float scanlinetoggle;
   float chromatoggle;
   float distortiontoggle;
   float screenscale_x;
   float screenscale_y;
   float screenoffset_x;
   float screenoffset_y;
   float swapxy;
// bloom params
   float bloomblendmode;
//	float vectorscreen;
//	float bloomscale;
//	float bloomoverdrive_r;
//	float bloomoverdrive_g;
//	float bloomoverdrive_b;
//	float level0weight;
//	float level1weight;
//	float level2weight;
//	float level3weight;
//	float level4weight;
//	float level5weight;
//	float level6weight;
//	float level7weight;
//	float level8weight;
// post params
   float mask_width;
   float mask_height;
   float mask_offset_x;
   float mask_offset_y;
   float preparebloom;
   float shadowtilemode;
   float power_r;
   float power_g;
   float power_b;
   float floor_r;
   float floor_g;
   float floor_b;
   float chromamode;
   float conversiongain_x;
   float conversiongain_y;
   float conversiongain_z;
   float humbaralpha;
   float backcolor_r;
   float backcolor_g;
   float backcolor_b;
   float shadowalpha;
   float shadowcount_x;
   float shadowcount_y;
   float shadowuv_x;
   float shadowuv_y;
// ntsc params
   float avalue;
   float bvalue;
   float ccvalue;
   float ovalue;
   float pvalue;
   float scantime;
   float notch_width;
   float yfreqresponse;
   float ifreqresponse;
   float qfreqresponse;
   float jitter_offset;
// color params
	float col_rr;
	float col_rg;
	float col_rb;
	float col_gr;
	float col_gg;
	float col_gb;
	float col_br;
	float col_bg;
	float col_bb;
	float col_offset_x;
	float col_offset_y;
	float col_offset_z;
	float col_scale_x;
	float col_scale_y;
	float col_scale_z;
	float col_saturation;
// deconverge params
	float converge_x_r;
	float converge_x_g;
	float converge_x_b;
	float converge_y_r;
	float converge_y_g;
	float converge_y_b;
	float radial_conv_x_r;
	float radial_conv_x_g;
	float radial_conv_x_b;
	float radial_conv_y_r;
	float radial_conv_y_g;
	float radial_conv_y_b;
// scanline params
   float scanlinealpha;
   float scanlinescale;
   float scanlineheight;
   float scanlinevariation;
   float scanlineoffset;
   float scanlinebrightscale;
   float scanlinebrightoffset;
   float scanline_jitter_amount;
// defocus params
	float defocus_x;
	float defocus_y;
// phosphor params
   float time;
   float phosphor_r;
   float phosphor_g;
   float phosphor_b;
   float phosphortoggle;
// chroma params
   float ygain_r;
   float ygain_g;
   float ygain_b;
   float chromaa_x;
   float chromaa_y;
   float chromab_x;
   float chromab_y;
   float chromac_x;
   float chromac_y;
// distortion params
	float distortion_amount;
	float cubic_distortion_amount;
	float distort_corner_amount;
	float round_corner_amount;
	float smooth_border_amount;
	float vignette_amount;
	float reflection_amount;
//	float reflection_col_r;
//	float reflection_col_g;
//	float reflection_col_b;
   float jitter;
   float humbar_hertz_rate;
   float prescale;
   float scanline_crawl;
   float noise_amt;
// vector params
// float timeratio;
// float timescale;
// float lengthratio;
// float lengthscale;
// float beamsmooth;
} global;

// Effect Toggles and Settings Used In Multiple Passes
#pragma parameter ntscsignal "NTSC Signal Mode" 0.0 0.0 1.0 1.0
bool NTSCSignal = bool(global.ntscsignal);
#pragma parameter scanlinetoggle "Scanline Toggle" 1.0 0.0 1.0 1.0
bool ScanlineToggle = bool(global.scanlinetoggle);
//#pragma parameter bloomtoggle "Bloom Enable" 0.0 0.0 1.0 1.0
//bool BloomToggle = bool(global.bloomtoggle);
#pragma parameter chromatoggle "Chromaticity Toggle" 0.0 0.0 1.0 1.0
bool Chromaticity = bool(global.chromatoggle);
#pragma parameter distortiontoggle "Distortion Toggle" 0.0 0.0 1.0 1.0
bool Distortion = bool(global.distortiontoggle);
#pragma parameter phosphortoggle "Phosphor Toggle" 0.0 0.0 1.0 1.0
bool Passthrough = !bool(global.phosphortoggle);
#pragma parameter screenscale_x "Screen Scale X" 0.0 -3.0 3.0 0.005
#pragma parameter screenscale_y "Screen Scale Y" 0.0 -3.0 3.0 0.005
vec2 ScreenScale = vec2(global.screenscale_x, global.screenscale_y);
//#pragma parameter vectorscreen "Vector Screen Mode" 0.0 0.0 1.0 1.0 // TODO/FIXME
const float vectorscreen = 0.0;
bool VectorScreen = bool(vectorscreen);
#pragma parameter jitter "Scanline Jitter (OFF/ON/AUTO)" 0.0 0.0 2.0 1.0
#pragma parameter scanline_crawl "Scanline Crawl" 0.0 0.0 1.0 1.0

#pragma parameter screenoffset_x "Screen Offset X" 0.0 -1.0 1.0 0.001
#pragma parameter screenoffset_y "Screen Offset Y" 0.0 -1.0 1.0 0.001
vec2 ScreenOffset = vec2(global.screenoffset_x, global.screenoffset_y);
#pragma parameter swapxy "Swap X and Y" 0.0 0.0 1.0 1.0
bool SwapXY = bool(global.swapxy);

// Bloom Pass
//#pragma parameter bloomblendmode "Bloom Blend Mode" 0.0 0.0 1.0 1.0
//#pragma parameter bloomscale "Bloom Scale" 0.33 0.0 1.0 0.01
//#pragma parameter bloomoverdrive_r "Bloom Overdrive R" 1.0 0.0 2.0 0.01
//#pragma parameter bloomoverdrive_g "Bloom Overdrive G" 1.0 0.0 2.0 0.01
//#pragma parameter bloomoverdrive_b "Bloom Overdrive B" 1.0 0.0 2.0 0.01
//#pragma parameter level0weight "Bloom Level 0 Weight" 0.64 0.0 1.0 0.01
//#pragma parameter level1weight "Bloom Level 1 Weight" 0.64 0.0 1.0 0.01
//#pragma parameter level2weight "Bloom Level 2 Weight" 0.32 0.0 1.0 0.01
//#pragma parameter level3weight "Bloom Level 3 Weight" 0.16 0.0 1.0 0.01
//#pragma parameter level4weight "Bloom Level 4 Weight" 0.08 0.0 1.0 0.01
//#pragma parameter level5weight "Bloom Level 5 Weight" 0.06 0.0 1.0 0.01
//#pragma parameter level6weight "Bloom Level 6 Weight" 0.04 0.0 1.0 0.01
//#pragma parameter level7weight "Bloom Level 7 Weight" 0.02 0.0 1.0 0.01
//#pragma parameter level8weight "Bloom Level 8 Weight" 0.01 0.0 1.0 0.01

// Post Pass
#pragma parameter humbaralpha "Hum Bar Amount" 0.0 0.0 1.0 0.01
#pragma parameter humbar_hertz_rate "Hum Bar Frequency" 0.001 0.0 1.0 0.001
//#pragma parameter backcolor_r "Back Color R" 0.0 0.0 1.0 0.01
//#pragma parameter backcolor_g "Back Color G" 0.0 0.0 1.0 0.01
//#pragma parameter backcolor_b "Back Color B" 0.0 0.0 1.0 0.01
#pragma parameter shadowtilemode "Shadow Mask Tile Mode" 0.0 0.0 1.0 1.0
#pragma parameter shadowalpha "Shadow Mask Amount" 0.5 0.0 1.0 0.01
#pragma parameter shadowcount_x "Shadow Mask Pixel Count X" 12.0 1.0 128.0 1.0
#pragma parameter shadowcount_y "Shadow Mask Pixel Count Y" 12.0 1.0 128.0 1.0
#pragma parameter shadowuv_x "Shadow Mask UV Size X" 0.5 0.0 1.0 0.001
#pragma parameter shadowuv_y "Shadow Mask UV Size Y" 0.5 0.0 1.0 0.001
//#pragma parameter mask_width "Mask Texture Width" 32.0 0.0 256.0 16.0
//#pragma parameter mask_height "Mask Texture Height" 32.0 0.0 256.0 16.0
#pragma parameter mask_offset_x "Mask Offset X" 0.0 0.0 10.0 0.001
#pragma parameter mask_offset_y "Mask Offset Y" 0.0 0.0 10.0 0.001
#pragma parameter chromamode "Chroma Mode" 3.0 1.0 3.0 1.0
#pragma parameter conversiongain_x "Conversion Gain X" 0.0 -5.0 5.0 0.5
#pragma parameter conversiongain_y "Conversion Gain Y" 0.0 -5.0 5.0 0.5
#pragma parameter conversiongain_z "Conversion Gain Z" 0.0 -5.0 5.0 0.5
#pragma parameter power_r "Color Power R" 1.0 1.0 10.0 1.0
#pragma parameter power_g "Color Power G" 1.0 1.0 10.0 1.0
#pragma parameter power_b "Color Power B" 1.0 1.0 10.0 1.0
#pragma parameter floor_r "Color Floor R" 0.0 0.0 1.0 0.01
#pragma parameter floor_g "Color Floor G" 0.0 0.0 1.0 0.01
#pragma parameter floor_b "Color Floor B" 0.0 0.0 1.0 0.01
//#pragma parameter preparebloom "Prepare Bloom" 0.0 0.0 1.0 1.0

// NTSC Pass
#pragma parameter avalue "NTSC A Value" 0.5 0.0 1.0 0.01
#pragma parameter bvalue "NTSC B Value" 0.5 0.0 1.0 0.01
#pragma parameter ccvalue "NTSC Color Carrier (Hz)" 3.5795454 3.47954 3.67954 0.00001
#pragma parameter ovalue "NTSC Outgoing Phase Offset" 0.0 0.0 1.0 0.01
#pragma parameter pvalue "NTSC Incoming Phase Pixel Clock Scale" 1.0 0.0 2.0 0.01
#pragma parameter scantime "NTSC Scanline Duration (uSec)" 52.6 1.0 150.0 0.1
#pragma parameter notch_width "NTSC Color Notch Filter Width" 2.0 1.0 4.0 0.01
#pragma parameter yfreqresponse "NTSC Y Signal Bandwidth (Hz)" 6.0 0.0 21.0 0.05
#pragma parameter ifreqresponse "NTSC I Signal Bandwidth (Hz)" 1.2 0.0 21.0 0.05
#pragma parameter qfreqresponse "NTSC Q Signal Bandwidth (Hz)" 0.6 0.0 21.0 0.05
#pragma parameter jitter_offset "NTSC Frame Jitter Offset" 0.0 0.0 2.0 0.01

// Color Pass
#pragma parameter col_rr "Red Shift Red" 1.0 0.0 2.0 0.01
#pragma parameter col_rg "Red Shift Green" 0.0 0.0 2.0 0.01
#pragma parameter col_rb "Red Shift Blue" 0.0 0.0 2.0 0.01
#pragma parameter col_gr "Green Shift Red" 0.0 0.0 2.0 0.01
#pragma parameter col_gg "Green Shift Green" 1.0 0.0 2.0 0.01
#pragma parameter col_gb "Green Shift Blue" 0.0 0.0 2.0 0.01
#pragma parameter col_br "Blue Shift Red" 0.0 0.0 2.0 0.01
#pragma parameter col_bg "Blue Shift Green" 0.0 0.0 2.0 0.01
#pragma parameter col_bb "Blue Shift Blue" 1.0 0.0 2.0 0.01
#pragma parameter col_offset_x "Offset X" 0.0 0.0 2.0 0.01
#pragma parameter col_offset_y "Offset Y" 0.0 0.0 2.0 0.01
#pragma parameter col_offset_z "Offset Z" 0.0 0.0 2.0 0.01
#pragma parameter col_scale_x "Scale X" 0.95 0.0 2.0 0.01
#pragma parameter col_scale_y "Scale Y" 0.95 0.0 2.0 0.01
#pragma parameter col_scale_z "Scale Z" 0.95 0.0 2.0 0.01
#pragma parameter col_saturation "Color Saturation" 1.2 0.0 4.0 0.01

// Deconverge Pass
#pragma parameter converge_x_r "Convergence X Red" 0.0 -10.0 10.0 0.05
#pragma parameter converge_x_g "Convergence X Green" 0.0 -10.0 10.0 0.05
#pragma parameter converge_x_b "Convergence X Blue" 0.0 -10.0 10.0 0.05
#pragma parameter converge_y_r "Convergence Y Red" 0.0 -10.0 10.0 0.05
#pragma parameter converge_y_g "Convergence Y Green" 0.0 -10.0 10.0 0.05
#pragma parameter converge_y_b "Convergence Y Blue" 0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_x_r "Radial Conv X Red" 0.0 -10.0 10.0 0.01
#pragma parameter radial_conv_x_g "Radial Conv X Green" 0.0 -10.0 10.0 0.01
#pragma parameter radial_conv_x_b "Radial Conv X Blue" 0.0 -10.0 10.0 0.01
#pragma parameter radial_conv_y_r "Radial Conv Y Red" 0.0 -10.0 10.0 0.01
#pragma parameter radial_conv_y_g "Radial Conv Y Green" 0.0 -10.0 10.0 0.01
#pragma parameter radial_conv_y_b "Radial Conv Y Blue" 0.0 -10.0 10.0 0.01

// Scanline Pass
#pragma parameter scanlinealpha "Scanline Amount" 0.35 0.0 1.0 0.01
#pragma parameter scanlinescale "Overall Scanline Scale" 1.0 0.0 4.0 0.01
#pragma parameter scanlineheight "Individual Scanline Scale" 1.0 0.0 4.0 0.01
#pragma parameter scanlinevariation "Scanline Variation" 1.0 0.0 4.0 0.01
//#pragma parameter scanlineoffset "Scanline Offset" 1.0 -1.5 3.0 0.01
#pragma parameter scanlinebrightscale "Scanline Brightness Scale" 2.0 0.0 4.0 0.01
#pragma parameter scanlinebrightoffset "Scanline Brightness Offset" 1.5 0.0 4.0 0.01
#pragma parameter scanline_jitter_amount "Scanline Jitter Amount" 0.25 0.0 2.0 0.01

// Defocus Pass
#pragma parameter defocus_x "Defocus X Axis" 0.0 0.0 10.0 0.1
#pragma parameter defocus_y "Defocus Y Axis" 0.0 0.0 10.0 0.1

// Phosphor Pass
#pragma parameter phosphor_r "Phosphor Red" 0.45 0.0 0.99 0.01
#pragma parameter phosphor_g "Phosphor Green" 0.45 0.0 0.99 0.01
#pragma parameter phosphor_b "Phosphor Blue" 0.45 0.0 0.99 0.01

// Chroma Pass
#pragma parameter ygain_r "Y Gain R Channel" 0.2126 0.0 1.0 0.01
#pragma parameter ygain_g "Y Gain G Channel" 0.7152 0.0 1.0 0.01
#pragma parameter ygain_b "Y Gain B Channel" 0.0722 0.0 1.0 0.01
#pragma parameter chromaa_x "Chroma A X" 0.630 0.0 1.0 0.01
#pragma parameter chromaa_y "Chroma A Y" 0.340 0.0 1.0 0.01
#pragma parameter chromab_x "Chroma B X" 0.310 0.0 1.0 0.01
#pragma parameter chromab_y "Chroma B Y" 0.595 0.0 1.0 0.01
#pragma parameter chromac_x "Chroma C X" 0.155 0.0 1.0 0.01
#pragma parameter chromac_y "Chroma C Y" 0.070 0.0 1.0 0.01

// Distortion Pass
#pragma parameter distortion_amount "Quadric Dist. Amt" 0.05 -2.0 2.0 0.01
#pragma parameter cubic_distortion_amount "Cubic Dist. Amt" 0.0 -2.0 2.0 0.01
#pragma parameter distort_corner_amount "Dist. Corner Amt" 0.05 0.0 2.0 0.01
#pragma parameter round_corner_amount "Rounded Corner Amt" 0.05 0.0 2.0 0.01
#pragma parameter smooth_border_amount "Smooth Border Amt" 0.03 0.0 2.0 0.01
#pragma parameter vignette_amount "Vignetting Amount" 0.08 0.0 2.0 0.01
#pragma parameter reflection_amount "Reflection Amount" 0.05 0.0 2.0 0.01
#pragma parameter noise_amt "Noise/Grain (reduces moire)" 1.0 0.0 5.0 0.25
//#pragma parameter reflection_col_r "Reflection Color R" 1.0 0.0 1.0 0.01
//#pragma parameter reflection_col_g "Reflection Color G" 1.0 0.0 1.0 0.01
//#pragma parameter reflection_col_b "Reflection Color B" 1.0 0.0 1.0 0.01

// Vector Pass
//#pragma parameter timeratio "Time Ratio" 1.0 0.0 2.0 0.01
//#pragma parameter timescale "Time Scale" 1.0 1.0 10.0 1.0
//#pragma parameter lengthratio "Length Ratio" 1.0 1.0 10.0 1.0
//#pragma parameter lengthscale "Length Scale" 1.0 1.0 10.0 1.0
//#pragma parameter beamsmooth "Beam Smooth Amt" 0.5 0.1 1.0 0.1

// Prescale Pass
#pragma parameter prescale "PreScale Factor" 2.0 1.0 4.0 1.0 

vec4 u_swap_xy = vec4(global.swapxy);
vec4 u_source_dims = params.OriginalSize;
vec4 u_target_dims = params.OutputSize;
vec4 u_target_scale = params.OutputSize / params.SourceSize;
vec4 u_screen_scale = vec4(1.-global.screenscale_x, 1.-global.screenscale_y, 1., 1.);
vec4 u_screen_offset = vec4(global.screenoffset_x, global.screenoffset_y, 0.,0.);
vec4 u_screen_count = vec4(1.);
vec4 u_screen_dims = vec4(params.OutputSize);
vec4 u_quad_dims = params.FinalViewportSize;
vec4 u_time = vec4(params.FrameCount);//vec4(mod(params.FrameCount, global.time / 16.));