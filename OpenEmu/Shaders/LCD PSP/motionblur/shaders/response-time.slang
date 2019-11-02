#version 450
/*
    Response Time
    Based on the response time function from Harlequin's Game Boy and LCD shaders
 
    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.
*/

layout(push_constant) uniform Push
{
    float response_time;
} params;

// Simulate response time
// Higher values result in longer color transition periods - [0, 1]
#pragma parameter response_time "LCD Response Time" 0.333 0.0 0.777 0.111

layout(std140, set = 0, binding = 0) uniform UBO
{
    mat4 MVP;
    vec4 OutputSize;
    vec4 OriginalSize;
    vec4 SourceSize;
} global;

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;

void main()
{
    gl_Position = global.MVP * Position;
    vTexCoord   = TexCoord;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
layout(set = 0, binding = 3) uniform sampler2D OriginalHistory1;
layout(set = 0, binding = 4) uniform sampler2D OriginalHistory2;
layout(set = 0, binding = 5) uniform sampler2D OriginalHistory3;
layout(set = 0, binding = 6) uniform sampler2D OriginalHistory4;
layout(set = 0, binding = 7) uniform sampler2D OriginalHistory5;
layout(set = 0, binding = 8) uniform sampler2D OriginalHistory6;
layout(set = 0, binding = 9) uniform sampler2D OriginalHistory7;

// Frame sampling definitions
#define curr_rgb  texture(Source,           vTexCoord).rgb
#define prev0_rgb texture(OriginalHistory1, vTexCoord).rgb
#define prev1_rgb texture(OriginalHistory2, vTexCoord).rgb
#define prev2_rgb texture(OriginalHistory3, vTexCoord).rgb
#define prev3_rgb texture(OriginalHistory4, vTexCoord).rgb
#define prev4_rgb texture(OriginalHistory5, vTexCoord).rgb
#define prev5_rgb texture(OriginalHistory6, vTexCoord).rgb
#define prev6_rgb texture(OriginalHistory7, vTexCoord).rgb

void main()
{
    // Sample color from the current and previous frames, apply response time modifier
    // Response time effect implmented through an exponential dropoff algorithm
    vec3 input_rgb = curr_rgb;
    input_rgb += (prev0_rgb - input_rgb) * params.response_time;
    input_rgb += (prev1_rgb - input_rgb) * pow(params.response_time, 2.0);
    input_rgb += (prev2_rgb - input_rgb) * pow(params.response_time, 3.0);
    input_rgb += (prev3_rgb - input_rgb) * pow(params.response_time, 4.0);
    input_rgb += (prev4_rgb - input_rgb) * pow(params.response_time, 5.0);
    input_rgb += (prev5_rgb - input_rgb) * pow(params.response_time, 6.0);
    input_rgb += (prev6_rgb - input_rgb) * pow(params.response_time, 7.0);

    FragColor = vec4(input_rgb, 0.0);
}
