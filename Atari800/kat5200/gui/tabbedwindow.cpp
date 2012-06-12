/******************************************************************************
*
* FILENAME: tabbedwindow.cpp
*
* DESCRIPTION:  Class used to create a tab control
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/26/06  bberlin      Creation
******************************************************************************/
#include <set>
#include <algorithm>
#include "tabbedwindow.hpp"

TabbedWindow::TabbedWindow()
{
	setAlignment ( gcn::Graphics::CENTER );
	setMovable ( false );
	mSelectedTab = 0;
	mSelectedRow = 0;
}

TabbedWindow::~TabbedWindow()
{

}

int TabbedWindow::getSelectedIndex()
{
	int row = -1;
	int index = 0;
	RowListIterator r_iter;

	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		++row;
		if ( row == mSelectedRow )
		{
			return index + mSelectedTab;
		}

		index += (*r_iter)->size();
	}

	return mSelectedTab;
}

int TabbedWindow::getSelectedRow()
{
	return mSelectedRow;
}

void TabbedWindow::addTab(unsigned int row, gcn::Window * window)
{
	WindowList *pList;

	while ( row >= mRows.size() ) 
	{
		mRows.push_back ( new WindowList );
	}

	pList = mRows.at(row);

	pList->push_back(window);

	if (mInternalFocusHandler == NULL)
	{
		window->_setFocusHandler(_getFocusHandler());
	}
	else
	{
		window->_setFocusHandler(mInternalFocusHandler);
	}

	setTitleBarHeight ( mRows.size() * (getFont()->getHeight() + 1) );

	window->_setParent(this);
	window->setFrameSize ( 0 );

}

void TabbedWindow::removeTab(gcn::Window * window)
{
	int row = -1;
	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		++row;
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			if (*w_iter == window)
		    {
				(*r_iter)->erase(w_iter);
				window->_setFocusHandler(NULL);
				window->_setParent(NULL);
				window->removeDeathListener(this);
				if ( row == mSelectedRow )
				{
					if ( mSelectedTab >= (*r_iter)->size() && (*r_iter)->size() )
						mSelectedTab--;
				}
				return;
			}
		}
	}

	throw GCN_EXCEPTION("There is no such window in this tabbed container.");
}

void TabbedWindow::selectTab(gcn::Window * window)
{
	int row = -1;
	int item = -1;
	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		++row;
		item = -1;
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			++item;
			if (*w_iter == window)
		    {
				mSelectedRow = row;
				mSelectedTab = item;
				return;
			}
		}
	}

	throw GCN_EXCEPTION("There is no such window in this tabbed container.");
}

void TabbedWindow::removeAll()
{
	int row = -1;
	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		++row;
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			(*w_iter)->_setFocusHandler(NULL);
			(*w_iter)->_setParent(NULL);
			(*w_iter)->removeDeathListener(this);
		}

		(*r_iter)->clear();

		delete (*r_iter);
	}

	mRows.clear();
	mSelectedRow = 0;
	mSelectedTab = 0;
}

int TabbedWindow::getTabLength(int row)
{
	WindowList *pList;
	int max_tab_length, actual_tab_length, tab_length;

	pList = mRows.at(row);

	max_tab_length = getWidth() / (int)(pList->size());
	actual_tab_length = 0;

    WindowListIterator it;
    for (it = pList->begin(); it != pList->end(); it++)
    {
		tab_length = getFont()->getWidth ( (*it)->getCaption() ) + 4;
		if ( tab_length > max_tab_length ) {
			actual_tab_length = max_tab_length;
		}
		else if ( tab_length > actual_tab_length ) {
			actual_tab_length = tab_length;
		}
    }

	//return actual_tab_length;
	return max_tab_length;
}

