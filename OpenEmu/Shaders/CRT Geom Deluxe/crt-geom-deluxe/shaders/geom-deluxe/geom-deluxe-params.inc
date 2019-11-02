layout(push_constant) uniform Push
{
   float phosphor_power, phosphor_amplitude, CRTgamma, width, aspect_x, aspect_y, d, R, angle_x, angle_y,
      aperture_strength, halation, curvature, cornersize, cornersmooth, overscan_x, overscan_y,
      monitorgamma, mask_type;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
   vec4 FinalViewportSize;
   vec4 internal1Size;
	mat4 MVP;
} global;

#pragma parameter mask_type "Mask (none, aperture, slot, delta)" 1.0 0.0 3.0 1.0
#pragma parameter aperture_strength "Shadow mask strength" 0.4 0.0 1.0 0.05
#pragma parameter phosphor_power "Phosphor decay power" 1.2 0.5 3.0 0.05
#pragma parameter phosphor_amplitude "Phosphor persistence amplitude" 0.04 0.0 0.2 0.01
#pragma parameter CRTgamma "Gamma of simulated CRT" 2.4 0.7 4.0 0.05 
#pragma parameter halation "Halation amplitude" 0.1 0.0 0.3 0.01
#pragma parameter width "Halation blur width" 2.0 0.1 4.0 0.1
#pragma parameter curvature "Enable Curvature" 1.0 0.0 1.0 1.0
#pragma parameter R "Radius of curvature" 3.5 0.5 10.0 0.1
#pragma parameter d "Distance to screen" 2.0 0.1 10.0 0.1
#pragma parameter angle_x "Tilt X" 0.0 -1.0 1.0 0.01
#pragma parameter angle_y "Tilt Y" 0.0 -1.0 1.0 0.01
#pragma parameter cornersize "Rounded corner size" 0.01 0.00 0.10 0.01
#pragma parameter cornersmooth "Border smoothness" 1000.0 100.0 2000.0 100.0
#pragma parameter overscan_x "Overscan X" 1.0 0.8 1.2 0.02
#pragma parameter overscan_y "Overscan Y" 1.0 0.8 1.2 0.02
#pragma parameter monitorgamma "Gamma of output display" 2.2 0.7 4.0 0.05
#pragma parameter aspect_x "Aspect ratio X" 1.0 0.3 1.0 0.01
#pragma parameter aspect_y "Aspect ratio Y" 0.75 0.3 1.0 0.01

#define phosphor_power params.phosphor_power
#define phosphor_amplitude params.phosphor_amplitude
#define CRTgamma params.CRTgamma
#define width params.width
#define curvature params.curvature
#define d params.d
#define cornersize params.cornersize
#define cornersmooth params.cornersmooth
#define monitorgamma params.monitorgamma
#define halation params.halation
#define aperture_strength params.aperture_strength
vec2 aspect = vec2(params.aspect_x, params.aspect_y);
vec2 angle = vec2(params.angle_x, params.angle_y);
vec2 overscan = vec2(params.overscan_x, params.overscan_y);
int mask_picker = int(params.mask_type);

const float gamma = 2.2;