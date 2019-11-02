////////////////////////////////////////////////////////////////////////////////
////          crt-royale, by TroggleMonkey <trogglemonkey@gmx.com>          ////
////                     Last Updated: August 16, 2014                      ////
////////////////////////////////////////////////////////////////////////////////

REQUIREMENTS:
The earliest official Retroarch version fully supporting crt-royale is 1.0.0.3
(currently unreleased).  Earlier versions lack shader parameters and proper
mipmapping and sRGB support, but the shader may still run at reduced quality.

The earliest development version fully supporting this shader is:
    commit ba40be909913c9ccc34dab5d452fba4fe61af9d0
    Author: Themaister <maister@archlinux.us>
    Date:   Thu Jun 5 17:41:10 2014 +0200
A few earlier revisions support the required features, but they may be buggier.


BASICS:
crt-royale is a highly customizable CRT shader for Retroarch and other programs
supporting the libretro Cg shader standard.  It uses a number of nonstandardized
extensions like sRGB FBO's, mipmapping, and runtime shader parameters, but
hopefully it will run without much of a fuss on new implementations of the
standard as well.

There are a huge number of parameters.  Among the things you can customize:
* Phosphor mask type: An aperture grille, slot mask, and shadow mask are each
  included, although the latter won't be seeing much usage until 1440p displays
  and better become more common (4k UHD and 8k UHD are increasingly optimal).
* Phosphor mask dot pitch
* Phosphor mask resampling method: Choose between Lanczos sinc resizing,
  mipmapped hardware resizing, and no resizing of the input LUT.
* Phosphor bloom softness and type (real or fake ;))
* Gaussian and generalized Gaussian scanline beam properties/distribution,
  including convergence offsets
* Screen geometry, including curvature (spherical, alternative spherical, or
  cylindrical like Trinitrons), tilt, and borders
* Antialiasing level, resampling filter, and sharpness parameters for gracefully
  combining screen curvature with high-frequency phosphor details, including
  optionally resampling based on RGB subpixel positions.
* Halation (electrons bouncing under the glass and lighting random phosphors)
  random phosphors)
* Refractive diffusion (light spreading from the imperfect CRT glass face)
* Interlacing options
* etc.

There are two major ways to customize the shader:
* Runtime shader parameters allow convenient experimentation with real-time
  feedback, but they are much slower, because they prevent static evaluation of
  a lot of math.  Disabling them drastically speeds up the shader.
* If runtime shader parameters are disabled (partially or totally), those same
  settings can be freely altered in the text of the user-settings.h file.  There
  are also a number of other static-only settings, including the #define macros
  which indicate where and when to allow runtime shader parameters.  To disable
  them entirely, comment out the "#define RUNTIME_SHADER_PARAMS_ENABLE" line by
  putting a double-slash ("//") at the beginning...your FPS will skyrocket.

You may also note that there are two major versions of the shader preset:
* crt-royale.cgh is the "full" version of the shader, which blooms the light
  from the brighter phosphors to maintain brightness and avoid clipping.
* crt-royale-fake-bloom.cgh is the "cheater's" version of the shader, which
  only fakes the bloom based on carefully blending in a [potentially blurred]
  version of the original input.  This version is MUCH faster, and you have to
  strain to see the difference, so people with slower GPU's will prefer it.

There's a lot to play around with, and I encourage everyone using this shader to
read through the user-settings.h file to learn about the parameters.  Before
loading the shader, be sure to read the next section, entitled...


////////////////////////////////////////////////////////////////////////////////
////                    FREQUENTLY EXPECTED QUESTIONS:                      ////
////////////////////////////////////////////////////////////////////////////////

1.) WHY IS THE SHADER CRASHING WHEN I LOAD IT?!?
Do you get C6001 or C6002 errors with integrated graphics, like Intel HD 4000?
If so, please try one of the following .cgp presets:
* crt-royale-intel.cgp
* crt-royale-fake-bloom-intel.cgp
These load .cg wrappers that #define INTEGRATED_GRAPHICS_COMPATIBILITY_MODE
(also available in user-settings.h) before loading the main .cg shader files.

Integrated graphics compatibility mode will disable these three features, which
currently require more registers or instructions than Intel GPU's allow:
* PHOSPHOR_MASK_MANUALLY_RESIZE: The phosphor mask will be softer.
  (This may be reenabled in a later release.)
