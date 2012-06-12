DEFS = @DEFS@ @CFLAG_VISIBILITY@
INCLUDES = -I./

noinst_LIBRARIES = libvorbisidec.a

libvorbisidec_a_SOURCES = mdct.c block.c window.c \
                        synthesis.c info.c \
                        floor1.c floor0.c vorbisfile.c \
                        res012.c mapping0.c registry.c codebook.c \
			sharedbook.c framing.c bitwise.c \
                        codebook.h misc.h mdct_lookup.h\
                        os.h mdct.h block.h ivorbisfile.h lsp_lookup.h\
                        registry.h window.h window_lookup.h\
                        codec_internal.h backends.h ogg.h \
			asm_arm.h ivorbiscodec.h
