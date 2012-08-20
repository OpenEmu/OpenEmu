#include "mednafen/mednafen-types.h"
#include "mednafen/mednafen.h"
#include "mednafen/git.h"
#include "mednafen/general.h"
#include <iostream>
#include "libretro.h"

static MDFNGI *game;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static MDFN_Surface *surf;

static uint16_t conv_buf[384 * 224] __attribute__((aligned(16)));
static uint32_t mednafen_buf[384 * 224] __attribute__((aligned(16)));
static bool failed_init;

void retro_init()
{
    MDFN_PixelFormat pix_fmt(MDFN_COLORSPACE_RGB, 16, 8, 0, 24);
    surf = new MDFN_Surface(mednafen_buf, 384, 224, 384, pix_fmt);
    
    std::vector<MDFNGI*> ext;
    MDFNI_InitializeModules(ext);
    
    const char *dir = NULL;
    const char *saves = NULL;
    
    std::vector<MDFNSetting> settings;
    
    environ_cb(RETRO_ENVIRONMENT_GET_SAVES_DIRECTORY, &saves);
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        std::string save_path = saves;
        //dox http://mednafen.sourceforge.net/documentation/09x/vb.html
        
        MDFNSetting vb_parallax = { "vb.disable_parallax", MDFNSF_EMU_STATE, "Disable parallax for BG and OBJ rendering", NULL, MDFNST_BOOL, "1", NULL, NULL, NULL };
        MDFNSetting vb_anaglyph_preset = { "vb.anaglyph.preset", MDFNSF_EMU_STATE, "Disable anaglyph preset", NULL, MDFNST_BOOL, "disabled", NULL, NULL, NULL };
        MDFNSetting vb_anaglyph_lcolor = { "vb.anaglyph.lcolor", MDFNSF_EMU_STATE, "Anaglyph l color", NULL, MDFNST_BOOL, "0xFF0000", NULL, NULL, NULL };
        //MDFNSetting vb_anaglyph_lcolor = { "vb.anaglyph.lcolor", MDFNSF_EMU_STATE, "Anaglyph l color", NULL, MDFNST_BOOL, "0xFFFFFF", NULL, NULL, NULL };
        MDFNSetting vb_anaglyph_rcolor = { "vb.anaglyph.rcolor", MDFNSF_EMU_STATE, "Anaglyph r color", NULL, MDFNST_BOOL, "0x000000", NULL, NULL, NULL };
        //MDFNSetting vb_allow_draw_skip = { "vb.allow_draw_skip", MDFNSF_EMU_STATE, "Allow draw skipping", NULL, MDFNST_BOOL, "1", NULL, NULL, NULL };
        //MDFNSetting vb_instant_display_hack = { "vb.instant_display_hack", MDFNSF_EMU_STATE, "ADisplay latency reduction hack", NULL, MDFNST_BOOL, "1", NULL, NULL, NULL };
        MDFNSetting filesys = { "filesys.path_sav", MDFNSF_NOFLAGS, "Memcards", NULL, MDFNST_STRING, save_path.c_str() };
        settings.push_back(vb_parallax);
        settings.push_back(vb_anaglyph_preset);
        settings.push_back(vb_anaglyph_lcolor);
        settings.push_back(vb_anaglyph_rcolor);
        //settings.push_back(vb_allow_draw_skip);
        //settings.push_back(vb_instant_display_hack);
        settings.push_back(filesys);
        MDFNI_Initialize(dir, settings);
    }
    else
    {
        fprintf(stderr, "System directory is not defined. Cannot continue ...\n");
        failed_init = true;
    }
    
    // Hints that we need a fairly powerful system to run this.
    unsigned level = 3;
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_deinit()
{
    delete surf;
    surf = NULL;
}

void retro_reset()
{
    MDFNI_Reset();
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
    return false;
}

bool retro_load_game(const struct retro_game_info *info)
{
    if (failed_init)
        return false;
    game = MDFNI_LoadGame("vb", info->path);
    return game;
}

void retro_unload_game()
{
    MDFNI_CloseGame();
}

#ifndef __SSE2__
#error "SSE2 required."
#endif

#include <emmintrin.h>

// PSX core should be able to output ARGB1555 directly,
// so we can avoid this conversion step.
// Done in SSE2 here because any system that can run this
// core to begin with will be at least that powerful (as of writing).
static inline void convert_surface()
{
    const uint32_t *pix = surf->pixels;
    for (unsigned i = 0; i < 384 * 224; i += 8)
    {
        __m128i pix0 = _mm_load_si128((const __m128i*)(pix + i + 0));
        __m128i pix1 = _mm_load_si128((const __m128i*)(pix + i + 4));
        
        __m128i red0   = _mm_and_si128(pix0, _mm_set1_epi32(0xf80000));
        __m128i green0 = _mm_and_si128(pix0, _mm_set1_epi32(0x00f800));
        __m128i blue0  = _mm_and_si128(pix0, _mm_set1_epi32(0x0000f8));
        __m128i red1   = _mm_and_si128(pix1, _mm_set1_epi32(0xf80000));
        __m128i green1 = _mm_and_si128(pix1, _mm_set1_epi32(0x00f800));
        __m128i blue1  = _mm_and_si128(pix1, _mm_set1_epi32(0x0000f8));
        
        red0   = _mm_srli_epi32(red0,   19 - 10); 
        green0 = _mm_srli_epi32(green0, 11 -  5); 
        blue0  = _mm_srli_epi32(blue0,   3 -  0); 
        
        red1   = _mm_srli_epi32(red1,   19 - 10); 
        green1 = _mm_srli_epi32(green1, 11 -  5); 
        blue1  = _mm_srli_epi32(blue1,   3 -  0); 
        
        __m128i res0 = _mm_or_si128(_mm_or_si128(red0, green0), blue0);
        __m128i res1 = _mm_or_si128(_mm_or_si128(red1, green1), blue1);
        
        _mm_store_si128((__m128i*)(conv_buf + i), _mm_packs_epi32(res0, res1));
    }
}