* RUNTIME_GEOMETRY_MODE: You must change the screen geometry/curvature using
  the geom_mode_static setting in user-settings.h.
* The high-quality 4x4 Gaussian resize for the bloom approximation

Using Intel-specific .cgp files is equivalent to #defining
INTEGRATED_GRAPHICS_COMPATIBILITY_MODE in your user-settings.h.  Out of the box,
user-settings.h is configured for maximum configurability and compatibility with
dedicated nVidia and AMD/ATI GPU's.  Compatibility mode is disabled by default
to avoid silently degrading quality for AMD/ATI and nVidia users, so Intel-
specific .cgp's are a convenient way for Intel users to play with the shader
without editing text files.

I've tested this solution on Intel HD 4000 graphics, and it should work for that
GPU at least, but please let me know if you're still having problems!

--------------------------------------------------------------------------------

2.) WHY IS EVERYTHING SO SLOW?!?:
Out of the box, this will be a problem for all but monster GPU's.  The default
user-settings.h file disables any features and optimizations which might cause
compilation failure on AMD/ATI GPU's.  Despite the name of the options, this is
not a problem with your card or drivers; it's a shortcoming in the Cg shader
compiler's nVidia-centric profile setups.

Uncommenting the following #define macros at the top of user-settings.h will
help performance a good deal on compatible nVidia cards:
    #define DRIVERS_ALLOW_DERIVATIVES
    #define DRIVERS_ALLOW_DYNAMIC_BRANCHES
    #define ACCOMODATE_POSSIBLE_DYNAMIC_LOOPS
    #define DRIVERS_ALLOW_TEX2DLOD
    #define DRIVERS_ALLOW_TEX2DBIAS
A few of these warrant some elaboration.  First, derivatives:

Derivatives allow the shader to cheaply calculate a tangent-space matrix for
correct antialiasing when curvature or overscan are used.  Without them, there
are two options:
    a.) Cheat, and there will be artifacts with strong cylindrical curvature
    b.) Compute the correct tangent-space matrix analytically.  This is used
        by default, and it's controlled by this option near the bottom:
            geom_force_correct_tangent_matrix = true

Dynamic branches:
Dynamic branches allow the shader to avoid performing computations that it
doesn't need (but might have, given different runtime options).  Without them,
the shader has to either let the GPU evaluate every possible codepath and select
a result, or make a "best guess" ahead of time.  The full phosphor bloom suffers
most from not having dynamic branches, because the shader doesn't know how big
of a blur to use until it knows your phosphor mask dot pitch...which you set at
runtime if shader parameters are enabled.

If RUNTIME_PHOSPHOR_BLOOM_SIGMA is commented out (faster), this won't matter:
The shader will just select the blur size and standard deviation suitable for
the mask_triad_size_desired_static setting in user-settings.cgp.  It will be
fast, but larger triads won't blur enough, and smaller triads will blur more
than they need to.  However, if RUNTIME_PHOSPHOR_BLOOM_SIGMA is enabled, the
shader will calculate an optimal standard deviation and *try* to use the right
blur size for it...but using an "if standard deviation is such and such"
condition would be prohibitively slow without dynamic branches.  Instead, the
shader uses the largest and slowest blur the user lets it use (to cover the
widest range of triad sizes and standard deviations), according to these macros:
    #define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS
The more you have uncommented, the larger the triads you can blur, but the
slower runtime sigmas will be if your GPU can't use dynamic branches.  By
default, triads up to 6 pixels wide will be bloomed perfectly, and a little
beyond that (8 should be fine), but going too far beyond that will create
blocking artifacts in the blur due to an insufficient support size.

tex2Dlod:
The tex2Dlod function allows the shader to disables anisotropic filtering, which
can get confused when we're manually tiling the texture coordinates for a small
resized phosphor mask tile (it creates nasty seam artifacts).  There are several
ways the shader can deal with this: The cheapest is to use tex2Dlod to tile the
output of MASK_RESIZE across the screen...and the slower alternatives either
require derivatives or force the shader to draw 2 tiles to MASK_RESIZE in each
direction, thereby reducing your maximum allowed dot pitch by half.

tex2Dbias:
According to nVidia's Cg language standard library page, tex2Dbias requires the
fp30 profile, which doesn't work on ATI/AMD cards...but you might actually have
mixed results.  This can be used as a substitute for tex2Dlod at times, so it's
worth trying even on ATI.

