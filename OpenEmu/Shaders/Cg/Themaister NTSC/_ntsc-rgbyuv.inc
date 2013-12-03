const float3x3 yiq2rgb_mat = float3x3(
   1.0, 0.956, 0.6210,
   1.0, -0.2720, -0.6474,
   1.0, -1.1060, 1.7046);

float3 yiq2rgb(float3 yiq)
{
   return mul(yiq2rgb_mat, yiq);
}

const float3x3 yiq_mat = float3x3(
      0.2989, 0.5870, 0.1140,
      0.5959, -0.2744, -0.3216,
      0.2115, -0.5229, 0.3114
);

float3 rgb2yiq(float3 col)
{
   return mul(yiq_mat, col);
}


