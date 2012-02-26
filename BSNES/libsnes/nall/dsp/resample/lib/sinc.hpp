// If these types are changed to anything other than "float", you should comment out the SSE detection directives below
// so that the SSE code is not used.

typedef float resample_coeff_t;	// note: sizeof(resample_coeff_t) must be == to a power of 2, and not larger than 16
typedef float resample_samp_t;


// ...but don't comment this single RESAMPLE_SSEREGPARM define out when disabling SSE.
#define RESAMPLE_SSEREGPARM	

#if defined(__SSE__)
  #define SINCRESAMPLE_USE_SSE 1
  #ifndef __x86_64__
    #undef RESAMPLE_SSEREGPARM
    #define RESAMPLE_SSEREGPARM __attribute__((sseregparm))
  #endif
#else
  // TODO: altivec here
#endif

namespace ResampleUtility
{
 inline void kaiser_window(double* io, int count, double beta);
 inline void gen_sinc(double* out, int size, double cutoff, double kaiser);
 inline void gen_sinc_os(double* out, int size, double cutoff, double kaiser);
 inline void normalize(double* io, int size, double gain = 1.0);

 inline void* make_aligned(void* ptr, unsigned boundary);	// boundary must be a power of 2
}

class SincResampleHR
{
 private:

 inline void Init(unsigned ratio_arg, double desired_bandwidth, double beta, double d);

 inline void write(resample_samp_t sample) RESAMPLE_SSEREGPARM;
 inline resample_samp_t read(void) RESAMPLE_SSEREGPARM;
 inline bool output_avail(void);

 private:

 inline resample_samp_t mac(const resample_samp_t *wave, const resample_coeff_t *coeff, unsigned count);

 unsigned ratio;
 unsigned num_convolutions;

 resample_coeff_t *coeffs;
 std::vector<unsigned char> coeffs_mem;

 // second half of ringbuffer should be copy of first half.
 resample_samp_t *rb;
 std::vector<unsigned char> rb_mem;

 signed rb_readpos;
 signed rb_writepos;
 signed rb_in;
 signed rb_eff_size;

 friend class SincResample;
};

class SincResample
{
 public:

 enum
 {
  QUALITY_LOW = 0,
  QUALITY_MEDIUM = 2,
  QUALITY_HIGH = 4
 };

 inline SincResample(double input_rate, double output_rate, double desired_bandwidth, unsigned quality = QUALITY_HIGH);

 inline void write(resample_samp_t sample) RESAMPLE_SSEREGPARM;
 inline resample_samp_t read(void) RESAMPLE_SSEREGPARM;
 inline bool output_avail(void);

 private:

 inline void Init(double input_rate, double output_rate, double desired_bandwidth, double beta, double d, unsigned pn_nume, unsigned phases_min);

 inline resample_samp_t mac(const resample_samp_t *wave, const resample_coeff_t *coeffs_a, const resample_coeff_t *coeffs_b, const double ffract, unsigned count) RESAMPLE_SSEREGPARM;

 unsigned num_convolutions;
 unsigned num_phases;

 unsigned step_int;
 double step_fract;

 double input_pos_fract;


 std::vector<resample_coeff_t *> coeffs;	// Pointers into coeff_mem.
 std::vector<unsigned char> coeff_mem;


 std::vector<resample_samp_t> rb;	// second half should be copy of first half.
 signed rb_readpos;
 signed rb_writepos;
 signed rb_in;

 bool hr_used;
 SincResampleHR hr;
};


//
// Code:
//
//#include "resample.hpp"

#if 0
namespace bit
{
    inline unsigned round(unsigned x) {
      if((x & (x - 1)) == 0) return x;
      while(x & (x - 1)) x &= x - 1;
      return x << 1;
    }
}
#endif

