int DTestButton(std::vector<ButtConfig> &bc, const char *KeyState, const uint32* MouseData);
int DTestButton(ButtConfig &bc, const char *KeyState, const uint32 *MouseData);

int DTestButtonCombo(std::vector<ButtConfig> &bc, const char *KeyState);
int DTestButtonCombo(ButtConfig &bc, const char *KeyState);

int ButtonConfigBegin(void);
void ButtonConfigEnd(void);

int DTryButtonBegin(ButtConfig *bc, int commandkey);
int DTryButton(void);
int DTryButtonEnd(ButtConfig *bc);