--------------------------------------------------------------------------------

3.) WHY IS EVERYTHING STILL SO SLOW?!?:
For maximum quality and configurability out of the box, almost all shader
parameters are enabled by default (except for the disproportionately expensive
runtime subpixel offsets).  Some are more expensive than others.  Commenting
the following macro disables all shader parameters:
    #define RUNTIME_SHADER_PARAMS_ENABLE
Commenting these macros disables selective shader parameters:
    #define RUNTIME_PHOSPHOR_BLOOM_SIGMA
    #define RUNTIME_ANTIALIAS_WEIGHTS
    //#define RUNTIME_ANTIALIAS_SUBPIXEL_OFFSETS
    #define RUNTIME_SCANLINES_HORIZ_FILTER_COLORSPACE
    #define RUNTIME_GEOMETRY_TILT
    #define RUNTIME_GEOMETRY_MODE
    #define FORCE_RUNTIME_PHOSPHOR_MASK_MODE_TYPE_SELECT
Note that all shader parameters will still show up in your GUI list, and the
disabled ones simply won't work.

Finally, there are a lot of other options enabled by default that carry serious
performance penalties.  For instance, the default antialiasing filter is a
cubic filter, because it's the most configurable, but it's also quite slow if
RUNTIME_ANTIALIAS_WEIGHTS is #defined.  A lot of the static true/false options
have a significant influence, and the shader is faster if the red subpixel
offset (from which the blue one is calculated as well) is zero...even if it's
a static value, because RUNTIME_ANTIALIAS_SUBPIXEL_OFFSETS is commented out.
To avoid any confusion, I should also clarify now that subpixel offsets are
separate from scanline beam convergence offsets.

To quickly see how much performance you can get from other settings, you can
temporarily replace your user-settings.h with one of:
a.) crt-royale-settings-files/user-settings-fast-static-ati.h
b.) crt-royale-settings-files/user-settings-fast-static-nvidia.h
Then load crt-royale-fake-bloom.cgp.  It should be far more playable.

--------------------------------------------------------------------------------

4.) WHY WON'T MY SHADER BLOOM MY PHOSPHORS ENOUGH?
First, see the discussion about dynamic branching above, in 1.
If you don't have dynamic branches, you can either uncomment the lines that
let the shader pessimistically use larger blurs than it's guaranteed to need
(which is slow), or...you can just use crt-royale-fake-bloom.cgp, which
doesn't have this problem. :)

--------------------------------------------------------------------------------

5.) WHY CAN'T I MAKE MY PHOSPHORS ANY BIGGER?
By default, the phosphor mask is Lanczos-resized in earlier passes to your
specified dot pitch (mask_sample_mode = 0).  This gives a much sharper result
than mipmapped hardware sampling (mask_sample_mode = 1), but it can be much
slower without taking proper care: If the input mask tile (containing 8
phosphor triads by default) is large, like 512x512, and you try to resize it
to 24x24 for 3x3 pixel triads, the resizer has to take 128 samples in each
pass/direction (the max allowed) for a 3-lobe Lanczos filter.  This can be
very slow, so I made the output of MASK_RESIZE very small by default: Just
1/16th of the viewport size in each direction.  The exact limit scales with
your viewport size, and it *should* be reasonable, but the restrictions can
get tighter if we can't use tex2Dlod and have to fit two whole tiles (16
phosphor triads with default 8-triad tiles) into the MASK_RESIZE pass for
compatibility with anisotropic filtering (long story).

If you want bigger phosphor triads, you have two options:
a.) Set mask_sample_mode to 1 in your shader params (if enabled) or set
    mask_sample_mode_static to 1 in your user-settings.h file.  This will use
    hardware sampling, which is softer but has no limitations.
