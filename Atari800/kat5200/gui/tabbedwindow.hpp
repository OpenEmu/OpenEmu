/******************************************************************************
*
* FILENAME: tabbedwindow.hpp
*
* DESCRIPTION:  Header to describe tab class
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/26/06  bberlin      Creation
******************************************************************************/
#ifndef tabbedwindow_hpp
#define tabbedwindow_hpp

#include <guichan.hpp>

typedef std::vector< gcn::Window * > WindowList;
typedef WindowList::iterator WindowListIterator;
typedef WindowList::reverse_iterator WindowListReverseIterator;
typedef std::vector< WindowList * > RowList;
typedef RowList::iterator RowListIterator;
typedef RowList::reverse_iterator RowListReverseIterator;

class TabbedWindow : public gcn::Window
{
public:
	TabbedWindow();
	~TabbedWindow();
	int getSelectedIndex();
	int getSelectedRow();
	void addTab ( unsigned int row, gcn::Window *window );
	void removeTab ( gcn::Window *window );
	void selectTab ( gcn::Window *window );
	void removeAll();
	int getTabLength ( int row );
	void draw(gcn::Graphics *graphics);
	void drawFrame(gcn::Graphics *graphics);
	void drawChildWindow(gcn::Graphics *graphics);
	gcn::Widget *getWidgetAt(int x, int y);
	void _setFocusHandler(gcn::FocusHandler* focusHandler);
	void setInternalFocusHandler(gcn::FocusHandler* focusHandler);
	void moveToTop(gcn::Widget* widget);
	void moveToBottom(gcn::Widget* widget);
	void logicChildren();
	void mousePressed(gcn::MouseEvent &mouseEvent);
private:
	unsigned int mSelectedTab;
	unsigned int mSelectedRow;
	std::vector< WindowList * > mRows;
};

#endif
