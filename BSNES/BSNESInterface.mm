#include <base.hpp>
#include "BSNESInterface.h"

#define conv555to565(a) ((((a)&~0x1f)<<1)|((a)&0x1f))

// TODO: Make a LUT for proper SNES pallated conversion from 555 to 565
void BSNESInterface::video_refresh(uint16_t *input, unsigned apitch, unsigned *line, unsigned awidth, unsigned aheight) {
	
	width = awidth;
	height = aheight;
	pitch = apitch;
	
	
	pitch >>= 1;
	
	uint16_t *output = video;
	for(unsigned y = 0; y < height; y++) 
	{
		if(width == 512 && line[y] == 256) 
		{
			for(unsigned x = 0; x < 256; x++) 
			{
				uint16_t p = *input++;
				p = conv555to565(p);
				*output++ = p;
				*output++ = p;
			}
			input += 256;
		} 
		else 
		{
			for(unsigned x = 0; x < width; x++) 
			{
				uint16_t p = *input++;
				p = conv555to565(p);
				*output++ = p;
			}
		}
		input  += pitch - width;
		output += pitch - width;
	}
}

void BSNESInterface::audio_sample(uint16_t left, uint16_t right) {
 // if(config.audio.mute) left = right = 0;
 // audio.sample(left, right);
	//printf("sampled");
	
	[ringBuffer write:&left maxLength:2];
	[ringBuffer write:&right maxLength:2];
}

void BSNESInterface::input_poll() {
 // inputManager.poll();
}

int16_t BSNESInterface::input_poll(unsigned deviceid, unsigned a) {
	//NSLog(@"polled input: device: %d id: %d", deviceid, id);
	if (deviceid == SNES::Input::DeviceIDJoypad1) {
		return pad[0][a];
	}
	else if(deviceid == SNES::Input::DeviceIDJoypad2) {
		return pad[1][a];	
	}
	
	return 0;//inputManager.getStatus(deviceid, id);
}
