#ifndef __MDFN_PSX_SIO_H
#define __MDFN_PSX_SIO_H

namespace MDFN_IEN_PSX
{

void SIO_Write(pscpu_timestamp_t timestamp, uint32 A, uint32 V);
uint32 SIO_Read(pscpu_timestamp_t timestamp, uint32 A);
void SIO_Power(void);

}

#endif
