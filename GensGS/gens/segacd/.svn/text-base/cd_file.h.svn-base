#ifndef GENS_CD_FILE_H
#define GENS_CD_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_ISO 1
#define TYPE_BIN 2
#define TYPE_MP3 3
#define TYPE_WAV 4


struct _file_track {
	FILE *F;
	FILE *F_decoded;
	int Length;
	int Type;
	char filename [512];
};

extern struct _file_track Tracks[100];

extern char cp_buf[2560];
extern char Track_Played;


int FILE_Init(void);
void FILE_End(void);
int Load_ISO(char* buf, const char* iso_name);
void Unload_ISO(void);
//int FILE_Read_One_CD_LBA(int lba);
int FILE_Read_One_LBA_CDC(void);
int FILE_Play_CD_LBA(int async);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CD_FILE_H */
