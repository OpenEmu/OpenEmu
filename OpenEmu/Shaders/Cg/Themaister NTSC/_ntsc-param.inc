#define PI 3.14159265

#if defined(TWO_PHASE)
#define CHROMA_MOD_FREQ (4.0 * PI / 15.0)
#elif defined(THREE_PHASE)
#define CHROMA_MOD_FREQ (PI / 3.0)
#endif

#if defined(COMPOSITE)
#define SATURATION 1.0
#define BRIGHTNESS 1.0
#define ARTIFACTING 1.0
#define FRINGING 1.0
#elif defined(SVIDEO)
#define SATURATION 1.0
#define BRIGHTNESS 1.0
#define ARTIFACTING 0.0
#define FRINGING 0.0
#endif

#if defined(COMPOSITE) || defined(SVIDEO)
const float3x3 mix_mat = float3x3(
      BRIGHTNESS, ARTIFACTING, ARTIFACTING,
      FRINGING, 2.0 * SATURATION, 0.0,
      FRINGING, 0.0, 2.0 * SATURATION 
);
#endif