void TabbedWindow::draw(gcn::Graphics* graphics)
{
	gcn::Color faceColor = getBaseColor();
	gcn::Color highlightColor, shadowColor;
	int alpha = getBaseColor().a;
	int width = getWidth() + getFrameSize() * 2 - 1;
	int height = getHeight() + getFrameSize() * 2 - 1;
	int x;
	int tab_length;
	highlightColor = faceColor + 0x303030;
	highlightColor.a = alpha;
	shadowColor = faceColor - 0x303030;
	shadowColor.a = alpha;

	gcn::Rectangle d = getChildrenArea();

	// Fill the background around the content
	graphics->setColor(faceColor);
	// Fill top
	graphics->fillRectangle(gcn::Rectangle(0,0,getWidth(),d.y - 1));
	// Fill left
	graphics->fillRectangle(gcn::Rectangle(0,d.y - 1, d.x - 1, getHeight() - d.y + 1));
	// Fill right
	graphics->fillRectangle(gcn::Rectangle(d.x + d.width + 1,
	                                  d.y - 1,
	                                  getWidth() - d.x - d.width - 1,
	                                  getHeight() - d.y + 1));
	// Fill bottom
	graphics->fillRectangle(gcn::Rectangle(d.x - 1,
	                                  d.y + d.height + 1,
	                                  d.width + 2,
	                                  getHeight() - d.height - d.y - 1));

	if (isOpaque())
	{
	    graphics->fillRectangle(d);
	}

	// Construct a rectangle one pixel bigger than the content
	d.x -= 1;
	d.y -= 1;
	d.width += 2;
	d.height += 2;

	// Draw a border around the content
	graphics->setColor(shadowColor);

	// Top line (of child area, bottom line for tabs)
	graphics->drawLine(d.x,
	                   d.y,
	                   d.x + d.width,
	                   d.y);

	// Now draw a line where the selected tab is (at bottom of title bar)
	graphics->setColor(faceColor);

	graphics->drawLine(d.x + mSelectedTab * getTabLength(mSelectedRow) - 1,
	                   d.y,
	                   d.x + getTabLength(mSelectedRow) * (mSelectedTab+1) - 3,
	                   d.y );

	graphics->setColor(shadowColor);

	/*
	// Left line
	graphics->drawLine(d.x,
	                   d.y + 1,
	                   d.x,
	                   d.y + d.height - 1);

	graphics->setColor(highlightColor);
	// Right line
	graphics->drawLine(d.x + d.width - 1,
	                   d.y,
	                   d.x + d.width - 1,
	                   d.y + d.height - 2);
	// Bottom line
	graphics->drawLine(d.x + 1,
	                   d.y + d.height - 1,
	                   d.x + d.width - 1,
	                   d.y + d.height - 1);
	*/

	drawChildren(graphics);
	drawChildWindow(graphics);

	// Now draw text for each tab
	int i,j, row = -1, drawn_row = 0;
	int textX;
	int textY;
	int current_x;
	textY = (getTitleBarHeight() - getFont()->getHeight()) / 2;

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		i = -1;
		++row;
		tab_length = getTabLength(row);
		if ( row == mSelectedRow )
			textY = ((getTitleBarHeight() / mRows.size()) * (mRows.size()-1)) + 2;
		else
		{
			textY = (getTitleBarHeight() / mRows.size() * drawn_row) + 2;
		}

		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			++i;
			current_x = tab_length * i;
			switch (getAlignment())
			{
				case gcn::Graphics::LEFT:
			      textX = 4 + current_x;
			      break;
				case gcn::Graphics::CENTER:
			      textX = (tab_length / 2) + current_x;
			      break;
				case gcn::Graphics::RIGHT:
			      textX = (tab_length - 4) + current_x;
			      break;
			  default:
			      throw GCN_EXCEPTION("Unknown alignment.");
			}

			graphics->setColor(getForegroundColor());
			graphics->setFont(getFont());
			graphics->drawText((*w_iter)->getCaption(), textX, textY, getAlignment());

			int y;
			x = 0;

			/*
			 * If this is not the selected tab 
			 */
			if ( i != mSelectedTab || row !=mSelectedRow )
			{
				/*
				 * Horizontal tab lines
				 */
				graphics->setColor(highlightColor);
				for ( j = 0; j < (int)getFrameSize(); ++j )
				{
					if ( (i+1) == mSelectedTab && row == mSelectedRow )
					{
						y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size()) + j + 1;
					   	graphics->drawLine(tab_length*i+j,y, tab_length*(i+1)-1, y);
					}
					else if ( row == mSelectedRow )
					{
						y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size()) + j + 1;
					   	graphics->drawLine(tab_length*i+j,y, tab_length*(i+1), y);
					}
					else
					{
						y = drawn_row * (getTitleBarHeight() / mRows.size()) + j + 1;
					   	graphics->drawLine(tab_length*i+j,y, tab_length*(i+1), y);
					}
				}

				/*
				 * Vertical tab lines
				 */
				if ( i < (int)((*r_iter)->size()-1)) {
					x = 1;
					if ( (i+1) == mSelectedTab && row == mSelectedRow )
					{
						y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size());
						graphics->drawLine(tab_length * (i + 1)-1,y, 
						                   tab_length * (i + 1)-1, y+ getFont()->getHeight()-1);
					}
					else if ( row == mSelectedRow )
					{
						y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size());
						graphics->drawLine(tab_length * (i + 1),y + 1, 
						                   tab_length * (i + 1), y+ getFont()->getHeight()-1);
					}
					else
					{
						y = drawn_row * (getTitleBarHeight() / mRows.size());
						graphics->drawLine(tab_length * (i + 1),y + 1, 
						                   tab_length * (i + 1), y + getFont()->getHeight()-1);
					}
				}

			} /* end this is not the selected tab */

			/*
			 * Else this is the selected tab
			 */
			else
			{
				graphics->setColor(highlightColor);
				for ( j = 0; j < (int)getFrameSize(); ++j )
				{
					y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size()) + j;
					graphics->drawLine(tab_length*mSelectedTab+j,y, tab_length*(mSelectedTab+1), y);
				}
			}

			/*
			 * Grey vertical line to right of tab when next is not selected
			 */
			if ( (i+1) != mSelectedTab || row != mSelectedRow )
			{
				x = 1;
				if ( row == mSelectedRow )
					y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size());
				else
					y = drawn_row * (getTitleBarHeight() / mRows.size());
				graphics->setColor(shadowColor);
				graphics->drawLine(tab_length * (i + 1)-x,y+1, 
				                   tab_length * (i + 1)-x, y+getFont()->getHeight()-1);
			}

		} /* for each tab */

		if ( row != mSelectedRow )
			drawn_row++;

	} /* for each row */
}

