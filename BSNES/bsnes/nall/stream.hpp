#ifndef NALL_STREAM_HPP
#define NALL_STREAM_HPP

#include <algorithm>
#include <memory>

#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/gzip.hpp>
#include <nall/http.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/zip.hpp>

#define NALL_STREAM_INTERNAL_HPP
#include <nall/stream/stream.hpp>
#include <nall/stream/memory.hpp>
#include <nall/stream/mmap.hpp>
#include <nall/stream/file.hpp>
#include <nall/stream/http.hpp>
#include <nall/stream/gzip.hpp>
#include <nall/stream/zip.hpp>
#include <nall/stream/auto.hpp>
#undef NALL_STREAM_INTERNAL_HPP

#endif
