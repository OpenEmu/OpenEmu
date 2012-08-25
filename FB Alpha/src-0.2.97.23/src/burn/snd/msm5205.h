
#define MSM5205_S96_3B 0     /* prescaler 1/96(4KHz) , data 3bit */
#define MSM5205_S48_3B 1     /* prescaler 1/48(8KHz) , data 3bit */
#define MSM5205_S64_3B 2     /* prescaler 1/64(6KHz) , data 3bit */
#define MSM5205_SEX_3B 3     /* VCLK slave mode      , data 3bit */

#define MSM5205_S96_4B 4     /* prescaler 1/96(4KHz) , data 4bit */
#define MSM5205_S48_4B 5     /* prescaler 1/48(8KHz) , data 4bit */
#define MSM5205_S64_4B 6     /* prescaler 1/64(6KHz) , data 4bit */
#define MSM5205_SEX_4B 7     /* VCLK slave mode      , data 4bit */

#define MSM6585_S160  (4+8)  /* prescaler 1/160(4KHz), data 4bit */
#define MSM6585_S40   (5+8)  /* prescaler 1/40(16KHz), data 4bit */
#define MSM6585_S80   (6+8)  /* prescaler 1/80 (8KHz), data 4bit */
#define MSM6585_S20   (7+8)  /* prescaler 1/20(32KHz), data 4bit */

void MSM5205ResetWrite(INT32 chip, INT32 reset);
void MSM5205DataWrite(INT32 chip, INT32 data);
void MSM5205VCLKWrite(INT32 chip, INT32 reset);
void MSM5205PlaymodeWrite(INT32 chip, INT32 select);

void MSM5205Init(INT32 chip, INT32 (*stream_sync)(INT32), INT32 clock, void (*vclk_callback)(), INT32 select, INT32 bAdd);
void MSM5205SetRoute(INT32 chip, double nVolume, INT32 nRouteDir);
void MSM5205SetLeftVolume(INT32 chip, double nLeftVolume);
void MSM5205SetRightVolume(INT32 chip, double nRightVolume);
void MSM5205SetSeperateVolumes(INT32 chip, INT32 state);
void MSM5205Reset();

/*
	All MSM5205 sounds need to be rendered in one call
	or the data will likely be output badly
*/

void MSM5205Render(INT32 chip, INT16 *buffer, INT32 len);
void MSM5205Exit();

void MSM5205Scan(INT32 nAction, INT32 *pnMin);

/*
	MSM5205CalcInterleave is used to calculate after how
	many cycles that the sound cpu has ran does
	MSM5205Update need to be called. It returns how many
	slices must be made of the sound cpu's cycles
*/	

INT32 MSM5205CalcInterleave(INT32 chip, INT32 cpu_speed);
void MSM5205Update();
