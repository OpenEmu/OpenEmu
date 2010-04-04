int DTestButton(std::vector<ButtConfig> &bc, char *KeyState, const uint32* MouseData);
int DTestButton(ButtConfig &bc, char *KeyState, const uint32 *MouseData);

int DTestButtonCombo(std::vector<ButtConfig> &bc, char *KeyState);
int DTestButtonCombo(ButtConfig &bc, char *KeyState);

int ButtonConfigBegin(void);
void ButtonConfigEnd(void);

int DTryButtonBegin(ButtConfig *bc, int commandkey);
int DTryButton(void);
int DTryButtonEnd(ButtConfig *bc);

