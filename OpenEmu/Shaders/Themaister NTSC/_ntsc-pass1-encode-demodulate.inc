float3 col = tex2D(s0, vertex.tex).rgb;
float3 yiq = rgb2yiq(col);

#if defined(TWO_PHASE)
float chroma_phase = PI * (fmod(vertex.pix_no.y, 2.0) + IN.frame_count);
#elif defined(THREE_PHASE)
float chroma_phase = 0.6667 * PI * (fmod(vertex.pix_no.y, 3.0) + IN.frame_count);
#endif

float mod_phase = chroma_phase + vertex.pix_no.x * CHROMA_MOD_FREQ;

float i_mod = cos(mod_phase);
float q_mod = sin(mod_phase);

yiq.yz *= float2(i_mod, q_mod); // Modulate.
yiq = mul(mix_mat, yiq); // Cross-talk.
yiq.yz *= float2(i_mod, q_mod); // Demodulate.
return float4(yiq, 1.0);