// Hardcoded for PSX. No reason to parse lots of structures ...
// See mednafen/psx/input/gamepad.cpp
static void update_input()
{
    static uint16_t input_buf[2];
    input_buf[0] = input_buf[1] = 0;
    static unsigned map[] = {
        RETRO_DEVICE_ID_JOYPAD_A,
        RETRO_DEVICE_ID_JOYPAD_B,
        RETRO_DEVICE_ID_JOYPAD_R,
        RETRO_DEVICE_ID_JOYPAD_L,
        RETRO_DEVICE_ID_JOYPAD_L2,    //right d-pad UP
        RETRO_DEVICE_ID_JOYPAD_R3,    //right d-pad RIGHT
        RETRO_DEVICE_ID_JOYPAD_RIGHT, //left d-pad
        RETRO_DEVICE_ID_JOYPAD_LEFT,  //left d-pad
        RETRO_DEVICE_ID_JOYPAD_DOWN,  //left d-pad
        RETRO_DEVICE_ID_JOYPAD_UP,    //left d-pad
        RETRO_DEVICE_ID_JOYPAD_START,
        RETRO_DEVICE_ID_JOYPAD_SELECT,
        RETRO_DEVICE_ID_JOYPAD_R2,    //right d-pad LEFT
        RETRO_DEVICE_ID_JOYPAD_L3,    //right d-pad DOWN
    };
    
    for (unsigned j = 0; j < 2; j++)
    {
        for (unsigned i = 0; i < 14; i++)
            input_buf[j] |= map[i] != -1u &&
            input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, map[i]) ? (1 << i) : 0;
    }
    
    // Possible endian bug ...
    game->SetInput(0, "gamepad", &input_buf[0]);
    game->SetInput(1, "gamepad", &input_buf[1]);
}

void retro_run()
{
    input_poll_cb();
    
    update_input();
    
    static int16_t sound_buf[0x10000];
    static MDFN_Rect rects[384];
    
    EmulateSpecStruct spec = {0}; 
    spec.surface = surf;
    spec.SoundRate = 48000;
    spec.SoundBuf = sound_buf;
    spec.LineWidths = rects;
    spec.SoundBufMaxSize = sizeof(sound_buf) / 2;
    spec.SoundVolume = 1.0;
    spec.soundmultiplier = 1.0;
    
    MDFNI_Emulate(&spec);
    
    //unsigned width = rects[0].w;
    //unsigned height = spec.DisplayRect.h;
    unsigned width = 384;
    unsigned height = 224;
    
    convert_surface();
    video_cb(conv_buf, width, height, width << 1);
    
    audio_batch_cb(spec.SoundBuf, spec.SoundBufSize);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "Mednafen Virtual Boy";
    info->library_version  = "0.9.22";
    info->need_fullpath    = true;
    info->valid_extensions = "vb|VB";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    // Just assume NTSC for now. TODO: Verify FPS.
    info->timing.fps            = 50.27;
    info->timing.sample_rate    = 48000; //actual is 41700?
    info->geometry.base_width   = game->nominal_width;
    info->geometry.base_height  = game->nominal_height;
    info->geometry.max_width    = 384;
    info->geometry.max_height   = 224;
    info->geometry.aspect_ratio = 4.0 / 3.0;
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

// TODO: Allow for different kinds of joypads?
void retro_set_controller_port_device(unsigned, unsigned)
{}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static size_t serialize_size;
size_t retro_serialize_size(void)
{
    //if (serialize_size)
    //   return serialize_size;
    
    if (!game->StateAction)
    {
        fprintf(stderr, "[mednafen]: Module %s doesn't support save states.\n", game->shortname);
        return 0;
    }
    
    StateMem st;
    memset(&st, 0, sizeof(st));
    
    if (!MDFNSS_SaveSM(&st, 0, 1))
    {
        fprintf(stderr, "[mednafen]: Module %s doesn't support save states.\n", game->shortname);
        return 0;
    }
    
    free(st.data);
    return serialize_size = st.len;
}

bool retro_serialize(void *data, size_t size)
{
    StateMem st;
    memset(&st, 0, sizeof(st));
    st.data     = (uint8_t*)data;
    st.malloced = size;
    
    return MDFNSS_SaveSM(&st, 0, 1);
}

bool retro_unserialize(const void *data, size_t size)
{
    StateMem st;
    memset(&st, 0, sizeof(st));
    st.data = (uint8_t*)data;
    st.len  = size;
    
    return MDFNSS_LoadSM(&st, 0, 1);
}

void *retro_get_memory_data(unsigned)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned)
{
    return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned, bool, const char *)
{}

