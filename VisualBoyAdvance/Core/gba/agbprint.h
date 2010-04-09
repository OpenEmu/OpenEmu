#ifndef AGBPRINT_H
#define AGBPRINT_H

void agbPrintEnable(bool enable);
bool agbPrintIsEnabled();
void agbPrintReset();
bool agbPrintWrite(u32 address, u16 value);
void agbPrintFlush();

#endif // AGBPRINT_H
