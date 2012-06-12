/******************************************************************************
*
* FILENAME: menutop.cpp
*
* DESCRIPTION:  Class used with the Menu Bar, represents the top button
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/20/07  bberlin      Creation, redo from 0.5.0
******************************************************************************/
#include "menutop.hpp"
#include "menubar.hpp"

MenuTop::MenuTop(MenuBar *menu_bar, std::string Id, std::string Title, gcn::ActionListener *actionListener )
{
	mDown = false;

	mMenuBar = menu_bar;

	mMenuTopItem = new MenuItem ( Id, Title, actionListener, menu_bar );

	setCaption ( Title );
}

MenuTop::~MenuTop()
{
	delete mMenuTopItem;
}

bool MenuTop::isDown()
{
	if ( ( (mHasMouse == true) || (mMenuTopItem->hasMouse()==true) || 
	     (mMenuTopItem->childrenHaveMouse()==true) ) && (mDown == true) )
		return true;
	else
		return false;
}

void MenuTop::setDown(bool down)
{
	mDown = down;
}

void MenuTop::draw(gcn::Graphics* graphics)
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
    int alpha = getBaseColor().a;
    
    if (isDown())
    {
		mMenuTopItem->setVisible ( true );
		mMenuTopItem->requestMoveToTop();
        //faceColor = faceColor + 0x303030;
        //faceColor.a = alpha;
        highlightColor = faceColor - 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor + 0x303030;      
        shadowColor.a = alpha;
    }
    else
    {
		if ( mDown == false )
			mMenuTopItem->setVisible ( false );
		else if ( mMenuTopItem->hasMouse()  )
			mMenuTopItem->setVisible ( true );
		else if ( mMenuTopItem->childrenHaveMouse()  )
			mMenuTopItem->setVisible ( true );
		else
			mMenuTopItem->setVisible ( false );

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
    
    switch (getAlignment())
    {
	  case gcn::Graphics::LEFT:
          textX = 4;
          break;
      case gcn::Graphics::CENTER:
          textX = getWidth() / 2;
          break;
      case gcn::Graphics::RIGHT:
          textX = getWidth() - 4;
          break;
      default:
          throw GCN_EXCEPTION("Unknown alignment.");
    }

    graphics->setFont(getFont());
    
    if (isDown())
    {
        graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
    }
    else
    {
        graphics->drawText(getCaption(), textX, textY, getAlignment());
        
        //if (isFocused())
        //{
        //    graphics->drawRectangle(gcn::Rectangle(2, 2, getWidth() - 4,
        //                                      getHeight() - 4));
        //}      
    }    
}

void MenuTop::drawFrame(gcn::Graphics* graphics)
{

}

int MenuTop::addMenuItem ( std::string Top, std::string Id, std::string Title, gcn::ActionListener *actionListener )
{
	return mMenuTopItem->addMenuItem( Top, Id, Title, actionListener );
}

void MenuTop::setItemChecked ( std::string item_id, bool checked )
{
	mMenuTopItem->setItemChecked ( item_id, checked );
}

void MenuTop::setItemDimmed ( std::string item_id, bool dimmed )
{
	mMenuTopItem->setItemDimmed ( item_id, dimmed );
}

void MenuTop::setItemCaption ( std::string item_id, std::string caption )
{
	mMenuTopItem->setItemCaption ( item_id, caption );
}

void MenuTop::setContainer ( gcn::Container *mTop )
{
	mMenuTopItem->setContainer ( mTop );
}

void MenuTop::setMenuX ( int x )
{
	mMenuTopItem->setX ( x );
}

void MenuTop::setMenuY ( int y )
{
	mMenuTopItem->setY ( y );
}

void MenuTop::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
		mMenuBar->setClick();
    }

	gcn::Button::mousePressed(mouseEvent);
}
