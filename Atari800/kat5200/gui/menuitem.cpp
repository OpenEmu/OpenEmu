/******************************************************************************
*
* FILENAME: menuitem.cpp
*
* DESCRIPTION:  Class used with the Menu Bar
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/20/07  bberlin      Creation, redo from 0.5.0
******************************************************************************/
#include "menuitem.hpp"
#include "menubar.hpp"

MenuItem::MenuItem(std::string Id, std::string Title, gcn::ActionListener *actionListener, MenuBar *menu_bar)
{
	setActionEventId(Id);
	mTitle = Title;
	mHasMouse = false;
	mChecked = false;
	mDimmed = false;
	mParentMenuItem = 0;
	mParentMenuBar = menu_bar;
	if ( actionListener )
		addActionListener ( actionListener );
	setListModel ( this );
	mouseY = -999;
}

MenuItem::~MenuItem()
{
	MenuItemIterator it;

	for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
   	{
		delete (*it);
	}
}

std::string MenuItem::getElementAt (int i)
{
	if ( i < getNumberOfElements() )
		return mMenuItems.at(i)->mTitle;
	else
		return "";
}

int MenuItem::getNumberOfElements()
{
	return mMenuItems.size();
}

bool MenuItem::hasMouse()
{
	return mHasMouse;
}

bool MenuItem::childrenHaveMouse()
{
	MenuItemIterator it;

	for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
   	{
		if ( (*it)->hasMouse() )
			return true;

		if ( (*it)->childrenHaveMouse() == true )
			return true;
	}

	return false;
}

bool MenuItem::isChecked()
{
	return mChecked;
}

void MenuItem::setContainer(gcn::Container *mTop)
{
	MenuItemIterator it;

	if ( !mTop )
		return;

	mTop->add(this);

	for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
	{
		(*it)->setContainer ( mTop );
	}
}

void MenuItem::setItemChecked(std::string item_id, bool checked )
{
	MenuItemIterator it;

	if ( item_id == mActionEventId )
		mChecked = checked;
	else
	{
		for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
		{
			(*it)->setItemChecked ( item_id, checked );
		}
	}
}

void MenuItem::setItemDimmed(std::string item_id, bool dimmed )
{
	MenuItemIterator it;

	if ( item_id == mActionEventId )
		mDimmed = dimmed;
	else
	{
		for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
		{
			(*it)->setItemDimmed ( item_id, dimmed );
		}
	}
}

bool MenuItem::isDimmed()
{
	return mDimmed;
}

void MenuItem::setItemCaption(std::string item_id, std::string caption )
{
	MenuItemIterator it;

	if ( item_id == mActionEventId )
	{
		mTitle = caption;
		if ( mParentMenuItem )
			mParentMenuItem->resize();
	}
	else
	{
		for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
		{
			(*it)->setItemCaption ( item_id, caption );
		}
	}
}

void MenuItem::resize()
{
	std::string tmp_string;
	int i;

	setWidth ( 0 );

    for (i = 0; i < getNumberOfElements(); ++i)
    {      
		tmp_string = "     " + getElementAt(i);
		if ( getFont()->getWidth(tmp_string)  > getWidth())
		{
			setWidth ( getFont()->getWidth(tmp_string) );
		}
	}
}

int MenuItem::addMenuItem ( std::string Top, std::string Id, std::string Title, gcn::ActionListener *actionListener )
{
	MenuItemIterator it;

	if ( Top == mActionEventId )
	{
		mMenuItems.push_back ( new MenuItem(Id,Title,actionListener,mParentMenuBar) );
		resize();
	}
	else 
	{
		for ( it = mMenuItems.begin(); it != mMenuItems.end(); ++it )
	   	{
			(*it)->addMenuItem( Top, Id, Title, actionListener );
		}
	}

	return 0;
}

