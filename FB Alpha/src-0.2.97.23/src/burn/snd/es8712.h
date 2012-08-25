void es8712Write(INT32 device, INT32 offset, UINT8 data);
void es8712Play(INT32 device);

void es8712SetBankBase(INT32 device, INT32 base);

void es8712Init(INT32 device, UINT8 *rom, INT32 sample_rate, INT32 addSignal);
void es8712SetRoute(INT32 device, double nVolume, INT32 nRouteDir);
void es8712Reset(INT32 device);
void es8712Update(INT32 device, INT16 *buffer, INT32 samples);
void es8712Exit(INT32 device);

INT32 es8712Scan(INT32 device, INT32 nAction);
