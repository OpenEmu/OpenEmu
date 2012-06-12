#ifndef PBI_SCSI_H_
#define PBI_SCSI_H_

#include "atari.h"
#include <stdio.h>

extern int PBI_SCSI_CD;
extern int PBI_SCSI_MSG;
extern int PBI_SCSI_IO;
extern int PBI_SCSI_BSY;
extern int PBI_SCSI_REQ;
extern int PBI_SCSI_SEL;
extern int PBI_SCSI_ACK;
extern FILE *PBI_SCSI_disk;

void PBI_SCSI_PutByte(UBYTE byte);
UBYTE PBI_SCSI_GetByte(void);
void PBI_SCSI_PutSEL(int newsel);
void PBI_SCSI_PutACK(int newack);

#endif /* PBI_MIO_H_ */
