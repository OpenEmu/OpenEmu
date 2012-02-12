#ifndef __FCEU_VIDEO_H
#define __FCEU_VIDEO_H

int FCEU_InitVirtualVideo(void);
extern uint8 *XBuf;
extern uint8 *XBackBuf;
extern struct GUIMESSAGE
{
	//the current gui message
	char errmsg[110];
} guiMessage;

#endif
