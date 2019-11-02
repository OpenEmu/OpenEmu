#version 450

layout(push_constant) uniform Push
{
	vec4 OutputSize;
	vec4 OriginalSize;
	vec4 SourceSize;
	uint FrameCount;
	float CRTgamma;
	float monitorgamma;
	float d;
	float R;
	float cornersize;
	float cornersmooth;
	float x_tilt;
	float y_tilt;
	float overscan_x;
	float overscan_y;
	float DOTMASK;
	float SHARPER;
	float scanline_weight;
	float CURVATURE;
} registers;

layout(std140, set = 0, binding = 0) uniform UBO
{
    mat4 MVP;
	vec4 OutputSize;
} global;

#pragma parameter CRTgamma "CRTGeom Target Gamma" 2.4 0.1 5.0 0.1
#pragma parameter monitorgamma "CRTGeom Monitor Gamma" 2.2 0.1 5.0 0.1
#pragma parameter d "CRTGeom Distance" 1.5 0.1 3.0 0.1
#pragma parameter CURVATURE "CRTGeom Curvature Toggle" 1.0 0.0 1.0 1.0
#pragma parameter R "CRTGeom Curvature Radius" 2.0 0.1 10.0 0.1
#pragma parameter cornersize "CRTGeom Corner Size" 0.03 0.001 1.0 0.005
#pragma parameter cornersmooth "CRTGeom Corner Smoothness" 1000.0 80.0 2000.0 100.0
#pragma parameter x_tilt "CRTGeom Horizontal Tilt" 0.0 -0.5 0.5 0.05
#pragma parameter y_tilt "CRTGeom Vertical Tilt" 0.0 -0.5 0.5 0.05
#pragma parameter overscan_x "CRTGeom Horiz. Overscan %" 100.0 -125.0 125.0 1.0
#pragma parameter overscan_y "CRTGeom Vert. Overscan %" 100.0 -125.0 125.0 1.0
#pragma parameter DOTMASK "CRTGeom Dot Mask Toggle" 0.3 0.0 0.3 0.3
#pragma parameter SHARPER "CRTGeom Sharpness" 1.0 1.0 3.0 1.0
#pragma parameter scanline_weight "CRTGeom Scanline Weight" 0.3 0.1 0.5 0.05

/*
    CRT-interlaced

    Copyright (C) 2010-2012 cgwg, Themaister and DOLLS

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    (cgwg gave their consent to have the original version of this shader
    distributed under the GPL in this message:

    http://board.byuu.org/viewtopic.php?p=26075#p26075

    "Feel free to distribute my shaders under the GPL. After all, the
    barrel distortion code was taken from the Curvature shader, which is
    under the GPL."
    )
    This shader variant is pre-configured with screen curvature
*/

// Comment the next line to disable interpolation in linear gamma (and
// gain speed).
#define LINEAR_PROCESSING

// Enable 3x oversampling of the beam profile; improves moire effect caused by scanlines+curvature
#define OVERSAMPLE

// Use the older, purely gaussian beam profile; uncomment for speed
#define USEGAUSSIAN
  
// Use interlacing detection; may interfere with other shaders if combined
#define INTERLACED

// Macros.
#define FIX(c) max(abs(c), 1e-5);
#define PI 3.141592653589

#ifdef LINEAR_PROCESSING
#       define TEX2D(c) pow(texture(Source, (c)), vec4(registers.CRTgamma))
#else
#       define TEX2D(c) texture(Source, (c))
#endif

// aspect ratio
vec2 aspect     = vec2(1.0,  0.75);
vec2 overscan   = vec2(1.01, 1.01);

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 sinangle;
layout(location = 2) out vec2 cosangle;
layout(location = 3) out vec3 stretch;
layout(location = 4) out vec2 ilfac;
layout(location = 5) out vec2 one;
layout(location = 6) out float mod_factor;
layout(location = 7) out vec2 TextureSize;

float intersect(vec2 xy)
{
    float A = dot(xy,xy) + registers.d*registers.d;
    float B = 2.0*(registers.R*(dot(xy,sinangle)-registers.d*cosangle.x*cosangle.y)-registers.d*registers.d);
    float C = registers.d*registers.d + 2.0*registers.R*registers.d*cosangle.x*cosangle.y;
    
    return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);
}

