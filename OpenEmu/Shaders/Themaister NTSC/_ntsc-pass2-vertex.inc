struct data
{
   float2 tex;
};

struct input
{
   float2 video_size;
   float2 texture_size;
   float2 output_size;
   float frame_count;
};

void main_vertex
(
   float4 position : POSITION,
   out float4 oPosition : POSITION,
   uniform float4x4 modelViewProj,
   float2 tex : TEXCOORD,
   uniform input IN,
   out data oData
)
{
   oPosition = mul(modelViewProj, position);
   oData.tex = tex - float2(0.5 / IN.texture_size.x, 0.0); // Compensate for decimate-by-2.
}


