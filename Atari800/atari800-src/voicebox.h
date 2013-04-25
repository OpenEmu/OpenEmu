#ifndef VOICEBOX_H_
#define VOICEBOX_H_

int VOICEBOX_Initialise(int *argc, char *argv[]);
void VOICEBOX_SKCTLPutByte(int byte);
void VOICEBOX_SEROUTPutByte(int byte);
extern int VOICEBOX_enabled;
extern int VOICEBOX_ii;
#define VOICEBOX_BASEAUDF 0xa0

#endif /* VOICEBOX_H_ */