vec2 bkwtrans(vec2 xy)
{
    float c     = intersect(xy);
    vec2 point  = (vec2(c, c)*xy - vec2(-registers.R, -registers.R)*sinangle) / vec2(registers.R, registers.R);
    vec2 poc    = point/cosangle;
    
    vec2 tang   = sinangle/cosangle;
    float A     = dot(tang, tang) + 1.0;
    float B     = -2.0*dot(poc, tang);
    float C     = dot(poc, poc) - 1.0;
    
    float a     = (-B + sqrt(B*B - 4.0*A*C))/(2.0*A);
    vec2 uv     = (point - a*sinangle)/cosangle;
    float r     = FIX(registers.R*acos(a));
    
    return uv*r/sin(r/registers.R);
}

vec2 fwtrans(vec2 uv)
{
    float r = FIX(sqrt(dot(uv,uv)));
    uv *= sin(r/registers.R)/r;
    float x = 1.0-cos(r/registers.R);
    float D = registers.d/registers.R + x*cosangle.x*cosangle.y+dot(uv,sinangle);
    
    return registers.d*(uv*cosangle-x*sinangle)/D;
}

vec3 maxscale()
{
    vec2 c  = bkwtrans(-registers.R * sinangle / (1.0 + registers.R/registers.d*cosangle.x*cosangle.y));
    vec2 a  = vec2(0.5,0.5)*aspect;
    
    vec2 lo = vec2(fwtrans(vec2(-a.x,  c.y)).x,
                   fwtrans(vec2( c.x, -a.y)).y)/aspect;

    vec2 hi = vec2(fwtrans(vec2(+a.x,  c.y)).x,
                   fwtrans(vec2( c.x, +a.y)).y)/aspect;
    
    return vec3((hi+lo)*aspect*0.5,max(hi.x-lo.x,hi.y-lo.y));
}

// Calculate the influence of a scanline on the current pixel.
//
// 'distance' is the distance in texture coordinates from the current
// pixel to the scanline in question.
// 'color' is the colour of the scanline at the horizontal location of
// the current pixel.
vec4 scanlineWeights(float distance, vec4 color)
{
    // "wid" controls the width of the scanline beam, for each RGB
    // channel The "weights" lines basically specify the formula
    // that gives you the profile of the beam, i.e. the intensity as
    // a function of distance from the vertical center of the
    // scanline. In this case, it is gaussian if width=2, and
    // becomes nongaussian for larger widths. Ideally this should
    // be normalized so that the integral across the beam is
    // independent of its width. That is, for a narrower beam
    // "weights" should have a higher peak at the center of the
    // scanline than for a wider beam.
    #ifdef USEGAUSSIAN
        vec4 wid = 0.3 + 0.1 * pow(color, vec4(3.0));
        vec4 weights = vec4(distance / wid);
        
        return 0.4 * exp(-weights * weights) / wid;
    #else
        vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));
        vec4 weights = vec4(distance / registers.scanline_weight);
        
        return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
    #endif
}
    
