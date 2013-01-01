/*
 * Copyright (c) 2009, 2010 Jochen Keil
 * For license terms, see the file COPYING in this distribution.
 */

extern "C" {
#include "cd.h"

    
    Cd* cue_parse_file(FILE*);
    Cd* cue_parse_string(const char*);
}
