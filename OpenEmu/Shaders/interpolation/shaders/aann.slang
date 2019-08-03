#version 450

// AntiAliased Nearest Neighbor
// by jimbo1qaz and wareya
// Licensed MIT

layout(push_constant) uniform Push
{
  float NOGAMMA;
  float MASKING;
  float BILINEAR;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
    mat4 MVP;
    vec4 OutputSize;
    vec4 OriginalSize;
    vec4 SourceSize;
} global;

// set to true to interpolate in sRGB instead of a pseudo-perceptual colorspace
#pragma parameter NOGAMMA "Interpolate in sRGB" 0.0 0.0 1.0 1.0

// set to true to compensate for 8px overscan masking
// Note: overscan compensation slightly alters (extremifies) the pixel aspect ratio of the game if said pixel aspect ratio is not exactly 1:1
#pragma parameter MASKING "8px Overscan Masking" 0.0 0.0 1.0 1.0

// Do bilinear filtering instead of anti-aliased nearest neighbor filtering (used for debugging color)
#pragma parameter BILINEAR "Force Bilinear Filtering" 0.0 0.0 1.0 1.0

// http://i.imgur.com/kzwZkVf.png

#define NOT(fl) (1-fl)
#define YES(fl) fl

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




// http://entropymine.com/imageworsener/srgbformula/
vec3 srgb2linear(vec3 srgb) {
    return vec3(
        srgb.r > 0.0404482362771082 ? pow(srgb.r*0.947867298578199 + 0.052132701421801, 2.4) : srgb.r*0.0773993808049536,
        srgb.g > 0.0404482362771082 ? pow(srgb.g*0.947867298578199 + 0.052132701421801, 2.4) : srgb.g*0.0773993808049536,
        srgb.b > 0.0404482362771082 ? pow(srgb.b*0.947867298578199 + 0.052132701421801, 2.4) : srgb.b*0.0773993808049536 
    );
}

vec3 linear2srgb(vec3 linear) {
    return vec3(
        linear.x > 0.00313066844250063 ? pow(linear.x, 0.416666666666667)*1.055 - 0.055 : linear.x*12.92,
        linear.y > 0.00313066844250063 ? pow(linear.y, 0.416666666666667)*1.055 - 0.055 : linear.y*12.92,
        linear.z > 0.00313066844250063 ? pow(linear.z, 0.416666666666667)*1.055 - 0.055 : linear.z*12.92
    );
}

// https://www.w3.org/Graphics/Color/srgb22
#define RS 0.2126
#define GS 0.7152
#define BS 0.0722

vec3 rgb2vry(vec3 rgb) {
    if (params.NOGAMMA == 1.0)
        return rgb;

    // https://en.wikipedia.org/wiki/Opponent_process
    vec3 linear = srgb2linear(rgb);

    // https://en.wikipedia.org/wiki/Lightness#Relationship_between_lightness.2C_value.2C_and_relative_luminance
    // "scientists eventually converged on a roughly cube-root curve"
    // CIE does the same thing.
    vec3 vry = vec3(
        pow(linear.x*RS + linear.y*GS + linear.z*BS, 0.333333333333333),
        linear.x - linear.y,
        (linear.x + linear.y) * 0.5 - linear.z
    );

    return vry;
}
vec3 vry2rgb(vec3 vry) {
    if (params.NOGAMMA == 1.0)
        return vry;

    // Magic.
    float t = pow(vry.x, 3);
    
    vec3 rgb = vec3(
        t + vry.y*(GS       + BS * 0.5) + vry.z*BS,
        t - vry.y*(RS       + BS * 0.5) + vry.z*BS,
        t + vry.y*(GS * 0.5 - RS * 0.5) - vry.z*(RS+GS)
    );
    
    return linear2srgb(rgb);
}

vec3 vry_interp(vec3 first, vec3 second, float frac) {
    if (params.NOGAMMA == 1.0)
        return first*NOT(frac) + second*YES(frac);
    
    // Because the chroma values were generated on linear light, but the luma must be interpolated in perceptual gamma (3)
    // it can cause out-of-gamut oversaturated values, since the chroma field is not a fixed size as luma values change.
    // To compensate, we can "pull" the chroma interpolation path in the opposite way the luma path is curved.
    float new_luma = first.x*NOT(frac) + second.x*YES(frac);
    float linear_span = pow(second.x, 3) - pow(first.x, 3);
    
    if (linear_span == 0) 
        linear_span = 1;

    float luma_fraction = (pow(new_luma, 3) - pow(first.x, 3)) / linear_span;
    
    return  vec3(new_luma,
                first.y*NOT(luma_fraction) + second.y*YES(luma_fraction),
                first.z*NOT(luma_fraction) + second.z*YES(luma_fraction)
            );
}

vec3 percent(float ssize, float tsize, float coord) {
    if (params.BILINEAR == 1.0)
        tsize = ssize;
    
    float minfull = (coord*tsize - 0.5)/tsize*ssize;
    float maxfull = (coord*tsize + 0.5)/tsize*ssize;

    float realfull = floor(maxfull);

    if (minfull > realfull) {
        return vec3(1, (realfull + 0.5)/ssize, (realfull + 0.5)/ssize);
    }

    return  vec3(
                (maxfull - realfull) / (maxfull - minfull),
                (realfull - 0.5) / ssize,
                (realfull + 0.5) / ssize
            );
}

void main() {
    vec2 viewportSize = global.OutputSize.xy;
    vec2 gameCoord = vTexCoord;
    if (params.MASKING == 1.0) {
        float hscale = viewportSize.x/global.SourceSize.x;
        float vscale = viewportSize.y/global.SourceSize.y;

        viewportSize.x += hscale*16;
        viewportSize.y += vscale*16;

        gameCoord.x = (8 + gameCoord.x*global.SourceSize.x)/(global.SourceSize.x + 16);
        gameCoord.y = (8 + gameCoord.y*global.SourceSize.y)/(global.SourceSize.y + 16);
    }

    vec3 xstuff = percent(global.SourceSize.x, viewportSize.x, gameCoord.x);
    vec3 ystuff = percent(global.SourceSize.y, viewportSize.y, gameCoord.y);

    float xkeep = xstuff[0];
    float ykeep = ystuff[0];

    // get points to interpoflate across in pseudo-perceptual colorspace
    vec3 a = rgb2vry(texture(Source, vec2(xstuff[1], ystuff[1])).rgb);
    vec3 b = rgb2vry(texture(Source, vec2(xstuff[2], ystuff[1])).rgb);
    vec3 c = rgb2vry(texture(Source, vec2(xstuff[1], ystuff[2])).rgb);
    vec3 d = rgb2vry(texture(Source, vec2(xstuff[2], ystuff[2])).rgb);

    // interpolate
    vec3 x1     = vry_interp(a,  b,  xkeep);
    vec3 x2     = vry_interp(c,  d,  xkeep);
    vec3 result = vry_interp(x1, x2, ykeep);

    // convert back to sRGB and return
    FragColor = vec4(vry2rgb(result), 1);
}
