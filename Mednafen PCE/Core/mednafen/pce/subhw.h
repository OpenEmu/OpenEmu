namespace MDFN_IEN_PCE
{
void SubHW_Init(void);
void SubHW_Kill(void);

void SubHW_WriteIOPage(uint32 A, uint8 V);
uint8 SubHW_ReadIOPage(uint32 A);

DECLFW(SubHW_WriteFEPage);
DECLFR(SubHW_ReadFEPage);

void SubHW_Power(void);

void SubHW_EndFrame(const int32 timestamp);
void SubHW_CVSync(void);
bool SubHW_DisplayLine(uint16 *out_buffer);

int SubHW_StateAction(StateMem *sm, int load, int data_only);
}
