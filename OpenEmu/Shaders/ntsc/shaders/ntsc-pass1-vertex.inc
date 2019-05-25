#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 pix_no;

void main()
{
   gl_Position = global.MVP * Position;
   vTexCoord = TexCoord;
   pix_no = TexCoord * global.SourceSize.xy * (global.OutputSize.xy / global.SourceSize.xy);
}
