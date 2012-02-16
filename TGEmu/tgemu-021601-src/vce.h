
#ifndef _VCE_H_
#define _VCE_H_

/* Macros and defines */
typedef struct
{
    uint8 ctrl;
    uint8 data[0x400];
    uint16 addr;
}t_vce;

/* Global data */
extern t_vce vce;

/* Function prototypes */
void vce_w(int address, int data);
int vce_r(int address);

#endif /* _VCE_H_ */
