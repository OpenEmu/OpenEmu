/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

//#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

//#include "devlegcy.h"

typedef void (*irq_callback)(INT32 param);
typedef UINT16(port_read)();

typedef struct _es5505_interface es5505_interface;
struct _es5505_interface
{
	const INT8 * region0;						/* memory region where the sample ROM lives */
	const INT8 * region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(INT32 state);	/* irq callback */
	UINT16 (*read_port)();			/* input port read */
};

//READ16_DEVICE_HANDLER( es5505_r );
//WRITE16_DEVICE_HANDLER( es5505_w );
//void es5505_voice_bank_w(device_t *device, int voice, int bank);

//DECLARE_LEGACY_SOUND_DEVICE(ES5505, es5505);


typedef struct _es5506_interface es5506_interface;
struct _es5506_interface
{
	const INT8 * region0;						/* memory region where the sample ROM lives */
	const INT8 * region1;						/* memory region where the sample ROM lives */
	const INT8 * region2;						/* memory region where the sample ROM lives */
	const INT8 * region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(INT32 state);	/* irq callback */
	UINT16 (*read_port)();			/* input port read */
};

//READ8_DEVICE_HANDLER( es5506_r );
//WRITE8_DEVICE_HANDLER( es5506_w );
//void es5506_voice_bank_w(device_t *device, int voice, int bank);

//DECLARE_LEGACY_SOUND_DEVICE(ES5506, es5506);

void ES5506Update(INT16 *pBuffer, INT32 samples);
#define ES5505Update ES5506Update
void ES5506Init(INT32 clock, UINT8 *region0, UINT8 *region1, UINT8 *region2, UINT8 *region3, irq_callback callback);
void ES5506Exit();
void ES5506Reset();
void ES5506Write(UINT32 offset, UINT8 data);
UINT8 ES5506Read(UINT32 offset);
void es5506_voice_bank_w(INT32 voice, INT32 bank);
void ES5505Init(INT32 clock, UINT8 *region0, UINT8* region1, irq_callback callback);
#define ES5505Reset	ES5506Reset
#define ES5505Exit	ES5505Exit
void ES5505Write(UINT32 offset, UINT16 data);
UINT16 ES5505Read(UINT32 offset);
void es5505_voice_bank_w(INT32 voice, INT32 bank);

#endif /* __ES5506_H__ */
