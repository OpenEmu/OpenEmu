#ifdef NETWORK
int MDFND_NetworkConnect(void);
void DrawNetplayTextBuffer(SDL_Surface *surface, const SDL_Rect *src_rect);
void NetplayText_InMainThread(uint8 *text, bool NetEcho);

int NetplayEventHook(const SDL_Event *event);
int NetplayEventHook_GT(const SDL_Event *event);

void Netplay_ToggleTextView(void);
int Netplay_GetTextView(void);
bool Netplay_IsTextInput(void);
bool Netplay_TryTextExit(void);

extern int MDFNDnetplay;

#endif
