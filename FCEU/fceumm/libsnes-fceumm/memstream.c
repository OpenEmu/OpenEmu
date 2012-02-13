#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "memstream.h"

static uint8_t *g_buffer = NULL;
static size_t g_size = 0;
static size_t g_last_file_size = 0;

struct memstream
{
   uint8_t *m_buf;
   size_t m_size;
   size_t m_ptr;
   size_t m_max_ptr;
   bool writing;
};

static void memstream_update_ptr(memstream_t *stream)
{
   if (stream->m_ptr > stream->m_max_ptr)
      stream->m_max_ptr = stream->m_ptr;
}

static void memstream_init(memstream_t *stream, uint8_t *buffer, size_t max_size, bool writing)
{
	stream->m_buf = buffer;
	stream->m_size = max_size;
	stream->m_ptr = 0;
	stream->m_max_ptr = 0;
   stream->writing = writing;
}

void memstream_set_buffer(uint8_t *buffer, size_t size)
{
   g_buffer = buffer;
   g_size = size;
}

size_t memstream_get_last_size(void)
{
   return g_last_file_size;
}

memstream_t *memstream_open(bool writing)
{
   memstream_t *stream;
   if (!g_buffer || !g_size)
      return NULL;

   stream = (memstream_t*)calloc(1, sizeof(*stream));
   memstream_init(stream, g_buffer, g_size, writing);
   g_buffer = NULL;
   g_size = 0;
   return stream;
}

void memstream_close(memstream_t *stream)
{
   g_last_file_size = stream->writing ? stream->m_max_ptr : stream->m_size;
   free(stream);
}

size_t memstream_read(memstream_t *stream, void *data, size_t bytes)
{
   size_t avail = stream->m_size - stream->m_ptr;
   if (bytes > avail)
      bytes = avail;

   memcpy(data, stream->m_buf + stream->m_ptr, bytes);
   stream->m_ptr += bytes;
   memstream_update_ptr(stream);
   return bytes;
}

size_t memstream_write(memstream_t *stream, const void *data, size_t bytes)
{
   size_t avail = stream->m_size - stream->m_ptr;
   if (bytes > avail)
      bytes = avail;

   memcpy(stream->m_buf + stream->m_ptr, data, bytes);
   stream->m_ptr += bytes;
   memstream_update_ptr(stream);
   return bytes;
}

int memstream_seek(memstream_t *stream, int offset, int whence)
{
   size_t ptr;
   if (whence == SEEK_SET)
      ptr = offset;
   else if (whence == SEEK_CUR)
      ptr = stream->m_ptr + offset;
   else if (whence == SEEK_END)
      ptr = (stream->writing ? stream->m_max_ptr : stream->m_size) + offset;
   else
      return -1;

   if (ptr <= stream->m_size)
   {
      stream->m_ptr = ptr;
      return 0;
   }
   else
      return -1;
}

size_t memstream_pos(memstream_t *stream)
{
   return stream->m_ptr;
}

int memstream_getc(memstream_t *stream)
{
   if (stream->m_ptr >= stream->m_size)
      return EOF;
   else
      return stream->m_buf[stream->m_ptr++];

   memstream_update_ptr(stream);
}

void memstream_putc(memstream_t *stream, int c)
{
   if (stream->m_ptr < stream->m_size)
      stream->m_buf[stream->m_ptr++] = c;

   memstream_update_ptr(stream);
}