void main()
{
    gl_Position = global.MVP * Position;
    vTexCoord = TexCoord * vec2(1.00001);

    // Precalculate a bunch of useful values we'll need in the fragment
    // shader.
    sinangle    = sin(vec2(registers.x_tilt, registers.y_tilt));
    cosangle    = cos(vec2(registers.x_tilt, registers.y_tilt));
    stretch     = maxscale();
    TextureSize = vec2(registers.SHARPER * registers.SourceSize.x, registers.SourceSize.y);
    
#ifdef INTERLACED
    ilfac = vec2(1.0, clamp(floor(registers.SourceSize.y/200.0),  1.0, 2.0));
#else
    ilfac = vec2(1.0, clamp(floor(registers.SourceSize.y/1000.0), 1.0, 2.0));
#endif

    // The size of one texel, in texture-coordinates.
    one = ilfac / TextureSize;

    // Resulting X pixel-coordinate of the pixel we're drawing.
    mod_factor = vTexCoord.x * registers.SourceSize.x * registers.OutputSize.x / registers.SourceSize.x;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 sinangle;
layout(location = 2) in vec2 cosangle;
layout(location = 3) in vec3 stretch;
layout(location = 4) in vec2 ilfac;
layout(location = 5) in vec2 one;
layout(location = 6) in float mod_factor;
layout(location = 7) in vec2 TextureSize;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

float intersect(vec2 xy)
{
    float A = dot(xy,xy) + registers.d*registers.d;
    float B = 2.0*(registers.R*(dot(xy,sinangle) - registers.d*cosangle.x*cosangle.y) - registers.d*registers.d);
    float C = registers.d*registers.d + 2.0*registers.R*registers.d*cosangle.x*cosangle.y;
    
    return (-B-sqrt(B*B - 4.0*A*C))/(2.0*A);
}

vec2 bkwtrans(vec2 xy)
{
    float c     = intersect(xy);
    vec2 point  = (vec2(c, c)*xy - vec2(-registers.R, -registers.R)*sinangle) / vec2(registers.R, registers.R);
    vec2 poc    = point/cosangle;
    vec2 tang   = sinangle/cosangle;

    float A     = dot(tang, tang) + 1.0;
    float B     = -2.0*dot(poc, tang);
    float C     = dot(poc, poc) - 1.0;

    float a     = (-B + sqrt(B*B - 4.0*A*C)) / (2.0*A);
    vec2 uv     = (point - a*sinangle) / cosangle;
    float r     = FIX(registers.R*acos(a));
    
    return uv*r/sin(r/registers.R);
}

vec2 fwtrans(vec2 uv)
{
    float r = FIX(sqrt(dot(uv, uv)));
    uv *= sin(r/registers.R)/r;
    float x = 1.0 - cos(r/registers.R);
    float D = registers.d/registers.R + x*cosangle.x*cosangle.y + dot(uv,sinangle);

    return registers.d*(uv*cosangle - x*sinangle)/D;
}

vec3 maxscale()
{
    vec2 c = bkwtrans(-registers.R * sinangle / (1.0 + registers.R/registers.d*cosangle.x*cosangle.y));
    vec2 a = vec2(0.5, 0.5)*aspect;

    vec2 lo = vec2(fwtrans(vec2(-a.x,  c.y)).x,
                   fwtrans(vec2( c.x, -a.y)).y)/aspect;
    vec2 hi = vec2(fwtrans(vec2(+a.x,  c.y)).x,
                   fwtrans(vec2( c.x, +a.y)).y)/aspect;

    return vec3((hi+lo)*aspect*0.5,max(hi.x-lo.x, hi.y-lo.y));
}

// Calculate the influence of a scanline on the current pixel.
//
// 'distance' is the distance in texture coordinates from the current
// pixel to the scanline in question.
// 'color' is the colour of the scanline at the horizontal location of
// the current pixel.
vec4 scanlineWeights(float distance, vec4 color)
{
    // "wid" controls the width of the scanline beam, for each RGB
    // channel The "weights" lines basically specify the formula
    // that gives you the profile of the beam, i.e. the intensity as
    // a function of distance from the vertical center of the
    // scanline. In this case, it is gaussian if width=2, and
    // becomes nongaussian for larger widths. Ideally this should
    // be normalized so that the integral across the beam is
    // independent of its width. That is, for a narrower beam
    // "weights" should have a higher peak at the center of the
    // scanline than for a wider beam.
#ifdef USEGAUSSIAN
    vec4 wid = 0.3 + 0.1 * pow(color, vec4(3.0));
    vec4 weights = vec4(distance / wid);
    return 0.4 * exp(-weights * weights) / wid;
#else
    vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));
    vec4 weights = vec4(distance / registers.scanline_weight);
    return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);
#endif
}
    
vec2 transform(vec2 coord)
{
    coord = (coord - vec2(0.5, 0.5))*aspect*stretch.z + stretch.xy;
    
    return (bkwtrans(coord) /
        vec2(registers.overscan_x / 100.0, registers.overscan_y / 100.0)/aspect + vec2(0.5, 0.5));
}
    
float corner(vec2 coord)
{
    coord = (coord - vec2(0.5)) * vec2(registers.overscan_x / 100.0, registers.overscan_y / 100.0) + vec2(0.5, 0.5);
    coord = min(coord, vec2(1.0) - coord) * aspect;
    vec2 cdist = vec2(registers.cornersize);
    coord = (cdist - min(coord, cdist));
    float dist = sqrt(dot(coord, coord));
    
    return clamp((cdist.x - dist)*registers.cornersmooth, 0.0, 1.0);
}
    
