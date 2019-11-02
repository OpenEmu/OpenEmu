layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
// effect toggles and multi
   float bloomtoggle;
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
	float bloomscale;
	float bloomoverdrive_r;
	float bloomoverdrive_g;
	float bloomoverdrive_b;
	float level0weight;
	float level1weight;
	float level2weight;
	float level3weight;
	float level4weight;
	float level5weight;
	float level6weight;
	float level7weight;
	float level8weight;
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
   float notchhalfwidth;
   float yfreqresponse;
   float ifreqresponse;
   float qfreqresponse;
   float signaloffset;
// color params
	float col_red;
	float col_grn;
	float col_blu;
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
// defocus params
	float defocus_x;
	float defocus_y;
// phosphor params
   float deltatime;
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
	float reflection_col_r;
	float reflection_col_g;
	float reflection_col_b;
// vector params
// float timeratio;
// float timescale;
// float lengthratio;
// float lengthscale;
// float beamsmooth;
} global;

// Effect Toggles and Settings Used In Multiple Passes
#pragma parameter ntscsignal "NTSC Signal Mode"     1.0 0.0 1.0 1.0
bool NTSCSignal = bool(global.ntscsignal);
#pragma parameter scanlinetoggle "Scanline"         1.0 0.0 1.0 1.0
bool ScanlineToggle = bool(global.scanlinetoggle);
#pragma parameter bloomtoggle "Bloom Enable"        1.0 0.0 1.0 1.0
bool BloomToggle = bool(global.bloomtoggle);
#pragma parameter chromatoggle "Chromaticity"       0.0 0.0 1.0 1.0
bool Chromaticity = bool(global.chromatoggle);
#pragma parameter distortiontoggle "Distortion"     1.0 0.0 1.0 1.0
bool Distortion = bool(global.distortiontoggle);
#pragma parameter phosphortoggle "Phosphor"         1.0 0.0 1.0 1.0
bool Passthrough = !bool(global.phosphortoggle);
//#pragma parameter vectorscreen "Vector Screen Mode" 0.0 0.0 1.0 1.0 // TODO/FIXME
const float vectorscreen = 0.0;
bool VectorScreen = bool(vectorscreen);

#pragma parameter screenscale_x "Screen Scale X" 1.0 0.5 3.0 0.01
#pragma parameter screenscale_y "Screen Scale Y" 1.0 0.5 3.0 0.01
vec2 ScreenScale = vec2(global.screenscale_x, global.screenscale_y);
#pragma parameter screenoffset_x "Screen Offset X" 0.0 -1.0 1.0 0.005
#pragma parameter screenoffset_y "Screen Offset Y" 0.0 -1.0 1.0 0.005
vec2 ScreenOffset = vec2(global.screenoffset_x, global.screenoffset_y);
#pragma parameter swapxy "Swap X and Y" 0.0 0.0 1.0 1.0
bool SwapXY = bool(global.swapxy);

// Bloom Pass
#pragma parameter bloomblendmode "Bloom Blend Mode (Brighten, Darken)"    0.0  0.0 1.0 1.0
#pragma parameter bloomscale "Bloom Scale"             0.33 0.0 2.0 0.05
#pragma parameter bloomoverdrive_r "Bloom Overdrive R" 1.0 0.0 2.0 0.01
#pragma parameter bloomoverdrive_g "Bloom Overdrive G" 1.0 0.0 2.0 0.01
#pragma parameter bloomoverdrive_b "Bloom Overdrive B" 1.0 0.0 2.0 0.01
#pragma parameter level0weight "Bloom Level 0 Weight" 0.64 0.0 1.0 0.01
#pragma parameter level1weight "Bloom Level 1 Weight" 0.64 0.0 1.0 0.01
#pragma parameter level2weight "Bloom Level 2 Weight" 0.32 0.0 1.0 0.01
#pragma parameter level3weight "Bloom Level 3 Weight" 0.16 0.0 1.0 0.01
#pragma parameter level4weight "Bloom Level 4 Weight" 0.08 0.0 1.0 0.01
#pragma parameter level5weight "Bloom Level 5 Weight" 0.06 0.0 1.0 0.01
#pragma parameter level6weight "Bloom Level 6 Weight" 0.04 0.0 1.0 0.01
#pragma parameter level7weight "Bloom Level 7 Weight" 0.02 0.0 1.0 0.01
#pragma parameter level8weight "Bloom Level 8 Weight" 0.01 0.0 1.0 0.01

