/******************************************************************************
*
* FILENAME: menubar.cpp
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
#include "menubar.hpp"

MenuBar::MenuBar()
{
	mDown = false;
	mTop = 0;
}

MenuBar::~MenuBar()
{
	MenuTopIterator it;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		delete (*it);
	}
}

int MenuBar::addMenuItem ( std::string Top, std::string Id, std::string Title, gcn::ActionListener *actionListener )
{
	int x=-1;
	MenuTopIterator it;
	MenuTop *newTop;

	if ( !Top.size() )
   	{
		newTop = new MenuTop(this,Id,Title,actionListener);

		if ( mTop )
			mTop->add(newTop);

		mMenuBar.push_back(newTop);
		resize();
	}
	else
   	{
		for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
	   	{
			(*it)->addMenuItem( Top, Id, Title, actionListener );
		}
	}

	return 0;
}

void MenuBar::resize()
{
	gcn::Font *font;
	MenuTopIterator it;
	int x=-1,y;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		font = (*it)->getFont();
		y = font->getHeight()>>2;

		x += font->getWidth("  ");

		(*it)->setX(x);
		(*it)->setY(y);
		(*it)->setMenuX(x);
		(*it)->setMenuY(y + (*it)->getHeight() - 1);

		(*it)->getCaption();
		(*it)->adjustSize();

		x += font->getWidth((*it)->getCaption());
	}
}

int MenuBar::setItemChecked ( std::string item_id, bool checked )
{
	MenuTopIterator it;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		(*it)->setItemChecked ( item_id, checked );
	}

	return 0;
}

int MenuBar::setItemDimmed ( std::string item_id, bool dimmed )
{
	MenuTopIterator it;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		(*it)->setItemDimmed ( item_id, dimmed );
	}

	return 0;
}

int MenuBar::setItemCaption ( std::string item_id, std::string caption )
{
	MenuTopIterator it;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		(*it)->setItemCaption ( item_id, caption );
	}

	return 0;
}

void MenuBar::setClick(void)
{
	MenuTopIterator it;

	if ( mDown == false )
		mDown = true;
	else
		mDown = false;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
	{
		(*it)->setDown( mDown );
	}
}

void MenuBar::setContainer(gcn::Container *top)
{
	MenuTopIterator it;

	mTop = top;

	for ( it = mMenuBar.begin(); it != mMenuBar.end(); ++it )
   	{
		mTop->add(*it);
		(*it)->setContainer(mTop);
	}
}