void main()
{
    // Here's a helpful diagram to keep in mind while trying to
    // understand the code:
    //
    //  |      |      |      |      |
    // -------------------------------
    //  |      |      |      |      |
    //  |  01  |  11  |  21  |  31  | <-- current scanline
    //  |      | @    |      |      |
    // -------------------------------
    //  |      |      |      |      |
    //  |  02  |  12  |  22  |  32  | <-- next scanline
    //  |      |      |      |      |
    // -------------------------------
    //  |      |      |      |      |
    //
    // Each character-cell represents a pixel on the output
    // surface, "@" represents the current pixel (always somewhere
    // in the bottom half of the current scan-line, or the top-half
    // of the next scanline). The grid of lines represents the
    // edges of the texels of the underlying texture.

    // Texture coordinates of the texel containing the active pixel.
	vec2 xy;
	if (registers.CURVATURE > 0.5)
    xy = transform(vTexCoord);
	else
	xy = vTexCoord;

    float cval = corner(xy);

    // Of all the pixels that are mapped onto the texel we are
    // currently rendering, which pixel are we currently rendering?
#ifdef INTERLACED
    vec2 ilvec = vec2(0.0, ilfac.y > 1.5 ? mod(float(registers.FrameCount), 2.0) : 0.0);
#else
    vec2 ilvec = vec2(0.0, ilfac.y);
#endif

    vec2 ratio_scale = (xy * TextureSize - vec2(0.5, 0.5) + ilvec) / ilfac;
    vec2 uv_ratio = fract(ratio_scale);

    // Snap to the center of the underlying texel.
    xy = (floor(ratio_scale)*ilfac + vec2(0.5, 0.5) - ilvec) / TextureSize;

    // Calculate Lanczos scaling coefficients describing the effect
    // of various neighbour texels in a scanline on the current
    // pixel.
    vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);

    // Prevent division by zero.
    coeffs = FIX(coeffs);

    // Lanczos2 kernel.
    coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);

    // Normalize.
    coeffs /= dot(coeffs, vec4(1.0));

    // Calculate the effective colour of the current and next
    // scanlines at the horizontal location of the current pixel,
    // using the Lanczos coefficients above.
    vec4 col = clamp(
        mat4(
            TEX2D(xy + vec2(-one.x, 0.0)),
            TEX2D(xy),
            TEX2D(xy + vec2(one.x, 0.0)),
            TEX2D(xy + vec2(2.0 * one.x, 0.0))
        ) * coeffs,
        0.0, 1.0
    );
    vec4 col2 = clamp(
        mat4(
            TEX2D(xy + vec2(-one.x, one.y)),
            TEX2D(xy + vec2(0.0, one.y)),
            TEX2D(xy + one),
            TEX2D(xy + vec2(2.0 * one.x, one.y))
        ) * coeffs,
        0.0, 1.0
    );

#ifndef LINEAR_PROCESSING
    col  = pow(col , vec4(registers.CRTgamma));
    col2 = pow(col2, vec4(registers.CRTgamma));
#endif

    // Calculate the influence of the current and next scanlines on
    // the current pixel.
    vec4 weights  = scanlineWeights(uv_ratio.y, col);
    vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);

#ifdef OVERSAMPLE
    float filter_ = fwidth(ratio_scale.y);
    uv_ratio.y    = uv_ratio.y + 1.0/3.0*filter_;
    weights       = (weights  + scanlineWeights(uv_ratio.y, col))/3.0;
    weights2      = (weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2))/3.0;
    uv_ratio.y    = uv_ratio.y - 2.0/3.0*filter_;
    weights       = weights  + scanlineWeights(abs(uv_ratio.y), col)/3.0;
    weights2      = weights2 + scanlineWeights(abs(1.0 - uv_ratio.y), col2)/3.0;
#endif

    vec3 mul_res  = (col * weights + col2 * weights2).rgb * vec3(cval);

    // dot-mask emulation:
    // Output pixels are alternately tinted green and magenta.
    vec3 dotMaskWeights = mix(
        vec3(1.0, 1.0 - registers.DOTMASK, 1.0),
        vec3(1.0 - registers.DOTMASK, 1.0, 1.0 - registers.DOTMASK),
        floor(mod(mod_factor, 2.0))
    );
      
    mul_res *= dotMaskWeights;

    // Convert the image gamma for display on our output device.
    mul_res = pow(mul_res, vec3(1.0 / registers.monitorgamma));

    FragColor = vec4(mul_res, 1.0);
}