void MenuItem::draw(gcn::Graphics* graphics)
{
	gcn::Rectangle MyRect = getDimension();
	bool child_has_mouse;
	std::string tmp_string;

    //graphics->setColor(getBackgroundColor());
    graphics->setColor(getBaseColor());
    graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    if (mListModel == NULL || !getNumberOfElements())      
        return;

    graphics->setColor(getForegroundColor());
    graphics->setFont(getFont());    

    int i, fontHeight,j;
    int y = 0;

    fontHeight = getFont()->getHeight();

    /**
     * @todo Check cliprects so we do not have to iterate over elements in the list model
     */
    for (i = 0; i < getNumberOfElements(); ++i)
    {      
		tmp_string = "   " + getElementAt(i);

		if ( mMenuItems.at(i)->getNumberOfElements() )
	   	{
			for ( j = 0; j < 6; ++j )
			{
				graphics->drawLine ( getWidth()-j-2, y+(fontHeight/2)-j, getWidth()-j-2, y+(fontHeight/2)+j );
			}
			mMenuItems.at(i)->setX (getX() + getWidth() - 1);
			mMenuItems.at(i)->setY (getY() + y + 1);
			child_has_mouse = mMenuItems.at(i)->hasMouse();
			mMenuItems.at(i)->setVisible ( false );

			if ( i == mSelected && (hasMouse() || child_has_mouse) )
			{
				mMenuItems.at(i)->setVisible ( true );
				mMenuItems.at(i)->requestMoveToTop ();
			}
		}

		// Now check for checked and draw
		if ( mMenuItems.at(i)->isChecked() == true )
		{
			graphics->drawLine(3, y+5, 3, (y+fontHeight) - 3);
			graphics->drawLine(4, y+5, 4, (y+fontHeight) - 3);

			graphics->drawLine(5, (y+fontHeight) - 4, (fontHeight) - 2, y+3);
			graphics->drawLine(5, (y+fontHeight) - 5, (fontHeight) - 4, y+4);
		}
		
        if (i == mSelected)
        {
            graphics->drawRectangle(gcn::Rectangle(0, y, getWidth(), fontHeight));
        }
  
		if ( !mMenuItems.at(i)->isDimmed() )
	        graphics->drawText(tmp_string, 1, y);

        y += fontHeight;
    }    

	drawFrame ( graphics );
}

void MenuItem::drawFrame(gcn::Graphics* graphics)
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

    graphics->setColor(highlightColor);
    graphics->drawLine(0, 0, getWidth() - 1, 0);
    graphics->drawLine(0, 1, 0, getHeight() - 1);

    graphics->setColor(shadowColor);
    graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
    graphics->drawLine(1, getHeight() - 1, getWidth() - 1, getHeight() - 1); 

	/*
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
	*/
}

void MenuItem::setParentMenuItem(MenuItem *parent)
{
	mParentMenuItem = parent;
}

void MenuItem::setVisible(bool visible)
{
	MenuItemIterator it;

	if ( visible == true && mMenuItems.size() )
		mVisible = true;
	else
		mVisible = false;

	if (!mVisible && isFocused())
	{
		mFocusHandler->focusNone();
	}

	if ( mVisible == false )
	{
	    for (it = mMenuItems.begin(); it != mMenuItems.end(); ++it)
   		{      
			(*it)->setVisible(false);
		}

		//mHasMouse = false;
	}
}

void MenuItem::mouseMoved(gcn::MouseEvent &mouseEvent)
{
	int i,fontHeight;

	fontHeight = getFont()->getHeight();

    for (i = 0; i < getNumberOfElements(); ++i)
    {
		if ( (mouseEvent.getY() >= (i*fontHeight)) && (mouseEvent.getY() < (i+1)*fontHeight) ) {
			mSelected = i;
			break;
		}
	}
}

void MenuItem::mousePressed(gcn::MouseEvent &mouseEvent)
{
	if ( (mouseEvent.getButton() == gcn::MouseEvent::LEFT) && hasMouse() )
	{
		setSelected(mouseEvent.getY() / getFont()->getHeight());

		if ( mMenuItems.at(getSelected())->mActionListeners.size() )
		{
			if ( mParentMenuBar )
				mParentMenuBar->setClick();

			setVisible ( false );

			if ( mParentMenuItem )
				mParentMenuItem->setVisible( false );

			mHasMouse = false;
		}

		if ( mMenuItems.at(getSelected())->isDimmed() )
		{
			setSelected(0);
			return;
		}

		mMenuItems.at(getSelected())->distributeActionEvent();
		setSelected(0);
	}
}

void MenuItem::mouseEntered(gcn::MouseEvent &mouseEvent)
{
	mHasMouse = true;
}

void MenuItem::mouseExited(gcn::MouseEvent &mouseEvent)
{
	mHasMouse = false;
}