// Post Pass
#pragma parameter humbaralpha "Hum Bar Alpha" 0.0 0.0 1.0 0.01
#pragma parameter backcolor_r "Back Color R" 0.0 0.0 1.0 0.01
#pragma parameter backcolor_g "Back Color G" 0.0 0.0 1.0 0.01
#pragma parameter backcolor_b "Back Color B" 0.0 0.0 1.0 0.01
#pragma parameter shadowtilemode "Mask Tiling Mode (Screen, Source)" 0.0 0.0 1.0 1.0
#pragma parameter shadowalpha "Mask Alpha" 0.3 0.0 1.0 0.01
#pragma parameter shadowcount_x "Mask Tile Size X" 12.0 1.0 32.0 1.0
#pragma parameter shadowcount_y "Mask Tile Size Y" 12.0 1.0 32.0 1.0
#pragma parameter shadowuv_x "Mask UV X" 0.25 0.03125 1.0 0.03125
#pragma parameter shadowuv_y "Mask UV Y" 0.25 0.03125 1.0 0.03125
#pragma parameter mask_width "Mask Texture Width"       32.0 0.0 256.0 16.0
#pragma parameter mask_height "Mask Texture Height"     32.0 0.0 256.0 16.0
#pragma parameter mask_offset_x "Mask Offset X"         0.0 -1.0 1.0 0.01
#pragma parameter mask_offset_y "Mask Offset Y"         0.0 -1.0 1.0 0.01
#pragma parameter chromamode "Color Mode (Monochrome, Dichrome, Trichrome)" 3.0 1.0 3.0 1.0
#pragma parameter conversiongain_x "Conversion Gain X" 0.0 -5.0 5.0 0.5
#pragma parameter conversiongain_y "Conversion Gain Y" 0.0 -5.0 5.0 0.5
#pragma parameter conversiongain_z "Conversion Gain Z" 0.0 -5.0 5.0 0.5
#pragma parameter power_r "Color Power R" 1.0 1.0 10.0 1.0
#pragma parameter power_g "Color Power G" 1.0 1.0 10.0 1.0
#pragma parameter power_b "Color Power B" 1.0 1.0 10.0 1.0
#pragma parameter floor_r "Color Floor R" 0.0 0.0 1.0 0.01
#pragma parameter floor_g "Color Floor G" 0.0 0.0 1.0 0.01
#pragma parameter floor_b "Color Floor B" 0.0 0.0 1.0 0.01
#pragma parameter preparebloom "Prepare Bloom" 0.0 0.0 1.0 1.0

// NTSC Pass
#pragma parameter avalue "A Value"                          0.5 -1.0 1.0 0.01
#pragma parameter bvalue "B Value"                          0.5 -1.0 1.0 0.01
#pragma parameter ccvalue "Color Carrier (Hz)"              3.5795454 0.0 6.0 0.00001
#pragma parameter ovalue "Outgoing Phase Offset"            0.0 -3.0 3.0 0.01
#pragma parameter pvalue "Incoming Phase Pixel Clock Scale" 1.0 -3.0 3.0 0.01
#pragma parameter scantime "Scanline Duration (uSec)"       52.6 0.0 100.0 0.1
#pragma parameter notchhalfwidth "Color Notch Filter Width" 1.0 0.0 6.0 0.05
#pragma parameter yfreqresponse "Y Signal Bandwidth (Hz)"   6.0 0.0 21.0 0.01
#pragma parameter ifreqresponse "I Signal Bandwidth (Hz)"   1.2 0.0 21.0 0.01
#pragma parameter qfreqresponse "Q Signal Bandwidth (Hz)"   0.6 0.0 21.0 0.01
#pragma parameter signaloffset "Signal Offset"              1.0 0.0 1.0 0.01

// Color Pass
#pragma parameter col_red "Red Shift" 1.0 0.0 2.0 0.01
#pragma parameter col_grn "Green Shift" 1.0 0.0 2.0 0.01
#pragma parameter col_blu "Blue Shift" 1.0 0.0 2.0 0.01
#pragma parameter col_offset_x "Offset X" 0.0 -1.0 1.0 0.01
#pragma parameter col_offset_y "Offset Y" 0.0 -1.0 1.0 0.01
#pragma parameter col_offset_z "Offset Z" 0.0 -1.0 1.0 0.01
#pragma parameter col_scale_x "Scale X" 1.0 0.0 2.0 0.01
#pragma parameter col_scale_y "Scale Y" 1.0 0.0 2.0 0.01
#pragma parameter col_scale_z "Scale Z" 1.0 0.0 2.0 0.01
#pragma parameter col_saturation "Saturation" 1.2 0.0 5.0 0.01