void SincResampleHR::Init(unsigned ratio_arg, double desired_bandwidth, double beta, double d)
{
 const unsigned align_boundary = 16;
 std::vector<double> coeffs_tmp;
 double cutoff;	// 1.0 = f/2

 ratio = ratio_arg;

 //num_convolutions = ((unsigned)ceil(d / ((1.0 - desired_bandwidth) / ratio)) + 1) &~ 1;	// round up to be even
 num_convolutions = ((unsigned)ceil(d / ((1.0 - desired_bandwidth) / ratio)) | 1);

 cutoff = (1.0 / ratio) - (d / num_convolutions);

//printf("%d %d %.20f\n", ratio, num_convolutions, cutoff);
 assert(num_convolutions > ratio);


 // Generate windowed sinc of POWER
 coeffs_tmp.resize(num_convolutions);
 //ResampleUtility::gen_sinc(&coeffs_tmp[0], num_convolutions, cutoff, beta);
 ResampleUtility::gen_sinc_os(&coeffs_tmp[0], num_convolutions, cutoff, beta);
 ResampleUtility::normalize(&coeffs_tmp[0], num_convolutions);

 // Copy from coeffs_tmp to coeffs~
 // We multiply many coefficients at a time in the mac loop, so make sure the last few that don't really
 // exist are allocated, zero'd mem.

 coeffs_mem.resize(((num_convolutions + 7) &~ 7) * sizeof(resample_coeff_t) + (align_boundary - 1));
 coeffs = (resample_coeff_t *)ResampleUtility::make_aligned(&coeffs_mem[0], align_boundary);


 for(unsigned i = 0; i < num_convolutions; i++)
  coeffs[i] = coeffs_tmp[i];

 rb_eff_size = nall::bit::round(num_convolutions * 2) >> 1;
 rb_readpos = 0;
 rb_writepos = 0;
 rb_in = 0;

 rb_mem.resize(rb_eff_size * 2 * sizeof(resample_samp_t) + (align_boundary - 1));
 rb = (resample_samp_t *)ResampleUtility::make_aligned(&rb_mem[0], align_boundary);
}


inline bool SincResampleHR::output_avail(void)
{
 return(rb_in >= (signed)num_convolutions);
}

inline void SincResampleHR::write(resample_samp_t sample)
{
 assert(!output_avail());

 rb[rb_writepos] = sample;
 rb[rb_writepos + rb_eff_size] = sample;
 rb_writepos = (rb_writepos + 1) & (rb_eff_size - 1);
 rb_in++;
}

resample_samp_t SincResampleHR::mac(const resample_samp_t *wave, const resample_coeff_t *coeff, unsigned count)
{
#if SINCRESAMPLE_USE_SSE
 __m128 accum_veca[2] = { _mm_set1_ps(0), _mm_set1_ps(0) };
 
 resample_samp_t accum;

 for(unsigned c = 0; c < count; c += 8)
 {
  for(unsigned i = 0; i < 2; i++)
  {
   __m128 co[2];
   __m128 w[2];

   co[i] = _mm_load_ps(&coeff[c + i * 4]);
   w[i] = _mm_load_ps(&wave[c + i * 4]);

   w[i] = _mm_mul_ps(w[i], co[i]);

   accum_veca[i] = _mm_add_ps(w[i], accum_veca[i]);
  }
 }

 __m128 accum_vec = _mm_add_ps(accum_veca[0], accum_veca[1]); //_mm_add_ps(_mm_add_ps(accum_veca[0], accum_veca[1]), _mm_add_ps(accum_veca[2], accum_veca[3]));

 accum_vec = _mm_add_ps(accum_vec, _mm_shuffle_ps(accum_vec, accum_vec, (3 << 0) | (2 << 2) | (1 << 4) | (0 << 6)));
 accum_vec = _mm_add_ps(accum_vec, _mm_shuffle_ps(accum_vec, accum_vec, (1 << 0) | (0 << 2) | (1 << 4) | (0 << 6)));

 _mm_store_ss(&accum, accum_vec);

 return accum;
#else
 resample_samp_t accum[4] = { 0, 0, 0, 0 };

 for(unsigned c = 0; c < count; c+= 4)
 {
  accum[0] += wave[c + 0] * coeff[c + 0];
  accum[1] += wave[c + 1] * coeff[c + 1];
  accum[2] += wave[c + 2] * coeff[c + 2];
  accum[3] += wave[c + 3] * coeff[c + 3];
 }

 return (accum[0] + accum[1]) + (accum[2] + accum[3]);	// don't mess with parentheses(assuming compiler doesn't already, which it may...

#endif
}


resample_samp_t SincResampleHR::read(void)
{
 assert(output_avail());
 resample_samp_t ret;

 ret = mac(&rb[rb_readpos], &coeffs[0], num_convolutions);

 rb_readpos = (rb_readpos + ratio) & (rb_eff_size - 1);
 rb_in -= ratio;

 return ret;
}


