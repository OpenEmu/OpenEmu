void saa1099Update(INT32 chip, INT16 *output, INT32 samples);
void saa1099Reset(INT32 chip);
void saa1099Init(INT32 chip, INT32 clock, INT32 volume, INT32 bAdd); // volume of 100.0% is 1000
void saa1099ControlWrite(INT32 chip, INT32 data);
void saa1099DataWrite(INT32 chip, INT32 data);
void saa1099Scan(INT32 chip, INT32 nAction);
void saa1099Exit(INT32 chip);
