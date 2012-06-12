mednafen_SOURCES	+=	cdrom/audioreader.cpp cdrom/cdromif.cpp cdrom/scsicd.cpp cdrom/pcecd.cpp
mednafen_SOURCES	+=	cdrom/CDUtility.cpp cdrom/crc32.cpp cdrom/galois.cpp cdrom/l-ec.cpp cdrom/recover-raw.cpp
mednafen_SOURCES	+=	cdrom/lec.cpp cdrom/CDAccess.cpp cdrom/CDAccess_Image.cpp

if HAVE_LIBCDIO
mednafen_SOURCES	+=	cdrom/CDAccess_Physical.cpp
endif