SincResample::SincResample(double input_rate, double output_rate, double desired_bandwidth, unsigned quality)
{
 const struct
 {
  double beta;
  double d;
  unsigned pn_nume;
  unsigned phases_min;
 } qtab[5] =
 {
  { 5.658, 3.62, 4096, 4 },
  { 6.764, 4.32, 8192, 4 },
  { 7.865, 5.0, 16384, 8 },
  { 8.960, 5.7, 32768, 16 },
  { 10.056, 6.4, 65536, 32 }
 };

 // Sanity checks
 assert(ceil(input_rate) > 0);
 assert(ceil(output_rate) > 0);
 assert(ceil(input_rate / output_rate) <= 1024);
 assert(ceil(output_rate / input_rate) <= 1024);

 // The simplistic number-of-phases calculation code doesn't work well enough for when desired_bandwidth is close to 1.0 and when
 // upsampling.
 assert(desired_bandwidth >= 0.25 && desired_bandwidth < 0.96);
 assert(quality >= 0 && quality <= 4);

 hr_used = false;

#if 1
 // Round down to the nearest multiple of 4(so wave buffer remains aligned)
 // It also adjusts the effective intermediate sampling rate up slightly, so that the upper frequencies below f/2
 // aren't overly attenuated so much.  In the future, we might want to do an FFT or something to choose the intermediate rate more accurately
 // to virtually eliminate over-attenuation.
 unsigned ioratio_rd = (unsigned)floor(input_rate / (output_rate * (1.0 + (1.0 - desired_bandwidth) / 2) )) & ~3;

 if(ioratio_rd >= 8)
 {
  hr.Init(ioratio_rd, desired_bandwidth, qtab[quality].beta, qtab[quality].d); //10.056, 6.4); 
  hr_used = true;

  input_rate /= ioratio_rd;
 }
#endif

 Init(input_rate, output_rate, desired_bandwidth, qtab[quality].beta, qtab[quality].d, qtab[quality].pn_nume, qtab[quality].phases_min);
}

void SincResample::Init(double input_rate, double output_rate, double desired_bandwidth, double beta, double d, unsigned pn_nume, unsigned phases_min)
{
 const unsigned max_mult_atatime = 8;	// multiply "granularity".  must be power of 2.
 const unsigned max_mult_minus1 = (max_mult_atatime - 1);
 const unsigned conv_alignment_bytes = 16;	// must be power of 2
 const double input_to_output_ratio = input_rate / output_rate;
 const double output_to_input_ratio = output_rate / input_rate;
 double cutoff;		// 1.0 = input_rate / 2
 std::vector<double> coeff_init_buffer;

 // Round up num_convolutions to be even.
 if(output_rate > input_rate)
  num_convolutions = ((unsigned)ceil(d / (1.0 - desired_bandwidth)) + 1) & ~1;
 else
  num_convolutions = ((unsigned)ceil(d / (output_to_input_ratio * (1.0 - desired_bandwidth))) + 1) & ~1;

 if(output_rate > input_rate)	// Upsampling
  cutoff = desired_bandwidth;
 else	// Downsampling
  cutoff = output_to_input_ratio * desired_bandwidth;

 // Round up to be even.
 num_phases = (std::max<unsigned>(pn_nume / num_convolutions, phases_min) + 1) &~1;

 // Adjust cutoff to account for the multiple phases.
 cutoff = cutoff / num_phases;

 assert((num_convolutions & 1) == 0);
 assert((num_phases & 1) == 0);

// fprintf(stderr, "num_convolutions=%u, num_phases=%u, total expected coeff byte size=%lu\n", num_convolutions, num_phases,
//        (long)((num_phases + 2) * ((num_convolutions + max_mult_minus1) & ~max_mult_minus1) * sizeof(float) + conv_alignment_bytes));

 coeff_init_buffer.resize(num_phases * num_convolutions);

 coeffs.resize(num_phases + 1 + 1);

 coeff_mem.resize((num_phases + 1 + 1) * ((num_convolutions + max_mult_minus1) &~ max_mult_minus1) * sizeof(resample_coeff_t) + conv_alignment_bytes);

 // Assign aligned pointers into coeff_mem
 {
  resample_coeff_t *base_ptr = (resample_coeff_t *)ResampleUtility::make_aligned(&coeff_mem[0], conv_alignment_bytes);

  for(unsigned phase = 0; phase < (num_phases + 1 + 1); phase++)
  {
   coeffs[phase] = base_ptr + (((num_convolutions + max_mult_minus1) & ~max_mult_minus1) * phase);
  }
 }

 ResampleUtility::gen_sinc(&coeff_init_buffer[0], num_phases * num_convolutions, cutoff, beta);
 ResampleUtility::normalize(&coeff_init_buffer[0], num_phases * num_convolutions, num_phases);

 // Reorder coefficients to allow for more efficient convolution.
 for(int phase = -1; phase < ((int)num_phases + 1); phase++)
 {
  for(int conv = 0; conv < (int)num_convolutions; conv++)
  {
   double coeff;

   if(phase == -1 && conv == 0)
    coeff = 0;
   else if(phase == (int)num_phases && conv == ((int)num_convolutions - 1))
    coeff = 0;
   else
    coeff = coeff_init_buffer[conv * num_phases + phase];

   coeffs[phase + 1][conv] = coeff;
  }
 }

 // Free a bit of mem
 coeff_init_buffer.resize(0);

 step_int = floor(input_to_output_ratio);
 step_fract = input_to_output_ratio - step_int;

 input_pos_fract = 0;

 // Do NOT use rb.size() later in the code, since it'll include the padding.
 // We should only need one "max_mult_minus1" here, not two, since it won't matter if it over-reads(due to doing "max_mult_atatime" multiplications at a time
 // rather than just 1, in which case this over-read wouldn't happen), from the first half into the duplicated half,
 // since those corresponding coefficients will be zero anyway; this is just to handle the case of reading off the end of the duplicated half to
 // prevent illegal memory accesses.
 rb.resize(num_convolutions * 2 + max_mult_minus1);

 rb_readpos = 0;
 rb_writepos = 0;
 rb_in = 0;
}