// Deconverge Pass
#pragma parameter converge_x_r "Red Linear Convergence, X"      0.0 -10.0 10.0 0.05
#pragma parameter converge_y_r "Red Linear Convergence, Y"      0.0 -10.0 10.0 0.05
#pragma parameter converge_x_g "Green Linear Convergence, X"    0.0 -10.0 10.0 0.05
#pragma parameter converge_y_g "Green Linear Convergence, Y"    0.0 -10.0 10.0 0.05
#pragma parameter converge_x_b "Blue Linear Convergence, X"     0.0 -10.0 10.0 0.05
#pragma parameter converge_y_b "Blue Linear Convergence, Y"     0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_x_r "Red Radial Convergence, X"   0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_y_r "Red Radial Convergence, Y"   0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_x_g "Green Radial Convergence, X" 0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_y_g "Green Radial Convergence, Y" 0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_x_b "Blue Radial Convergence, X"  0.0 -10.0 10.0 0.05
#pragma parameter radial_conv_y_b "Blue Radial Convergence, Y"  0.0 -10.0 10.0 0.05

// Scanline Pass
#pragma parameter scanlinealpha "Scanline Alpha"                    0.0  0.0 1.0 0.01
#pragma parameter scanlinescale "Overall Scanline Scale"            1.0  0.0 4.0 0.01
#pragma parameter scanlineheight "Individual Scanline Scale"        1.0  0.0 4.0 0.01
#pragma parameter scanlinevariation "Scanline Variation"            1.0  0.0 4.0 0.01
#pragma parameter scanlineoffset "Scanline Offset"                  1.5 -1.5 3.0 0.01
#pragma parameter scanlinebrightscale "Scanline Brightness Scale"   1.0  0.0 2.0 0.01
#pragma parameter scanlinebrightoffset "Scanline Brightness Offset" 1.5 -1.5 4.0 0.01

// Defocus Pass
#pragma parameter defocus_x "Defocus X Axis" 0.5 0.0 10.0 0.1
#pragma parameter defocus_y "Defocus Y Axis" 0.5 0.0 10.0 0.1

// Phosphor Pass
#pragma parameter deltatime "Delta Time"                    0.50 0.0 2.0 0.1
#pragma parameter phosphor_r "Phosphor Persistence Red"     0.45 0.0 1.0 0.01
#pragma parameter phosphor_g "Phosphor Persistence Green"   0.45 0.0 1.0 0.01
#pragma parameter phosphor_b "Phosphor Persistence Blue"    0.45 0.0 1.0 0.01

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
#pragma parameter distortion_amount "Quadric Distortion Amount"     0.05 -2.0 2.0 0.01
#pragma parameter cubic_distortion_amount "Cubic Distortion Amount" 0.00 -2.0 2.0 0.01
#pragma parameter distort_corner_amount "Distorted Corner Amount"   0.05  0.0 2.0 0.01
#pragma parameter round_corner_amount "Rounded Corner Amount"       0.05  0.0 2.0 0.01
#pragma parameter smooth_border_amount "Smooth Border Amount"       0.03  0.0 2.0 0.01
#pragma parameter vignette_amount "Vignetting Amount"               0.08  0.0 2.0 0.01
#pragma parameter reflection_amount "Reflection Amount"             0.00 0.0 1.0 0.01
#pragma parameter reflection_col_r "Reflection Color R" 1.0 0.0 1.0 0.01
#pragma parameter reflection_col_g "Reflection Color G" 0.9 0.0 1.0 0.01
#pragma parameter reflection_col_b "Reflection Color B" 0.8 0.0 1.0 0.01

// Vector Pass
//#pragma parameter timeratio "Time Ratio" 1.0 0.0 2.0 0.01
//#pragma parameter timescale "Time Scale" 1.0 1.0 10.0 1.0
//#pragma parameter lengthratio "Length Ratio" 1.0 1.0 10.0 1.0
//#pragma parameter lengthscale "Length Scale" 1.0 1.0 10.0 1.0
//#pragma parameter beamsmooth "Beam Smooth Amt" 0.5 0.1 1.0 0.1
