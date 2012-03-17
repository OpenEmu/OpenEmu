#ifndef GENS_CDDA_MP3_H
#define GENS_CDDA_MP3_H
	
#ifdef __cplusplus
extern "C" {
#endif

int MP3_Init(void);
void MP3_Reset(void);
int MP3_Get_Bitrate(FILE *f);
int MP3_Length_LBA(FILE *f);
int MP3_Find_Frame(FILE *f, int pos_wanted);
int MP3_Play(int track, int lba_pos, int async);
int MP3_Update(char *buf, int *rate, int *channel, unsigned int length_dest);
void MP3_Test(FILE* f);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CDDA_MP3_H */
