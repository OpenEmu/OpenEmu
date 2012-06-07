include_directories(${libmpc_SOURCE_DIR}/include)
if(SHARED)
  add_library(mpcdec SHARED huffman mpc_decoder mpc_reader streaminfo mpc_bits_reader mpc_demux requant synth_filter ${libmpc_SOURCE_DIR}/common/crc32)
else(SHARED)
  add_library(mpcdec_static STATIC huffman mpc_decoder mpc_reader streaminfo mpc_bits_reader mpc_demux requant synth_filter ${libmpc_SOURCE_DIR}/common/crc32)
endif(SHARED)

