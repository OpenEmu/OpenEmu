#version 450

#include "geom-deluxe-params.inc"

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
layout(set = 0, binding = 3) uniform sampler2D PassFeedback1;

void main()
{
   vec4 screen = texture(Source, vTexCoord);
   vec4 phosphor = texture(PassFeedback1, vTexCoord);
   
   vec3 cscrn = pow(screen.rgb, vec3(gamma));
   vec3 cphos = pow(phosphor.rgb, vec3(gamma));
   
   // encode the upper 2 bits of the time elapsed in the lower 2 bits of b
   float t = 255.0*phosphor.a + fract(phosphor.b*255.0/4.0)*1024.0;

   cphos *= vec3( phosphor_amplitude * pow(t,-phosphor_power) );
   
   vec3 col = pow(cscrn + cphos, vec3(1.0/gamma));
   
   FragColor = vec4(col, 1.0);
}