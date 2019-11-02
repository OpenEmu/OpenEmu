#version 450

//  crt-royale-bloom-approx.slang #includes "bloom-functions.h", which checks
//  whether PHOSPHOR_BLOOM_FAKE is defined to decide whether to use:
//  a.) bloom_approx_size_x (default 320.0), or
//  b.) bloom_approx_size_x_for_skip (default 400.0)
//  from "user-cgp-constants.h."  This is used to calculate a blur sigma (which
//  incidentally is only used by some codepaths).  These values often differ:
//  a.) 400x300 is better for the fake-bloom shader.
//  b.) 320x240 can be a better choice for the real bloom shader.
//  The only purpose of this file is to tell bloom-functions.h which to assume.

#define PHOSPHOR_BLOOM_FAKE
#include "crt-royale-bloom-approx.h"
