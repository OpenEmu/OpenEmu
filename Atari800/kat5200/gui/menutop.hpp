/******************************************************************************
*
* FILENAME: menutop.hpp
*
* DESCRIPTION:  Header to describe class used with the Menu Bar
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/20/06  bberlin      Creation, redo from 0.5.0
******************************************************************************/
#ifndef menutop_hpp
#define menutop_hpp

#include <guichan.hpp>
#include "menuitem.hpp"

class MenuBar;

class MenuTop : public gcn::Button
{
public:
	MenuTop (MenuBar *menu_bar, std::string Id, std::string Title="", gcn::ActionListener *actionListener=0);
	~MenuTop ();

	void draw(gcn::Graphics *graphics);
	void drawFrame(gcn::Graphics *graphics);
	void mousePressed(gcn::MouseEvent& mouseEvent);

	int addMenuItem ( std::string Top, std::string Id, std::string Title="", 
	                  gcn::ActionListener *actionListener=0 );
	void setItemChecked ( std::string Id, bool checked );
	void setItemDimmed ( std::string Id, bool dimmed );
	void setItemCaption ( std::string item_id, std::string caption );
	void setContainer ( gcn::Container *mTop );
	void setMenuX ( int x );
	void setMenuY ( int y );
	bool isDown(void);
	void setDown(bool down);

private:
	bool mDown;
	MenuBar *mMenuBar;
	MenuItem *mMenuTopItem;
};

#endif
