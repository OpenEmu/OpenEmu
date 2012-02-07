This is NOT the complete zlib distribution; it is just a subset of the
source needed by the File_Extractor library. I've made some minor
changes:

* Enabled DYNAMIC_CRC_TABLE in zconf.h, to reduce executable size
slightly.
* Made z_stream_s's msg const char* to eliminate many warnings.

You can remove these sources and link to your own copy of zlib if
desired.
