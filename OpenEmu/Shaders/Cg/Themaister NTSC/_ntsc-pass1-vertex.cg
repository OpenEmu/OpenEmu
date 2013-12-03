struct data
{
   float2 tex;
   float2 pix_no;
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
   oData.tex = tex;
   oData.pix_no = tex * IN.texture_size * (IN.output_size / IN.video_size);
}


