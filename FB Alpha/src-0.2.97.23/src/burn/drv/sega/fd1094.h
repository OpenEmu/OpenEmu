#define FD1094_STATE_RESET	0x0100
#define FD1094_STATE_IRQ	0x0200
#define FD1094_STATE_RTE	0x0300

INT32 fd1094_set_state(UINT8 *key, INT32 state);
INT32 fd1094_decode(INT32 address, INT32 val, UINT8 *key, INT32 vector_fetch);
