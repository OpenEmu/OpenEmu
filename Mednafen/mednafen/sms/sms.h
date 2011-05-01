
#ifndef _SMS_H_
#define _SMS_H_

namespace MDFN_IEN_SMS
{

enum {
    SLOT_BIOS   = 0,
    SLOT_CARD   = 1,
    SLOT_CART   = 2,
    SLOT_EXP    = 3
};

enum {
    MAPPER_NONE         = 0,
    MAPPER_SEGA         = 1,
    MAPPER_CODIES       = 2,
    MAPPER_CASTLE	= 3,
};

enum {
    DISPLAY_NTSC        = 0,
    DISPLAY_PAL         = 1
};

enum {
    CLOCK_NTSC        = 3579545,
    CLOCK_PAL         = 3546893
};

enum {
    CONSOLE_SMS         = 0x20,
    CONSOLE_SMSJ        = 0x21,
    CONSOLE_SMS2        = 0x22,

    CONSOLE_GG          = 0x40,
    CONSOLE_GGMS        = 0x41,

    CONSOLE_MD          = 0x80,
    CONSOLE_MDPBC       = 0x81,
    CONSOLE_GEN         = 0x82,
    CONSOLE_GENPBC      = 0x83
};

#define HWTYPE_SMS  CONSOLE_SMS
#define HWTYPE_GG   CONSOLE_GG
#define HWTYPE_MD   CONSOLE_MD

#define IS_SMS      (sms.console & HWTYPE_SMS)
#define IS_GG       (sms.console & HWTYPE_GG)
#define IS_MD       (sms.console & HWTYPE_MD)

enum {
    TERRITORY_DOMESTIC  = 0,
    TERRITORY_EXPORT    = 1
};

/* SMS context */
typedef struct
{
    uint8 wram[0x2000];
    uint8 paused;
    uint8 save;
    uint8 territory;
    uint8 console;
    uint8 display;
    uint8 fm_detect;
    uint8 use_fm;
    uint8 memctrl;
    uint8 ioctrl;
    struct {
        uint8 pdr;      /* Parallel data register */
        uint8 ddr;      /* Data direction register */
        uint8 txdata;   /* Transmit data buffer */
        uint8 rxdata;   /* Receive data buffer */
        uint8 sctrl;    /* Serial mode control and status */
    } sio;
    struct {
        int type;
    } device[2];

    uint32 timestamp;
    int32 cycle_counter;
} sms_t;

/* Global data */
extern sms_t sms;

/* Function prototypes */
uint8 sms_readbyte(uint16 A);
void sms_init(void);
void sms_reset(void);
void sms_shutdown(void);
void sms_mapper_w(int address, int data);
int sms_irq_callback(int param);

}
#endif /* _SMS_H_ */