void TabbedWindow::drawFrame(gcn::Graphics* graphics)
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

	unsigned int i,j;
	for (i = 0; i < getFrameSize(); ++i)
	{
		int row = -1;
		RowListIterator r_iter;
		for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
		{
			++row;
		    graphics->setColor(highlightColor);
		    //graphics->drawLine(i,i, width - i, i);
			for ( j = 0; j < (*r_iter)->size(); ++j )
			{
				if ( j == mSelectedTab ) 
				{
					//int y = getTitleBarHeight() - (getTitleBarHeight() / mRows.size()) + i;
			    	//graphics->drawLine(getTabLength(row)*j+i,y, getTabLength(row)*(j+1), y);
				}
			}
		    graphics->drawLine(i,i + 1, i, height - i - 1);
		    graphics->setColor(shadowColor);
		    //graphics->drawLine(width - i,i + 1, width - i, height - i);
		    graphics->drawLine(width - i,i + getTitleBarHeight(), width - i, height - i);
		    graphics->drawLine(i,height - i, width - i - 1, height - i);
		}
	}
}

void TabbedWindow::drawChildWindow(gcn::Graphics* graphics)
{
	int i = -1;
	int row = -1;

    graphics->pushClipArea(getChildrenArea());

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		i = -1;
		++row;
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			++i;
			if ( i == mSelectedTab && row == mSelectedRow )
			{
				graphics->pushClipArea((*w_iter)->getDimension());
				(*w_iter)->setVisible(true);
				(*w_iter)->draw(graphics);
				graphics->popClipArea();
			}
			else {
				(*w_iter)->setVisible(false);
			}
		}
	}

    graphics->popClipArea();
}

gcn::Widget *TabbedWindow::getWidgetAt(int x, int y)
{
	gcn::Rectangle r = getChildrenArea();

    if (!r.isPointInRect(x, y))
    {
        return NULL;
    }

    x -= r.x;
    y -= r.y;

    WidgetListReverseIterator it;
    for (it = mWidgets.rbegin(); it != mWidgets.rend(); it++)
    {
        if ((*it)->isVisible() && (*it)->getDimension().isPointInRect(x, y))
        {
            return (*it);
        }
    }

	RowListIterator r_iter;
	WindowListReverseIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		for (w_iter = (*r_iter)->rbegin(); w_iter != (*r_iter)->rend(); w_iter++)
		{
	        if ((*w_iter)->isVisible() && (*w_iter)->getDimension().isPointInRect(x, y))
			{
				return (*w_iter);
			}
		}
    }

    return NULL;
}

