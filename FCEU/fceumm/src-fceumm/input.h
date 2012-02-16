typedef struct {
  uint8 (*Read)(int w);
  void (*Write)(uint8 v);
  void (*Strobe)(int w);
  void (*Update)(int w, void *data, int arg);
  void (*SLHook)(int w, uint8 *bg, uint8 *spr, uint32 linets, int final);
  void (*Draw)(int w, uint8 *buf, int arg);
} INPUTC;

typedef struct {
  uint8 (*Read)(int w, uint8 ret);
  void (*Write)(uint8 v);
  void (*Strobe)(void);
  void (*Update)(void *data, int arg);
  void (*SLHook)(uint8 *bg, uint8 *spr, uint32 linets, int final);
  void (*Draw)(uint8 *buf, int arg);
} INPUTCFC;

void FCEU_DrawInput(uint8 *buf);
void FCEU_UpdateInput(void);
void InitializeInput(void);
extern void (*PStrobe[2])(void);
extern void (*InputScanlineHook)(uint8 *bg, uint8 *spr, uint32 linets, int final);
