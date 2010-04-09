/*
 *  BSNESInterface.h
 *  BSNES
 *
 *  Created by Joshua Weinberg on 7/20/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

//#include <base.hpp>
#include "system/interface/interface.hpp"
#import "OERingBuffer.h"

class BSNESInterface : public SNES::Interface {
public:
	void video_refresh(uint16_t *data, unsigned pitch, unsigned *line, unsigned width, unsigned height);
	void audio_sample(uint16_t left, uint16_t right);
	void input_poll();
	int16_t input_poll(unsigned deviceid, unsigned id);
	OERingBuffer *ringBuffer;
	int width;
	int height;
	int pitch;
	int16_t pad[2][12];
	uint16_t *video;
};
