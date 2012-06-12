#ifndef COMPFILE_H_
#define COMPFILE_H_

#include <stdio.h>  /* FILE */

int CompFile_ExtractGZ(const char *infilename, FILE *outfp);
int CompFile_DCMtoATR(FILE *infp, FILE *outfp);

#endif /* COMPFILE_H_ */
