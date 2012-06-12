/******************************************************************************
*
* FILENAME: decoration.hpp
*
* DESCRIPTION:  This class will show a boxed decoration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/24/06  bberlin      Creation
******************************************************************************/
#ifndef decoration_hpp
#define decoration_hpp

#include <guichan.hpp>

class Decoration : public gcn::Widget
{
	public:
		Decoration (const std::string& caption="", int type=1);
		~Decoration();
		void setType(int type);
		void setCaption(const std::string& caption);
		void drawFrame(gcn::Graphics* graphics);
		void draw(gcn::Graphics* graphics);

        enum
        {
            DIV_TYPE_RECESSED = 0,
            DIV_TYPE_LINE,
            DIV_TYPE_RAISED
        };
	private:
		std::string mText;
		int mType;
};

#endif
