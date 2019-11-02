vec3 col = texture(Source, vTexCoord).rgb;
vec3 yiq = rgb2yiq(col);

#if defined(TWO_PHASE)
float chroma_phase = PI * (mod(pix_no.y, 2.0) + global.FrameCount);
#elif defined(THREE_PHASE)
float chroma_phase = 0.6667 * PI * (mod(pix_no.y, 3.0) + global.FrameCount);
#endif

float mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ;

float i_mod = cos(mod_phase);
float q_mod = sin(mod_phase);

yiq.yz *= vec2(i_mod, q_mod); // Modulate.
yiq *= mix_mat; // Cross-talk.
yiq.yz *= vec2(i_mod, q_mod); // Demodulate.
FragColor = vec4(yiq, 1.0);

