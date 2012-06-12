/******************************************************************************
*
* FILENAME: guiinputbutton.cpp
*
* DESCRIPTION:  Button inherited from Radio Button and looks like a Button
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/28/06  bberlin      Creation
******************************************************************************/
#include "guiinputbutton.hpp"

GuiInputButton::GuiInputButton ( const std::string &caption, const std::string &group, bool marked )
{
	setCaption(caption);
	setGroup(group);
	setSelected(marked);
	mCtrl = 0;
}

void GuiInputButton::draw(gcn::Graphics* graphics)
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;

	if (isSelected())
	{
		faceColor = faceColor - 0x303030;
		faceColor.a = alpha;
		highlightColor = faceColor - 0x303030;
		highlightColor.a = alpha;
		shadowColor = faceColor + 0x303030;
		shadowColor.a = alpha;
	}
	else
	{
		highlightColor = faceColor + 0x303030;
		highlightColor.a = alpha;
		shadowColor = faceColor - 0x303030;
		shadowColor.a = alpha;
	}

	graphics->setColor(faceColor);
	graphics->fillRectangle(gcn::Rectangle(1, 1, getDimension().width-1, getHeight() - 1));

	graphics->setColor(highlightColor);
	graphics->drawLine(0, 0, getWidth() - 1, 0);
	graphics->drawLine(0, 1, 0, getHeight() - 1);

	graphics->setColor(shadowColor);
	graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
	graphics->drawLine(1, getHeight() - 1, getWidth() - 1, getHeight() - 1);

	graphics->setColor(getForegroundColor());

	int textX;
	int textY = getHeight() / 2 - getFont()->getHeight() / 2;

	//switch (getAlignment())
	//{
	//  case gcn::Graphics::LEFT:
	//	  textX = 4;
	//	  break;
	 // case gcn::Graphics::CENTER:
	//	  textX = getWidth() / 2;
	//	  break;
	 // case gcn::Graphics::RIGHT:
	//	  textX = getWidth() - 4;
	//	  break;
	 // default:
	//	  throw GCN_EXCEPTION("Unknown alignment.");
	//}
	textX = getWidth() / 2;

	graphics->setFont(getFont());

	if (isSelected())
	{
		//graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
		graphics->drawText(getCaption(), textX + 1, textY + 1, gcn::Graphics::CENTER);
	}
	else
	{
		//graphics->drawText(getCaption(), textX, textY, getAlignment());
		graphics->drawText(getCaption(), textX, textY, gcn::Graphics::CENTER);

		if (isFocused())
		{
			graphics->drawRectangle(gcn::Rectangle(2, 2, getWidth() - 4,
			                        getHeight() - 4));
		}
	}
}

void GuiInputButton::drawFrame(gcn::Graphics* graphics)
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

		unsigned int i;
		for (i = 0; i < getFrameSize(); ++i)
		{
			graphics->setColor(shadowColor);
			graphics->drawLine(i,i, width - i, i);
			graphics->drawLine(i,i + 1, i, height - i - 1);
			graphics->setColor(highlightColor);
			graphics->drawLine(width - i,i + 1, width - i, height - i);
			graphics->drawLine(i,height - i, width - i - 1, height - i);
		}
}

void GuiInputButton::adjustSize()
{
	setWidth(getFont()->getWidth(getCaption()) + 8);
	setHeight(getFont()->getHeight() + 8);
}

GuiInputButton *GuiInputButton::getMarkedButton()
{
    if (mGroup != "")
         {
             GroupIterator iter, iterEnd;
             iterEnd = mGroupMap.upper_bound(mGroup);
 
             for (iter = mGroupMap.lower_bound(mGroup);
                  iter != iterEnd;
                  iter++)
             {
                 if (iter->second->isSelected())
                 {
                     return (GuiInputButton *)iter->second;
                 }
             }
         }

	return this;
}
