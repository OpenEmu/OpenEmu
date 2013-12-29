#import "CSHandle.h"

typedef struct
{
	int x,y;
} SWFPoint;

typedef struct
{
	int x,y,width,height;
} SWFRect;

typedef struct
{
	int a00,a01,a02;
	int a10,a11,a12;
} SWFMatrix;

static inline SWFPoint SWFMakePoint(int x,int y) { SWFPoint res={x,y}; return res; }
SWFPoint SWFPointOnLine(SWFPoint a,SWFPoint b,float t);

static inline SWFRect SWFMakeRect(int x,int y,int width,int height) { SWFRect res={x,y,width,height}; return res; }
SWFRect SWFParseRect(CSHandle *fh);
void SWFWriteRect(SWFRect rect,CSHandle *fh);

static inline SWFMatrix SWFMakeMatrix(int a00,int a01,int a02,int a10,int a11,int a12) { SWFMatrix res={a00,a01,a02,a10,a11,a12}; return res; }
static inline SWFMatrix SWFTranslationMatrix(int x,int y) { return SWFMakeMatrix(1<<16,0,x,0,1<<16,y); }
SWFMatrix SWFParseMatrix(CSHandle *fh);
void SWFWriteMatrix(SWFMatrix mtx,CSHandle *fh);
SWFMatrix SWFMultiplyMatrices(SWFMatrix a,SWFMatrix b);
SWFMatrix SWFScalingMatrix(float x_scale,float y_scale);
SWFMatrix SWFRotationMatrix(float degrees);

//SWFMatrix SWFMatrixFromAffineTransform(NSAffineTransform *t);
//NSAffineTransform *SWFAffineTransformFromMatrix(SWFMatrix m);

#define SWFZeroPoint SWFMakePoint(0,0)
#define SWFEmptyRect SWFMakeRect(0,0,0,0)
#define SWFIdentityMatrix SWFTranslationMatrix(0,0)

int SWFCountBits(uint32_t val);
int SWFCountBits2(uint32_t val1,uint32_t val2);
int SWFCountBits4(uint32_t val1,uint32_t val2,uint32_t val3,uint32_t val4);
int SWFCountSignedBits(int32_t val);
int SWFCountSignedBits2(int32_t val1,int32_t val2);
int SWFCountSignedBits4(int32_t val1,int32_t val2,int32_t val3,int32_t val4);
int SWFCountSignedBitsPoint(SWFPoint point);