resample_samp_t SincResample::mac(const resample_samp_t *wave, const resample_coeff_t *coeffs_a, const resample_coeff_t *coeffs_b, const double ffract, unsigned count)
{
 resample_samp_t accum = 0;
#if SINCRESAMPLE_USE_SSE
 __m128 accum_vec_a[2] = { _mm_set1_ps(0), _mm_set1_ps(0) };
 __m128 accum_vec_b[2] = { _mm_set1_ps(0), _mm_set1_ps(0) };

 for(unsigned c = 0; c < count; c += 8) //8) //4)
 {
  __m128 coeff_a[2];
  __m128 coeff_b[2];
  __m128 w[2];
  __m128 result_a[2], result_b[2];

  for(unsigned i = 0; i < 2; i++)
  {
   coeff_a[i] = _mm_load_ps(&coeffs_a[c + (i * 4)]);
   coeff_b[i] = _mm_load_ps(&coeffs_b[c + (i * 4)]);
   w[i] = _mm_loadu_ps(&wave[c + (i * 4)]);

   result_a[i] = _mm_mul_ps(coeff_a[i], w[i]);
   result_b[i] = _mm_mul_ps(coeff_b[i], w[i]);

   accum_vec_a[i] = _mm_add_ps(result_a[i], accum_vec_a[i]);
   accum_vec_b[i] = _mm_add_ps(result_b[i], accum_vec_b[i]);
  }
 }

 __m128 accum_vec, av_a, av_b;
 __m128 mult_a_vec = _mm_set1_ps(1.0 - ffract);
 __m128 mult_b_vec = _mm_set1_ps(ffract);

 av_a = _mm_mul_ps(mult_a_vec, /*accum_vec_a[0]);*/ _mm_add_ps(accum_vec_a[0], accum_vec_a[1]));
 av_b = _mm_mul_ps(mult_b_vec, /*accum_vec_b[0]);*/ _mm_add_ps(accum_vec_b[0], accum_vec_b[1]));

 accum_vec = _mm_add_ps(av_a, av_b);

 accum_vec = _mm_add_ps(accum_vec, _mm_shuffle_ps(accum_vec, accum_vec, (3 << 0) | (2 << 2) | (1 << 4) | (0 << 6)));
 accum_vec = _mm_add_ps(accum_vec, _mm_shuffle_ps(accum_vec, accum_vec, (1 << 0) | (0 << 2) | (1 << 4) | (0 << 6)));

 _mm_store_ss(&accum, accum_vec);
#else
 resample_coeff_t mult_a = 1.0 - ffract;
 resample_coeff_t mult_b = ffract;

 for(unsigned c = 0; c < count; c += 4)
 {
  accum += wave[c + 0] * (coeffs_a[c + 0] * mult_a + coeffs_b[c + 0] * mult_b);
  accum += wave[c + 1] * (coeffs_a[c + 1] * mult_a + coeffs_b[c + 1] * mult_b);
  accum += wave[c + 2] * (coeffs_a[c + 2] * mult_a + coeffs_b[c + 2] * mult_b);
  accum += wave[c + 3] * (coeffs_a[c + 3] * mult_a + coeffs_b[c + 3] * mult_b);
 }
#endif

 return accum;
}

inline bool SincResample::output_avail(void)
{
 return(rb_in >= (int)num_convolutions);
}

