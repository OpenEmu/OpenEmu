/* Finite impulse response (FIR) resampler with adjustable FIR size */

/* Game_Music_Emu 0.5.2 */
#ifndef FIR_RESAMPLER_H
#define FIR_RESAMPLER_H

#define STEREO        2
#define MAX_RES       32
#define WIDTH         16
#define WRITE_OFFSET  (WIDTH * STEREO) - STEREO
#define ROLLOFF       0.999
#define GAIN          1.0

typedef short sample_t;

extern int Fir_Resampler_initialize( int new_size );
extern void Fir_Resampler_shutdown( void );
extern void Fir_Resampler_clear( void );
extern double Fir_Resampler_time_ratio( double new_factor );
extern double Fir_Resampler_ratio( void );
extern int Fir_Resampler_max_write( void );
extern sample_t* Fir_Resampler_buffer( void );
extern int Fir_Resampler_written( void );
extern int Fir_Resampler_avail( void );
extern void Fir_Resampler_write( long count );
extern int Fir_Resampler_read( sample_t** out, unsigned long count );
extern int Fir_Resampler_input_needed( unsigned long output_count );
extern int Fir_Resampler_skip_input( long count );

#endif
