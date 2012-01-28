/* Fast sound synthesis buffer for use in real-time emulators of electronic
sound generator chips like those in early video game consoles. Uses linear
interpolation. Higher-quality versions are available that use sinc-based
band-limited synthesis. */

#ifndef BLIP_H
#define BLIP_H

#ifdef __cplusplus
  extern "C" {
#endif

/* Creates a new blip_buffer with specified input clock rate, output
sample rate, and size (in samples), or returns NULL if out of memory. */
typedef struct blip_buffer_t blip_buffer_t;
blip_buffer_t* blip_alloc( double clock_rate, double sample_rate, int size );

/* Frees memory used by a blip_buffer. No effect if NULL is passed. */
void blip_free( blip_buffer_t* );

/* Removes all samples and clears buffer. */
void blip_clear( blip_buffer_t* );

/* Adds an amplitude transition of delta at specified time in source clocks.
Delta can be negative. */
void blip_add( blip_buffer_t*, int time, int delta );

/* Number of additional clocks needed until n samples will be available.
If buffer cannot even hold n samples, returns number of clocks until buffer
becomes full. */
int blip_clocks_needed( const blip_buffer_t*, int samples_needed );

/* Ends current time frame of specified duration and make its samples available
(along with any still-unread samples) for reading with read_samples(), then
begins a new time frame at the end of the current frame. */
void blip_end_frame( blip_buffer_t*, int duration );

/* Number of samples available for reading with read(). */
int blip_samples_avail( const blip_buffer_t* );

/* Reads at most n samples out of buffer into out, removing them from from
the buffer. Returns number of samples actually read and removed. If stereo is
true, increments 'out' one extra time after writing each sample, to allow
easy interleving of two channels into a stereo output buffer. */
int blip_read_samples( blip_buffer_t*, short out [], int n, int stereo );

#ifdef __cplusplus
  }
#endif

#endif
