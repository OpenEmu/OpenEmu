float one_x = 1.0 / global.SourceSize.x;
vec3 signal = vec3(0.0);
for (int i = 0; i < TAPS; i++)
{
   float offset = float(i);

   vec3 sums = fetch_offset(offset - float(TAPS), one_x) +
      fetch_offset(float(TAPS) - offset, one_x);

   signal += sums * vec3(luma_filter[i], chroma_filter[i], chroma_filter[i]);
}
signal += texture(Source, vTexCoord).xyz *
   vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);
