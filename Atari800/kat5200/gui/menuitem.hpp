/******************************************************************************
*
* FILENAME: menuitem.hpp
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
#ifndef menuitem_hpp
#define menuitem_hpp

#include <guichan.hpp>

class MenuBar;

class MenuItem : public gcn::ListBox,
                 public gcn::ListModel
{
public:
	MenuItem(std::string Id, std::string Title, gcn::ActionListener *Al=0, MenuBar *menu_bar=0);
	~MenuItem();
	bool isChecked();
	void setItemChecked(std::string item_id, bool checked);
	bool isDimmed();
	void setItemDimmed(std::string item_id, bool dimmed);
	void setItemCaption(std::string item_id, std::string caption );
	bool hasMouse();
	bool childrenHaveMouse();
	void resize();

	std::string getElementAt (int i);
	int	getNumberOfElements ();

	int addMenuItem ( std::string Top, std::string Id, std::string Title="", 
	                  gcn::ActionListener *actionListener=0 );

	void draw(gcn::Graphics *graphics);
	void drawFrame(gcn::Graphics *graphics);
	void setVisible(bool visible);
	void setParentMenuItem ( MenuItem *parent );
	void setContainer(gcn::Container *mTop);
	void mouseMoved(gcn::MouseEvent &mouseEvent);
	void mousePressed(gcn::MouseEvent &mouseEvent);
	void mouseEntered (gcn::MouseEvent &mouseEvent);
	void mouseExited (gcn::MouseEvent &mouseEvent);
private:
	int mouseY;
	bool mDimmed;
	bool mChecked;
	bool mHasMouse;
	MenuItem *mParentMenuItem;
	MenuBar *mParentMenuBar;
	std::string mTitle;
	std::vector<MenuItem *> mMenuItems;
	typedef std::vector<MenuItem *>::iterator MenuItemIterator;
};

#endif
