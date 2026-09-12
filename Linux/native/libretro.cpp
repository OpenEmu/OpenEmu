/* SPDX-License-Identifier: MIT
 * Software-rendered Libretro frontend for the OpenEmu Linux helper process.
 * The official API header and its license are retained in vendor/libretro.h.
 */
#include "core.h"
#include "vendor/libretro.h"
#include <dlfcn.h>
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct Host {
    void *module = nullptr;
    bool initialized = false, loaded = false, shutdown = false;
    unsigned keys = 0, pixel_format = RETRO_PIXEL_FORMAT_0RGB1555;
    unsigned width = 256, height = 224;
    double fps = 60, rate = 48000;
    double phase = 0;
    double accumulated = 0, sum_left = 0, sum_right = 0;
    int16_t previous_left = 0, previous_right = 0;
    bool have_previous = false;
    std::string rom, save, system, core, error;
    std::vector<char> content;
    std::vector<uint32_t> pixels;
    std::vector<int16_t> audio;
    std::vector<int16_t> output;
    std::map<std::string,std::string> options;
#define API(name) decltype(&retro_##name) name = nullptr
    API(init); API(deinit); API(api_version); API(get_system_info); API(get_system_av_info);
    API(set_environment); API(set_video_refresh); API(set_audio_sample); API(set_audio_sample_batch);
    API(set_input_poll); API(set_input_state); API(set_controller_port_device);
    API(load_game); API(unload_game); API(run); API(reset); API(serialize_size);
    API(serialize); API(unserialize); API(get_memory_data); API(get_memory_size);
#undef API
};
static Host *active = nullptr;
static std::string last_error;
static void log_message(enum retro_log_level, const char *format, ...) {
    va_list args; va_start(args, format); vfprintf(stderr, format, args); va_end(args);
}
static bool valid_av(const retro_system_av_info &info) {
    return info.geometry.base_width > 0 && info.geometry.base_width <= 1024 &&
        info.geometry.base_height > 0 && info.geometry.base_height <= 1024 &&
        std::isfinite(info.timing.fps) && info.timing.fps >= 10 && info.timing.fps <= 240 &&
        std::isfinite(info.timing.sample_rate) && info.timing.sample_rate >= 8000 && info.timing.sample_rate <= 4194304;
}
template<class T> static void defaults(const T *definitions) {
    if (!definitions) return;
    for (unsigned i = 0; i < 2048 && definitions[i].key; ++i) {
        auto &d = definitions[i];
        const char *value = d.default_value ? d.default_value : d.values[0].value;
        if (value) active->options.emplace(d.key, value);
    }
}
static bool environment(unsigned command, void *data) {
    if (!active) return false;
    switch (command) {
    case RETRO_ENVIRONMENT_GET_CAN_DUPE: *static_cast<bool*>(data) = true; return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
        unsigned format = *static_cast<unsigned*>(data);
        if (format > RETRO_PIXEL_FORMAT_RGB565) return false;
        active->pixel_format = format; return true;
    }
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: *static_cast<const char**>(data) = active->system.c_str(); return true;
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *static_cast<const char**>(data) = active->save.c_str(); return true;
    case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: *static_cast<const char**>(data) = active->core.c_str(); return true;
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: static_cast<retro_log_callback*>(data)->log = log_message; return true;
    case RETRO_ENVIRONMENT_GET_LANGUAGE: *static_cast<unsigned*>(data) = RETRO_LANGUAGE_ENGLISH; return true;
    case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS: *static_cast<unsigned*>(data) = 1; return true;
    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: return true;
    case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: *static_cast<int*>(data) = 3; return true;
    case RETRO_ENVIRONMENT_GET_FASTFORWARDING: *static_cast<bool*>(data) = false; return true;
    case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE: *static_cast<float*>(data) = active->fps; return true;
    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: *static_cast<unsigned*>(data) = 2; return true;
    case RETRO_ENVIRONMENT_SET_VARIABLES: {
        auto vars = static_cast<const retro_variable*>(data);
        for (unsigned i = 0; vars && i < 2048 && vars[i].key; ++i) {
            std::string desc = vars[i].value ? vars[i].value : "";
            auto start = desc.find("; ");
            if (start != std::string::npos) {
                auto value = desc.substr(start + 2); value = value.substr(0, value.find('|'));
                active->options.emplace(vars[i].key, value);
            }
        }
        return true;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        auto var = static_cast<retro_variable*>(data);
        auto found = active->options.find(var->key ? var->key : "");
        var->value = found == active->options.end() ? nullptr : found->second.c_str();
        return var->value != nullptr;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *static_cast<bool*>(data) = false; return true;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: defaults(static_cast<retro_core_option_definition*>(data)); return true;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: defaults(static_cast<retro_core_options_intl*>(data)->us); return true;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2: defaults(static_cast<retro_core_options_v2*>(data)->definitions); return true;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL: {
        auto intl = static_cast<retro_core_options_v2_intl*>(data);
        if (intl->us) defaults(intl->us->definitions);
        return true;
    }
    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
        auto info = static_cast<retro_system_av_info*>(data);
        if (!valid_av(*info)) return false;
        if (active->rate != info->timing.sample_rate) {
            active->phase = active->sum_left = active->sum_right = active->accumulated = 0;
            active->have_previous = false;
        }
        active->fps = info->timing.fps; active->rate = info->timing.sample_rate;
        return true;
    }
    case RETRO_ENVIRONMENT_SET_GEOMETRY: return true; // The video callback supplies actual dimensions.
    case RETRO_ENVIRONMENT_SHUTDOWN: active->shutdown = true; return true;
    case RETRO_ENVIRONMENT_SET_MESSAGE: return true;
    case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
    case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: return true;
    default: return false; // Hardware rendering and optional interfaces are not advertised.
    }
}
static void video(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!active || !data) return; // NULL duplicates the previous frame.
    if (data == RETRO_HW_FRAME_BUFFER_VALID || !width || !height || width > 1024 || height > 1024) {
        active->error = "This core requires unsupported video output."; return;
    }
    unsigned bytes = active->pixel_format == RETRO_PIXEL_FORMAT_XRGB8888 ? 4 : 2;
    if (pitch < width * bytes || pitch > 1024 * 16) { active->error = "Invalid core video stride."; return; }
    active->width = width; active->height = height; active->pixels.resize(width * height);
    for (unsigned y = 0; y < height; ++y) {
        auto row = static_cast<const uint8_t*>(data) + y * pitch;
        for (unsigned x = 0; x < width; ++x) {
            uint32_t color;
            if (bytes == 4) { memcpy(&color, row + x * 4, 4); color |= 0xff000000; }
            else {
                uint16_t value; memcpy(&value, row + x * 2, 2);
                unsigned r, g, b = value & 31;
                if (active->pixel_format == RETRO_PIXEL_FORMAT_RGB565) {
                    r = (value >> 11) & 31; g = (value >> 5) & 63; g = (g << 2) | (g >> 4);
                } else { r = (value >> 10) & 31; g = (value >> 5) & 31; g = (g << 3) | (g >> 2); }
                r = (r << 3) | (r >> 2); b = (b << 3) | (b >> 2);
                color = 0xff000000 | (r << 16) | (g << 8) | b;
            }
            active->pixels[y * width + x] = color;
        }
    }
}
static size_t audio_batch(const int16_t *samples, size_t frames) {
    if (active && samples && frames <= 524288 && active->audio.size() + frames * 2 <= 1048576)
        active->audio.insert(active->audio.end(), samples, samples + frames * 2);
    else if (active) active->error = "The core produced too much audio in one frame.";
    return frames;
}
static void audio_sample(int16_t left, int16_t right) { int16_t samples[2] = {left, right}; audio_batch(samples, 1); }
static void poll() {}
static int16_t input(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (!active || port || (device & RETRO_DEVICE_MASK) != RETRO_DEVICE_JOYPAD || index) return 0;
    const unsigned mapping[16] = {2,2048,4,8,64,128,32,16,1,1024,512,256,0,0,0,0};
    unsigned mask = 0;
    for (unsigned i = 0; i < 16; ++i) if (active->keys & mapping[i]) mask |= 1u << i;
    return id == RETRO_DEVICE_ID_JOYPAD_MASK ? static_cast<int16_t>(mask) : (id < 16 && (mask & (1u << id)) ? 1 : 0);
}
static void memory_file(Host *h, unsigned id, const char *name, bool write) {
    size_t size = h->get_memory_size(id); void *memory = h->get_memory_data(id);
    if (!memory || !size || size > 32 * 1024 * 1024) return;
    std::string path = h->save + "/" + name;
    if (write) {
        std::ofstream file(path + ".tmp", std::ios::binary | std::ios::trunc);
        file.write(static_cast<const char*>(memory), size); file.close();
        if (!file || std::rename((path + ".tmp").c_str(), path.c_str())) h->error = "Could not write battery save.";
    } else {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (file && file.tellg() == static_cast<std::streamoff>(size)) {
            file.seekg(0); file.read(static_cast<char*>(memory), size);
        }
    }
}
extern "C" {
const char *oe_error() { return active && !active->error.empty() ? active->error.c_str() : last_error.c_str(); }
void oe_flush(void *instance) {
    auto h = static_cast<Host*>(instance);
    if (h && h->loaded) { memory_file(h, RETRO_MEMORY_SAVE_RAM, "battery.srm", true); memory_file(h, RETRO_MEMORY_RTC, "clock.rtc", true); }
}
void oe_destroy(void *instance) {
    auto h = static_cast<Host*>(instance); if (!h) return;
    if (h->loaded) { oe_flush(h); h->unload_game(); }
    if (h->initialized) h->deinit();
    if (h->module) dlclose(h->module);
    if (active == h) active = nullptr;
    delete h;
}
void *oe_libretro_create(const char *core, const char *rom, const char *save, const char *system) {
    if (active) { last_error = "A Libretro core is already running in this process."; return nullptr; }
    auto h = new Host(); active = h; last_error.clear();
    h->core = core; h->rom = rom; h->save = save; h->system = system;
    h->module = dlopen(core, RTLD_NOW | RTLD_LOCAL);
    if (!h->module) { last_error = dlerror(); oe_destroy(h); return nullptr; }
#define LOAD(name) h->name = reinterpret_cast<decltype(h->name)>(dlsym(h->module, "retro_" #name)); if (!h->name) { last_error = "Missing Libretro function: " #name; oe_destroy(h); return nullptr; }
    LOAD(init); LOAD(deinit); LOAD(api_version); LOAD(get_system_info); LOAD(get_system_av_info);
    LOAD(set_environment); LOAD(set_video_refresh); LOAD(set_audio_sample); LOAD(set_audio_sample_batch);
    LOAD(set_input_poll); LOAD(set_input_state); LOAD(set_controller_port_device); LOAD(load_game);
    LOAD(unload_game); LOAD(run); LOAD(reset); LOAD(serialize_size); LOAD(serialize); LOAD(unserialize);
    LOAD(get_memory_data); LOAD(get_memory_size);
#undef LOAD
    if (h->api_version() != RETRO_API_VERSION) { last_error = "Unsupported Libretro API version."; oe_destroy(h); return nullptr; }
    h->set_environment(environment); h->set_video_refresh(video); h->set_audio_sample(audio_sample);
    h->set_audio_sample_batch(audio_batch); h->set_input_poll(poll); h->set_input_state(input);
    h->init(); h->initialized = true;
    retro_system_info info{}; h->get_system_info(&info);
    if (!info.need_fullpath) {
        std::ifstream file(rom, std::ios::binary);
        h->content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    retro_game_info game{h->rom.c_str(), info.need_fullpath ? nullptr : h->content.data(), info.need_fullpath ? 0 : h->content.size(), nullptr};
    if (!h->load_game(&game)) { last_error = "The Libretro core could not load this ROM. Check its format and any required BIOS files."; oe_destroy(h); return nullptr; }
    h->loaded = true;
    h->set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    retro_system_av_info av{}; h->get_system_av_info(&av);
    if (!valid_av(av)) {
        last_error = "Unsupported core video dimensions or audio timing: " + std::to_string(av.geometry.base_width) + "x" +
            std::to_string(av.geometry.base_height) + ", " + std::to_string(av.timing.fps) + " fps, " + std::to_string(av.timing.sample_rate) + " Hz.";
        oe_destroy(h); return nullptr;
    }
    h->width = av.geometry.base_width; h->height = av.geometry.base_height;
    h->fps = av.timing.fps; h->rate = av.timing.sample_rate; h->pixels.resize(h->width * h->height, 0xff000000);
    memory_file(h, RETRO_MEMORY_SAVE_RAM, "battery.srm", false); memory_file(h, RETRO_MEMORY_RTC, "clock.rtc", false);
    return h;
}
void *oe_create(const char*, const char*) { last_error = "Select a Libretro core file first."; return nullptr; }
void oe_reset(void *h) { static_cast<Host*>(h)->reset(); }
int oe_frame(void *instance, unsigned keys) {
    auto h = static_cast<Host*>(instance); h->keys = keys; h->audio.clear(); h->run();
    if (!h->error.empty() || h->shutdown) return -1;
    // Convert each core's native rate (including SameBoy's MHz-rate audio) to the
    // desktop's 48 kHz stream, preserving fractional position between frames.
    h->output.clear();
    double step = h->rate / 48000.0;
    for (size_t i = 0; i + 1 < h->audio.size(); i += 2) {
        int16_t left = h->audio[i], right = h->audio[i + 1];
        if (step >= 1) {
            // Average the complete source interval instead of discarding
            // high-rate samples, so downsampling also provides a low-pass filter.
            double remaining = 1;
            while (remaining > 1e-9) {
                double part = std::min(remaining, step - h->accumulated);
                h->sum_left += left * part; h->sum_right += right * part;
                h->accumulated += part; remaining -= part;
                if (h->accumulated >= step - 1e-9) {
                    h->output.push_back(std::lround(h->sum_left / step));
                    h->output.push_back(std::lround(h->sum_right / step));
                    h->sum_left = h->sum_right = h->accumulated = 0;
                }
            }
            continue;
        }
        if (!h->have_previous) { h->previous_left = left; h->previous_right = right; h->have_previous = true; }
        while (h->phase < 1.0) {
            h->output.push_back(std::lround(h->previous_left + (left - h->previous_left) * h->phase));
            h->output.push_back(std::lround(h->previous_right + (right - h->previous_right) * h->phase));
            h->phase += step;
        }
        h->phase -= 1.0; h->previous_left = left; h->previous_right = right;
    }
    return h->output.size() / 2;
}
const uint32_t *oe_pixels(void *h) { return static_cast<Host*>(h)->pixels.data(); }
const int16_t *oe_audio(void *h) { return static_cast<Host*>(h)->output.data(); }
unsigned oe_width(void *h) { return static_cast<Host*>(h)->width; }
unsigned oe_height(void *h) { return static_cast<Host*>(h)->height; }
double oe_fps(void *h) { return static_cast<Host*>(h)->fps; }
unsigned oe_sample_rate(void*) { return 48000; }
int oe_save_state(void *instance, const char *path) {
    auto h = static_cast<Host*>(instance); size_t size = h->serialize_size();
    if (!size || size > 31 * 1024 * 1024) return 0;
    std::vector<char> state(size); if (!h->serialize(state.data(), size)) return 0;
    std::ofstream file(path, std::ios::binary); file.write(state.data(), size); return file.good();
}
int oe_load_state(void *instance, const char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file || file.tellg() <= 0 || file.tellg() > 31 * 1024 * 1024) return 0;
    size_t size = file.tellg(); file.seekg(0); std::vector<char> state(size); file.read(state.data(), size);
    auto h = static_cast<Host*>(instance);
    bool ok = file.good() && h->unserialize(state.data(), state.size());
    h->audio.clear(); h->output.clear(); h->have_previous = false; h->phase = 0;
    h->sum_left = h->sum_right = h->accumulated = 0; return ok;
}
}
