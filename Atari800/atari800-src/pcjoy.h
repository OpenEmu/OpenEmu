/* joystick types for DOS port of David Firth's Atari800 emulator */

#define joy_off 0
#define joy_analog 1
#define joy_lpt1 2
#define joy_lpt2 3
#define joy_lpt3 4
#define joy_keyset0 5
#define joy_keyset1 6
#define joy_keyset2 7
#define joy_keyset3 8

#define JOYSTICKTYPES 9

char joyparams[JOYSTICKTYPES][10]={"OFF","PC","LPT1","LPT2",
                       "LPT3","KEYSET_0","KEYSET_1","KEYSET_2","KEYSET_3"};

