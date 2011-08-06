#ifndef __MDFN_DRIVERS_PROMPT_H
#define __MDFN_DRIVERS_PROMPT_H

class HappyPrompt
{
	public:
	HappyPrompt(const std::string &ptext, const std::string &zestring);
	HappyPrompt();

	virtual ~HappyPrompt();

	void Draw(SDL_Surface *surface, const SDL_Rect *rect);
	void Event(const SDL_Event *event);
	void Init(const std::string &ptext, const std::string &zestring);
	void SetText(const std::string &ptext);
	void SetKBB(const std::string &zestring);

        virtual void TheEnd(const std::string &pstring);


	protected:
        std::string PromptText;

	private:

	std::vector<uint32> kb_buffer;
	unsigned int kb_cursor_pos;
};

#endif