b.) To increase the limit with manual mask-resizing (best quality), you need to
    do five things:
    1.) Go into your .cgp file and find the MASK_RESIZE pass (the horizontal
        mask resizing pass) and the one before it (the vertical mask resizing
        pass).  Find the viewport-relative scales, which should say 0.0625, and
        change them to 0.125 or even 0.25.
    2.) Still in your .cgp file, also make sure your mask_*_texture_small
        filenames point to LUT textures that are larger than your final desired
        onscreen size (upsizing is not currently permitted).
    3.) Go into user-cgp-constants.h and change mask_resize_viewport_scale from
        0.0625 to the new value you changed it to in step 1.  This is necessary,
        because we can't pass that value from the .cgp file to the shader, and
        the shader can't compute the viewport size (necessary) without it.
    4.) Still in user-cgp-constants.h, update mask_texture_small_size and
        mask_triads_per_tile appropriately if you changed your LUT texture in
        step 2.
    5.) Reload your .cgp file.
I REALLY wish there was an easier way to do that, but my hands are tied until
.cgp files are allowed to pass more information to .cg shaders (which would
require major updates to the cg2glsl script).

--------------------------------------------------------------------------------

6.) WHY CAN'T I MAKE MY PHOSPHORS ANY SMALLER THAN 2 PIXELS PER TRIAD?
This is controlled by mask_min_allowed_triad_size in your user-settings.h file.
Set it to 1.0 instead of 2.0 (anything lower than 1 is pointless), and you're
set.  It defaults to 2.0 to make mask resizing twice as fast when dynamic
branches aren't allowed.  Some people may want to be able to fade the phosphors
away entirely to get a more PVM-like scanlined image though, so change it to 1.0
for that (or get a higher-resolution display ;)).

Note: This setting should be obsolete soon.  I have some ideas for more
sophisticated mask resampling that I just don't have a spare few hours to
implement yet.

--------------------------------------------------------------------------------

7.) I AM NOT RUNNING INTEGRATED GRAPHICS.  WHY AM I GETTING ERRORS?
First recheck the top of your user-settings.h to make sure incompatible driver
options are commented out (disabled).  If they're all disabled and you're still
having problems, you've probably found a bug.  There are bound to be a number of
them with certain setting combinations, and there might even be a few individual
settings I broke more recently than I tested them.  My contact information is up
top, so let me know!

--------------------------------------------------------------------------------

8.) WHY AM I GETTING BANDING IN DARK COLORS?  OR, WHY WON'T MIPMAPPING WORK?
crt-royale uses features like sRGB and mipmapping, which are not available in
the latest Retroarch release (1.0.0.2) at the time of this writing.

You may get banding in dark colors if your platform or Retroarch version doesn't
support sRGB FBO's, and mask_sample_mode 1 will look awful without mipmapping.
I expect most platforms capable of running this shader at full speed will
support sRGB FBO's, but if yours doesn't, please let me know, and I'll include
a note about it.

Alternately, setting levels_autodim_temp too low will cause precision loss and
banding.

--------------------------------------------------------------------------------

9.) HOW DO I SET GEOMETRY/CURVATURE/ETC.?
If RUNTIME_SHADER_PARAMS_ENABLE and RUNTIME_GEOMETRY_MODE are both #defined (not
commented out) in user-settings.cgp, you can find these options in your shader
parameters (in Retroarch's RGUI for instance) under e.g. geom_mode.  Otherwise,
you can set the corresponding e.g. geom_mode_static options in user-settings.h.

--------------------------------------------------------------------------------

10.) WHY DON'T MY SHADER PARAMETERS STICK?
This is a bit confusing, at least in the version of Retroarch I'm using.
In the Shader Options menu, Parameters (Current) controls what's on your screen
right now, whereas Parameters (RGUI) seems to control what gets saved to a
shader preset (in your base shaders directory) with Save As Shader Preset.

--------------------------------------------------------------------------------

11.) WHY DID YOU SLOW THE SHADER DOWN WITH ALL OF THESE FEATURES I DON'T WANT?
     WHY DIDN'T YOU MAKE THE DEFAULTS MORE TO MY LIKING?

The default settings tend to best match flat ~13" slot mask TV's with sharp
scanlines.  Real CRT's however vary a lot in their characteristics (and many are
softer in more ways than one), so it's impossible to make the default settings
look like everyone's favorite CRT.  Moreover, it's impossible to decide which
of the slower features and options are superfluous:

Some people love curvature, and some people hate it.  Some people love
scanlines, and some people hate them.  Some people love phosphors, and some
people hate them.  Some people love interlacing support, and some people hate
it.  Some people love sharpness, and some people hate it.  Some people love
convergence error, and some people hate it.  The one thing you hate the most is
probably someone else's most critical feature.  This is why there are so many
options, why the shader is so complicated, and why it's impossible to please
everyone out of the box...unfortunately.

