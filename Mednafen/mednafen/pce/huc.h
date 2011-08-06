namespace MDFN_IEN_PCE
{

typedef enum
{
 SYSCARD_NONE = 0,
 SYSCARD_1,
 SYSCARD_2,
 SYSCARD_3,
 SYSCARD_ARCADE		// 3.0 + extras
} SysCardType;

int HuCLoad(const uint8 *data, uint32 len, uint32 crc32, bool DisableBRAM = false, SysCardType syscard = SYSCARD_NONE);

void HuCClose(void);
int HuC_StateAction(StateMem *sm, int load, int data_only);

void HuC_Power(void);

DECLFR(PCE_ACRead);
DECLFW(PCE_ACWrite);

extern bool PCE_IsCD;
extern bool IsTsushin;

};
