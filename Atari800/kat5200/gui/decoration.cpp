/******************************************************************************
*
* FILENAME: decoration.cpp
*
* DESCRIPTION:  This class will show boxed decoration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   03/28/06  bberlin      Creation
******************************************************************************/
#include "decoration.hpp"

Decoration::Decoration (const std::string& caption, int type)
{
	mType = type;
	mText = caption;
	setFrameSize ( 1 );
}

Decoration::~Decoration ()
{
}

void Decoration::setType(int type)
{
	mType = type;
}

void Decoration::setCaption(const std::string& caption)
{
	mText = caption;
}

void Decoration::draw(gcn::Graphics* graphics)
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
    int alpha = getBaseColor().a;
    int width = getWidth() + getFrameSize() * 2 - 1;
    int height = getHeight() + getFrameSize() * 2 - 1;
    highlightColor = faceColor + 0x303030;
    highlightColor.a = alpha;
    shadowColor = faceColor - 0x303030;
    shadowColor.a = alpha;
	int x_start = getFont()->getWidth("A") - 1;
	int x_stop = x_start + getFont()->getWidth(mText) + 1;
	int y_offset = getFont()->getHeight()/2;

    unsigned int i;
    for (i = 0; i < getFrameSize(); ++i)
    {
		if ( mType == DIV_TYPE_RAISED )
        	graphics->setColor(highlightColor);
		else
	        graphics->setColor(shadowColor);

		// Top Line
		if ( mText.size() > 0 )
		{
	        graphics->drawLine(i,i + y_offset, x_start, i + y_offset );
			graphics->drawLine(x_stop,i + y_offset, width - i, i + y_offset);
		}
		else
		{
			graphics->drawLine(i,i + y_offset, width - i, i + y_offset);
		}

		// Left Line
        graphics->drawLine(i,i + 1 + y_offset, i, height - i - 1);

		if ( mType == DIV_TYPE_RAISED )
	        graphics->setColor(shadowColor);
		else
	        graphics->setColor(shadowColor);

		// Right Line
        graphics->drawLine(width - i - 2,i + 1 + y_offset, width - i - 2, height - i);

		//Bottom Line
        graphics->drawLine(i,height - i - 2, width - i - 1, height - i - 2);
    }
	graphics->drawText ( mText, x_start+1, 1 );
}

void Decoration::drawFrame(gcn::Graphics* graphics)
{
}
