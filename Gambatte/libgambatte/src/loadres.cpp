#include "loadres.h"

namespace gambatte {

static char const * to_cstr(LoadRes const loadres) {
	switch (loadres) {
	case LOADRES_BAD_FILE_OR_UNKNOWN_MBC:       return "Bad file or unknown MBC";
	case LOADRES_IO_ERROR:                      return "I/O error";
	case LOADRES_UNSUPPORTED_MBC_HUC3:          return "Unsupported MBC: HuC3";
	case LOADRES_UNSUPPORTED_MBC_TAMA5:         return "Unsupported MBC: Tama5";
	case LOADRES_UNSUPPORTED_MBC_POCKET_CAMERA: return "Unsupported MBC: Pocket Camera";
	case LOADRES_UNSUPPORTED_MBC_MBC4:          return "Unsupported MBC: MBC4";
	case LOADRES_UNSUPPORTED_MBC_MMM01:         return "Unsupported MBC: MMM01";
	case LOADRES_OK:                            return "OK";
	}

	return "";
}

std::string const to_string(LoadRes loadres) { return to_cstr(loadres); }

}
