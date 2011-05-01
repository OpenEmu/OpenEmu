#ifndef __MDFN_MD_GENIO_H
#define __MDFN_MD_GENIO_H

namespace MDFN_IEN_MD
{

/* Function prototypes */
extern void gen_io_reset(void);
extern void gen_io_w(int offset, int value);
extern int gen_io_r(int offset);

extern void gen_io_update(void);
extern void gen_io_set_device(int which, int type);

void MDIO_Init(bool overseas, bool PAL, bool overseas_reported, bool PAL_reported);

void MDINPUT_Frame(void);
void MDINPUT_SetInput(int port, const char *type, void *ptr);

extern InputInfoStruct MDInputInfo;

class MD_Input_Device
{
	public:
	MD_Input_Device();
	virtual ~MD_Input_Device();
        virtual void Write(uint8 data);
        virtual uint8 Read();
        virtual void Update(const void *data);
	virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);
};

#if 0
typedef struct
{
        void (*Write)(void *pirate, uint8 data);
        uint8 (*Read)(void *pirate);
        void (*Update)(void *pirate, const void *data);
        void *pirate;
} MD_Input_Device;
#endif

int MDINPUT_StateAction(StateMem *sm, int load, int data_only);

}

#endif

