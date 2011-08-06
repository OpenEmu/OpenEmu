static const char bsnesVersion[] = "0.059";
static const char bsnesTitle[] = "bsnes";
static const unsigned bsnesSerializerVersion = 4;

//S-DSP can be encapsulated into a state machine using #define magic
//this avoids ~2.048m co_switch() calls per second (~5% speedup)
#define DSP_STATE_MACHINE

//game genie + pro action replay code support (~2% speed hit)
#define CHEAT_SYSTEM

//enable debugging extensions (~15% speed hit)
//#define DEBUGGER

#include <libco/libco.h>

#include <nall/algorithm.hpp>
#include <nall/any.hpp>
#include <nall/array.hpp>
#include <nall/bit.hpp>
#include <nall/detect.hpp>
#include <nall/dl.hpp>
#include <nall/endian.hpp>
#include <nall/file.hpp>
#include <nall/function.hpp>
#include <nall/moduloarray.hpp>
#include <nall/platform.hpp>
#include <nall/priorityqueue.hpp>
#include <nall/property.hpp>
#include <nall/serializer.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>
#include <nall/vector.hpp>
using namespace nall;

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#include "interface.hpp"