resample_samp_t SincResample::read(void)
{
 assert(output_avail());
 double phase = input_pos_fract * num_phases - 0.5;
 signed phase_int = (signed)floor(phase);
 double phase_fract = phase - phase_int;
 unsigned phase_a = num_phases - 1 - phase_int;
 unsigned phase_b = phase_a - 1;
 resample_samp_t ret;

 ret = mac(&rb[rb_readpos], &coeffs[phase_a + 1][0], &coeffs[phase_b + 1][0], phase_fract, num_convolutions);

 unsigned int_increment = step_int;

 input_pos_fract += step_fract;
 int_increment += floor(input_pos_fract);
 input_pos_fract -= floor(input_pos_fract);

 rb_readpos = (rb_readpos + int_increment) % num_convolutions;
 rb_in -= int_increment;

 return ret;
}

inline void SincResample::write(resample_samp_t sample)
{
 assert(!output_avail());

 if(hr_used)
 {
  hr.write(sample);

  if(hr.output_avail())
  {
   sample = hr.read();
  }
  else
  {
   return;
  }
 }

 rb[rb_writepos + 0 * num_convolutions] = sample;
 rb[rb_writepos + 1 * num_convolutions] = sample;
 rb_writepos = (rb_writepos + 1) % num_convolutions;
 rb_in++;
}

void ResampleUtility::kaiser_window( double* io, int count, double beta)
{
        int const accuracy = 24; //16; //12;

        double* end = io + count;

        double beta2    = beta * beta * (double) -0.25;
        double to_fract = beta2 / ((double) count * count);
        double i        = 0;
        double rescale = 0; // Doesn't need an initializer, to shut up gcc

        for ( ; io < end; ++io, i += 1 )
        {
                double x = i * i * to_fract - beta2;
                double u = x;
                double k = x + 1;

                double n = 2;
                do
                {
                        u *= x / (n * n);
                        n += 1;
                        k += u;
                }
                while ( k <= u * (1 << accuracy) );

                if ( !i )
                        rescale = 1 / k; // otherwise values get large

                *io *= k * rescale;
        }
}

void ResampleUtility::gen_sinc(double* out, int size, double cutoff, double kaiser)
{
        assert( size % 2 == 0 ); // size must be even
 
        int const half_size = size / 2;
        double* const mid = &out [half_size];
 
        // Generate right half of sinc
        for ( int i = 0; i < half_size; i++ )
        {
                double angle = (i * 2 + 1) * (M_PI / 2);
                mid [i] = sin( angle * cutoff ) / angle;
        }
 
        kaiser_window( mid, half_size, kaiser );
 
        // Mirror for left half
        for ( int i = 0; i < half_size; i++ )
                out [i] = mid [half_size - 1 - i];
}

void ResampleUtility::gen_sinc_os(double* out, int size, double cutoff, double kaiser)
{
        assert( size % 2 == 1); // size must be odd
 
	for(int i = 0; i < size; i++)
	{
         if(i == (size / 2))
          out[i] = 2 * M_PI * (cutoff / 2); //0.078478; //1.0; //sin(2 * M_PI * (cutoff / 2) * (i - size / 2)) / (i - (size / 2));
	 else
 	  out[i] = sin(2 * M_PI * (cutoff / 2) * (i - size / 2)) / (i - (size / 2));

//	 out[i] *= 0.3635819 - 0.4891775 * cos(2 * M_PI * i / (size - 1)) + 0.1365995 * cos(4 * M_PI * i / (size - 1)) - 0.0106411 * cos(6 * M_PI * i / (size - 1));
//0.42 - 0.5 * cos(2 * M_PI * i / (size - 1)) + 0.08 * cos(4 * M_PI * i / (size - 1));

//         printf("%d %f\n", i, out[i]);
	}

	kaiser_window(&out[size / 2], size / 2 + 1, kaiser);

        // Mirror for left half
        for ( int i = 0; i < size / 2; i++ )
                out [i] = out [size - 1 - i];

}

void ResampleUtility::normalize(double* io, int size, double gain)
{
        double sum = 0;
        for ( int i = 0; i < size; i++ )
                sum += io [i];

        double scale = gain / sum;
        for ( int i = 0; i < size; i++ )
                io [i] *= scale;
}

void* ResampleUtility::make_aligned(void* ptr, unsigned boundary)
{
 unsigned char* null_ptr = (unsigned char *)NULL;
 unsigned char* uc_ptr = (unsigned char *)ptr;

 uc_ptr += (boundary - ((uc_ptr - null_ptr) & (boundary - 1))) & (boundary - 1);

 //while((uc_ptr - null_ptr) & (boundary - 1))
 // uc_ptr++;

 //printf("%16llx %16llx\n", (unsigned long long)ptr, (unsigned long long)uc_ptr);

 assert((uc_ptr - (unsigned char *)ptr) < boundary && (uc_ptr >= (unsigned char *)ptr));

 return uc_ptr;
}
