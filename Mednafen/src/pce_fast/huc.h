namespace PCE_Fast
{

int HuCLoad(const uint8 *data, uint32 len, uint32 crc32);
int HuCLoadCD(const char *bios_path);
void HuCClose(void);
int HuC_StateAction(StateMem *sm, int load, int data_only);

void HuC_Power(void);

DECLFR(PCE_ACRead);
DECLFW(PCE_ACWrite);

extern bool PCE_IsCD;
extern bool IsTsushin;

};
