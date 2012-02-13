#ifdef __cplusplus
extern "C" {
#endif

void GetFileBase(const char *f);
extern uint32 uppow2(uint32 n);

const char * FCEU_MakeFName(int type, int id1, const char *cd1);

#ifdef __cplusplus
}
#endif

#define FCEUMKF_STATE  1
#define FCEUMKF_SAV  3
#define FCEUMKF_CHEAT  4
#define FCEUMKF_FDSROM  5
#define FCEUMKF_PALETTE  6
#define FCEUMKF_GGROM  7
#define FCEUMKF_IPS  8
#define FCEUMKF_FDS  9
#define FCEUMKF_NPTEMP  11
#define FCEUCOPY_FAMI 12
