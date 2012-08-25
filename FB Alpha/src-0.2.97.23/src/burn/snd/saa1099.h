void saa1099Update(INT32 chip, INT16 *output, INT32 samples);
void saa1099Reset(INT32 chip);
void saa1099Init(INT32 chip, INT32 clock, INT32 bAdd);
void saa1099SetRoute(INT32 chip, INT32 nIndex, double nVolume, INT32 nRouteDir);
void saa1099ControlWrite(INT32 chip, INT32 data);
void saa1099DataWrite(INT32 chip, INT32 data);
void saa1099Scan(INT32 chip, INT32 nAction);
void saa1099Exit(INT32 chip);

#define BURN_SND_SAA1099_ROUTE_1		0
#define BURN_SND_SAA1099_ROUTE_2		1

#define saa1099SetAllRoutes(i, v, d)						\
	saa1099SetRoute(i, BURN_SND_SAA1099_ROUTE_1, v, d);		\
	saa1099SetRoute(i, BURN_SND_SAA1099_ROUTE_2, v, d);
