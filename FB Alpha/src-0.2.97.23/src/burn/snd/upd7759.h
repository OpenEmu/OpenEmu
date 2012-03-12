#define UPD7759_STANDARD_CLOCK		640000

typedef void (*drqcallback)(INT32 param);

extern void UPD7759Update(INT32 chip, INT16 *pSoundBuf, INT32 nLength);
extern void UPD7759Reset();
extern void UPD7759Init(INT32 chip, INT32 clock, UINT8* pSoundData);
extern void UPD7759SetDrqCallback(INT32 chip, drqcallback Callback);
extern INT32 UPD7759BusyRead(INT32 chip);
extern void UPD7759ResetWrite(INT32 chip, UINT8 Data);
extern void UPD7759StartWrite(INT32 chip, UINT8 Data);
extern void UPD7759PortWrite(INT32 chip, UINT8 Data);
extern INT32 UPD7759Scan(INT32 chip, INT32 nAction,INT32 *pnMin);
extern void UPD7759Exit();
