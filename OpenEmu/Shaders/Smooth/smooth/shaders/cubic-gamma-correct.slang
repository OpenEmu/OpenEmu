#version 450

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
   vTexCoord = TexCoord;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void weights(out vec4 x, out vec4 y, vec2 t)
{
   vec2 t2 = t * t;
   vec2 t3 = t2 * t;

   vec4 xs = vec4(1.0, t.x, t2.x, t3.x);
   vec4 ys = vec4(1.0, t.y, t2.y, t3.y);

   const vec4 p0 = vec4(+0.0, -0.5, +1.0, -0.5);
   const vec4 p1 = vec4(+1.0,  0.0, -2.5, +1.5);
   const vec4 p2 = vec4(+0.0, +0.5, +2.0, -1.5);
   const vec4 p3 = vec4(+0.0,  0.0, -0.5, +0.5);

   x = vec4(dot(xs, p0), dot(xs, p1), dot(xs, p2), dot(xs, p3));
   y = vec4(dot(ys, p0), dot(ys, p1), dot(ys, p2), dot(ys, p3));
}

void main()
{
   vec2 uv = vTexCoord * global.SourceSize.xy - 0.5;
   vec2 texel = floor(uv);
   vec2 tex = (texel + 0.5) * global.SourceSize.zw;
   vec2 phase = uv - texel;

#define TEX(x, y) textureLodOffset(Source, tex, 0.0, ivec2(x, y)).rgb

   vec4 x;
   vec4 y;
   weights(x, y, phase);

   vec3 color;
   vec4 row = x * y.x;
   color  = TEX(-1, -1) * row.x;
   color += TEX(+0, -1) * row.y;
   color += TEX(+1, -1) * row.z;
   color += TEX(+2, -1) * row.w;

   row = x * y.y;
   color += TEX(-1, +0) * row.x;
   color += TEX(+0, +0) * row.y;
   color += TEX(+1, +0) * row.z;
   color += TEX(+2, +0) * row.w;

   row = x * y.z;
   color += TEX(-1, +1) * row.x;
   color += TEX(+0, +1) * row.y;
   color += TEX(+1, +1) * row.z;
   color += TEX(+2, +1) * row.w;

   row = x * y.w;
   color += TEX(-1, +2) * row.x;
   color += TEX(+0, +2) * row.y;
   color += TEX(+1, +2) * row.z;
   color += TEX(+2, +2) * row.w;

   color = sqrt(clamp(color, vec3(0.0), vec3(1.0)));
   FragColor = vec4(color, 1.0);
}
