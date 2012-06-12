/******************************************************************************
*
* FILENAME: menubar.hpp
*
* DESCRIPTION:  Header to describe class used with the Menu
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/20/07  bberlin      Creation, redo from 0.5.0
******************************************************************************/
#ifndef menubar_hpp
#define menubar_hpp

#include <guichan.hpp>
#include "menutop.hpp"

class MenuBar
{
public:
	MenuBar();
	~MenuBar();
	void setContainer(gcn::Container *top);
	void setClick(void);
	void resize(void);
	int addMenuItem ( std::string Top, std::string Id, std::string Title="", 
	                  gcn::ActionListener *actionListener=0 );
	int setItemChecked ( std::string Id, bool checked );
	int setItemDimmed ( std::string Id, bool dimmed );
	int setItemCaption ( std::string Id, std::string caption );
private:
	bool mDown;
	gcn::Container *mTop;
	std::vector<MenuTop *> mMenuBar;
	typedef std::vector<MenuTop *>::iterator MenuTopIterator;
};

#endif