void TabbedWindow::_setFocusHandler(gcn::FocusHandler* focusHandler)
{
	gcn::Widget::_setFocusHandler(focusHandler);

    if (mInternalFocusHandler != NULL)
    {
        return;
    }

    WidgetListIterator iter;
    for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
    {
        (*iter)->_setFocusHandler(focusHandler);
    }

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
	        (*w_iter)->_setFocusHandler(focusHandler);
		}
	}
}

void TabbedWindow::setInternalFocusHandler(gcn::FocusHandler* focusHandler)
{
    mInternalFocusHandler = focusHandler;

    WidgetListIterator iter;
    for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
    {
        if (mInternalFocusHandler == NULL)
        {
            (*iter)->_setFocusHandler(_getFocusHandler());
        }
        else
        {
            (*iter)->_setFocusHandler(mInternalFocusHandler);
        }
    }

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
	        if (mInternalFocusHandler == NULL)
			{
				(*w_iter)->_setFocusHandler(_getFocusHandler());
			}
			else
			{
				(*w_iter)->_setFocusHandler(mInternalFocusHandler);
			}
		}
	}
}

void TabbedWindow::logicChildren()
{
    WidgetListIterator iter;
    for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
    {
        (*iter)->logic();
    }

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
	        (*w_iter)->logic();
		}
	}
}

void TabbedWindow::mousePressed(gcn::MouseEvent &mouseEvent)
{
		int selected_row, selected_tab;

		gcn::Window::mousePressed(mouseEvent);

        if (mouseEvent.getSource() != this)
        {
            return;
        }

        if ( mouseEvent.getY() < (int)(getTitleBarHeight() + getPadding()) && 
		     mouseEvent.getButton() == gcn::MouseEvent::LEFT)
        {
			selected_row = mouseEvent.getY() / (getTitleBarHeight() / mRows.size());
			selected_tab = mouseEvent.getX() / (getTabLength(mSelectedRow)+1);
			if ( selected_row == mRows.size()-1 )
			{
				if ( selected_tab < (int)mRows.at(mSelectedRow)->size() )
					mSelectedTab = selected_tab;
			}
			else
			{
				RowListIterator r_iter;
				int row = -1, drawn_row = -1;
				for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
				{
					++row;
					if ( row != mSelectedRow )
						++drawn_row;

					if ( drawn_row == selected_row )
					{
						mSelectedRow = row;
						if ( selected_tab < (int)mRows.at(mSelectedRow)->size() )
							mSelectedTab = selected_tab;
					}
				}
			}
        }
}

void TabbedWindow::moveToTop(gcn::Widget* widget)
{
    WidgetListIterator iter;
    for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
    {
        if (*iter == widget)
        {
            mWidgets.erase(iter);
            mWidgets.push_back(widget);
            return;
        }
    }

	RowListIterator r_iter;
	WindowListIterator w_iter;
	for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
	{
		for (w_iter = (*r_iter)->begin(); w_iter != (*r_iter)->end(); w_iter++)
		{
			if (*w_iter == widget)
			{
				return;
			}
		}
	}

    throw GCN_EXCEPTION("There is no such widget in this container.");
}

void TabbedWindow::moveToBottom(gcn::Widget* widget)
{
    WidgetListIterator iter;
    iter = find(mWidgets.begin(), mWidgets.end(), widget);

    if (iter == mWidgets.end())
    {
		RowListIterator r_iter;
    	WindowListIterator iter_w;
		for (r_iter = mRows.begin(); r_iter != mRows.end(); r_iter++)
		{
		    iter_w = find((*r_iter)->begin(), (*r_iter)->end(), widget);
			if ( iter_w != (*r_iter)->end() )
				break;
		}
		if ( iter_w == (*r_iter)->end() )
		{
	        throw GCN_EXCEPTION("There is no such widget in this container.");
		}
    }
	else {
	    mWidgets.erase(iter);
		mWidgets.push_front(widget);
	}
}