That said, if you spend some time tweaking the settings, you're bound to get a
picture you like.  Once you've made up your mind, you can save the settings to
a user-settings.h file and disable shader parameters and other slow options to
get the kind of performance you want.

--------------------------------------------------------------------------------

12.) WHY DIDN'T YOU INCLUDE A SHADER PRESET WITH NTSC SUPPORT?  WHY DIDN'T YOU
     INCLUDE MORE CANNED PRESETS WITH DIFFERENT OPTIONS?  WHY CAN'T I SELECT
     FROM ONE OF SEVERAL USER SETTINGS FILES WITHOUT MANUAL FILE RENAMING?

I do plan on adding a version that uses the NTSC shader for the first two
passes, but it will take a bit of work, because there are several NTSC shader
versions as it is.  It's easy enough to combine the HALATION_BLUR passes into a
one-pass blur from blurs/blur9x9fast.cg, but I'm not sure yet just how much
modification the NTSC shader passes themselves might need for best results.

I originally wanted NTSC support to be included out-of-the-box, but I'd also
like to release the shader ASAP, so it'll have to wait.

As for other canned presets, that's a little more complicated: I DO intend on
creating more canned presets, but the combinatorial explosion of major codepath
options in this shader is too overwhelming to be as exhaustive as I'd like.
When I get the time, I'll add what I can to make this more user-friendly.
In the meantime, I'll start adding a few different default versions of the
user settings file and put them in a subdirectory for people to manually
place in the main directory and rename to "user-settings.h."

However, the libretro Cg shader specification (and the Cg to GLSL compiler) does
not currently allow .cgp files to pass any static settings to the source files.
This presents a huge problem, because it means that in order to create a new
preset with different options, I also have to create duplicate files for EVERY
single .cg pass for every permutation, not just the .cgp.  I plan on creating
a number of skeleton wrapper .cg files in a subdirectory (which set a few
options and then include the main .cg file for the pass), but it'll be a while
yet.  In the meantime, I'd rather let people play with what's already done than
keep it hidden on my hard drive.

--------------------------------------------------------------------------------

13.) WHY DO SO MANY VALUES IN USER_SETTINGS.H HAVE A _STATIC SUFFIX?

The "_static" suffix is there to prevent naming conflicts with runtime shader
parameters: The shader usually uses a version without the suffix, which is
assigned either the value of the "_static" version or the runtime shader
parameter version.  If a value in uset-settings.h doesn't have a "_static"
suffix, it's usually because it's a static compile-time option only, with no
corresponding runtime version.  Basically, you can ignore the suffix. :)

--------------------------------------------------------------------------------

14.) ARE THERE ANY BROKEN SETTINGS I SHOULD BE AWARE OF?
     WHAT IF I WANT TO CHANGE SETTINGS IN THE .CGP FILE?

As far as I know, all of the options in user-settings.h and the runtime shader
parameters are pretty robust, with a few caveats:
* As noted above, there are some tradeoffs between runtime and compile-time
  options.  If runtime blur sigmas are disabled for instance, the phosphor
  bloom (and to a lesser extent, the fake bloom) may not blur the right amount.
* If you set your aspect ratio incorrectly, and mask_specify_num_triads == 1.0
  (i.e. true, as opposed to 0.0, which is false), the shader will misinterpret
  the number of triads you want by the same proportion.
* Disabled shader parameters will do nothing, including either:
    a.) mask_triad_size_desired
    b.) mask_num_triads_desired,
  depending on the value of mask_specify_num_triads.

There is a broken and unimplemented option in derived-settings-and-constants.h,
but users shouldn't need to mess around in there anyway.  (It's related to the
more efficient phosphor mask resampling I want to implement.)

However, the .cgp files are another story: They are pretty brittle, especially
when it comes to their interaction with user-cgp-constants.h.  Be aware that the
shader passes rely on scale types and sizes in your .cgp file being exactly what
they expect.  Do not change any scale types from the defaults, or you'll get
artifacts under certain conditions.  You can change the BLOOM_APPROX and
MASK_RESIZE scale values (not scale types), but you must update the associated
constant in user-cgp-constants.h to let the .cg shader files know about it, and
the implications may reach farther than you expect.  Similarly, if you plan on
changing an LUT texture, make sure you update the associated constants in
user-cgp-constants.h.  In short, if you plan on changing anything in a .cgp
file, you'll want to read it thoroughly first, especially the "IMPORTANT"
section at the top.

