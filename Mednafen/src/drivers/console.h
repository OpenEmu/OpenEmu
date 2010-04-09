#include <vector>

class MDFNConsole
{
	public:
	MDFNConsole(bool shellstyle = 0, bool SmallFont = 0);
	~MDFNConsole();

	void ShowPrompt(bool shown);
	void Draw(SDL_Surface *surface, const SDL_Rect *src_rect);
	int Event(const SDL_Event *event);
	void WriteLine(UTF8 *text);
	void AppendLastLine(UTF8 *text);
        virtual bool TextHook(UTF8 *text);

	void SetSmallFont(bool newsmallfont) { SmallFont = newsmallfont; }
	void SetShellStyle(bool newsetting) { shellstyle = newsetting; }
	void Scroll(int amount);
	private:
	std::vector<std::string> TextLog;
	std::vector<std::string> kb_buffer;
	unsigned int kb_cursor_pos;
	bool shellstyle;
	bool prompt_visible;
	uint32 Scrolled;
	bool SmallFont;
};
