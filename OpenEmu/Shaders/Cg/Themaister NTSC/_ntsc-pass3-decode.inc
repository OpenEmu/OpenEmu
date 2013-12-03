float one_x = 1.0 / IN.texture_size.x;
float3 signal = float3(0.0);
for (int i = 0; i < TAPS; i++)
{
   float offset = float(i);

   float3 sums = fetch_offset(offset - float(TAPS), one_x) +
      fetch_offset(float(TAPS) - offset, one_x);

   signal += sums * float3(luma_filter[i], chroma_filter[i], chroma_filter[i]);
}
signal += tex2D(s0, vertex.tex).xyz *
   float3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);