--------------------------------------------------------------------------------

15.) WHAT ARE THE MOST COMMON DOT PITCHES FOR CRT TELEVISIONS?
     WHAT KIND OF RESOLUTION WOULD I NEED FOR A REAL SHADOW MASK?

The most demanding CRT we're ever likely to emulate is a Sony PVM-20M4U:
    Width: 450mm
    Aperture Grille Pitch: 0.31mm
    Triads in 4:3 frame: 1451, assuming little to no overscan
For 3-pixel triads, we would need about 6k UHD resolution.  A BVM-20F1U has
similar requirements.

However, common slot masks are far more similar to the kind of image this shader
will produce at 900p, 1080p, 1200p, and 1440p:
1.) A typical 13" diagonal CRT might have a 0.60mm slot pitch, for a total of
    440.26666666666665 or so phosphor triads horizontally.
2.) A typical 19" diagonal CRT might have a 0.75mm slot pitch, for a total of
    514.7733333333333 or so phosphor triads horizontally.
3.) According to http://repairfaq.ece.drexel.edu/REPAIR/F_crtfaq.html, a
    typical 25" diagonal CRT might have a 0.9mm slot pitch, for a total of
    564.4444444444445 or so phosphor triads horizontally.
4.) A 21" Samsung SMC210N CCTV monitor (450 TV lines) has a 0.7mm stripe
    pitch, for a total of 609.6 or so phosphor triads horizontally.

The included EDP shadow mask starts looking very good with ~6-pixel triads, so
it may take nearly 4k resolution to make it a particularly compelling option.
However, it's possible to make smaller shadow masks on a pixel-by-pixel basis
and tile them at a 1:1 ratio (mask_sample_mode = 2).  I may include a mask like
this in a future update.

--------------------------------------------------------------------------------

16.) IS THIS PHOSPHOR BLOOM REALISTIC?

Probably not:

Realistically, the "phosphor bloom" blurs bright phosphors significantly more
than your eyes would bloom the brighter phosphors on a real CRT.  This extra
blurring however is necessary to distribute enough brightness to nearby pixels
that we can amplify the overall brightness to that of the original source after
applying the phosphor mask.  If you're interested, there are more comments on
the subject at the top of the fragment shader in crt-royale-bloom-approx.cg.

On the subject of the phosphor bloom: I intended to include some exposition
about the math behind the brightpass calculation (and the much more complex
and thorough calculation I originally used to blur the minimal amount necessary,
which turned out to be inferior in practice), but that document isn't release-
ready at the moment.  Sorry Hyllian. ;)

--------------------------------------------------------------------------------

17.) SO WHAT DO YOU PLAN ON ADDING IN THE FUTURE?

I'd like to add these relatively soon:
1.) A combined ntsc-crt-royale.cgp and ntsc-crt-royale-fake-bloom.cgp.
2.) More presets, especially if maister or squarepusher find a way to make the
Cg to GLSL compiler process .cgp files (which will allows .cgp's to pass
arbitrary #defines to the .cg shader passes).
3.) More efficient and flexible phosphor mask resampling.  Hopefully, this will
    make it possible to manually resize the mask on Intel HD Graphics as well.
4.) Make it more easy and convenient to use and experiment with mask_sample_mode
    2 (direct 1:1 tiling of an input texture) by using a separate LUT texture
    with its own parameters in user-cgp-constants.h, etc.  I haven't done this
    yet because it requires yet another texture sample that could hurt other
    codepaths, and I'm waiting until I have time to optimize it.
5.) Refine the runtime shader parameters: Some of them are probably too fine-
    grained and slow to change.

Maybe's:
1.) I've had trouble getting LUT's from subdirectories to work consistently
    across platforms, but I'd like to get around that and include more mask
    textures I've made.
2.) If you're using spherical curvature with a small radius, the edges of the
    sphere are blocky due to the pixel discards being done in 2x2 fragment
    blocks.  I'd like to fix this if it can be done without a performance hit.
3.) I have some ideas for procedural mask generation with a fast, closed-form
    low-pass filter, but I don't know if I'll ever get around to it.

