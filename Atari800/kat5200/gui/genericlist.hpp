/******************************************************************************
*
* FILENAME: genericlist.hpp
*
* DESCRIPTION:  Header to describe file list used with the File Browser
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/05/06  bberlin      Creation
******************************************************************************/
#ifndef genericlist_hpp
#define genericlist_hpp

#include <guichan.hpp>

extern "C" {
	#include "../interface/kconfig.h"
}

typedef enum {
	GLIST_SORT_ALPHA,
	GLIST_SORT_FILE_ALPHA,
	GLIST_SORT_VALUE,
	GLIST_SORT_NONE
} e_glist_sort;

class GenericList : public gcn::ListModel
{
private:
	struct generic_node *element;
	int num_items;
	e_glist_sort sort_method;
	char empty_label[2];

public:

	GenericList ( e_glist_sort method=GLIST_SORT_ALPHA ) {
		element = 0;
		num_items = 0;
		sort_method = method;
		strcpy ( empty_label, "" );
	}

	~GenericList ( ) {
		deleteAll();
	}

	void setList ( struct generic_node *list )
	{
		element = list;
		getNumberOfItems();
	}

	struct generic_node * getList ( void )
	{
		return element;
	}

	void setSortMethod ( e_glist_sort type )
	{
		sort_method = type;
	}

	int getNumberOfElements()
	{
		return num_items;
	}

	std::string getElementAt(int i)
	{
		struct generic_node *tmp_item;
		int j=0;

		tmp_item = element;
		while ( tmp_item->next ) {
			if ( j == i ) break;
			tmp_item = tmp_item->next;
			j++;
		}

		return tmp_item->name;
	}

	std::string eraseOneLetter(int i)
	{
		struct generic_node *tmp_item;
		int j=0;

		tmp_item = element;
		while ( tmp_item->next ) {
			if ( j == i ) break;
			tmp_item = tmp_item->next;
			j++;
		}

		tmp_item->name[strlen(tmp_item->name)-1] = '\0';

		return tmp_item->name;
	}

	int getIndexFromValue( int value )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( tmp_item->value == value )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		return i;
	}

	unsigned long getValueFromIndex( int index )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( i == index )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		if ( tmp_item )
			return tmp_item->value;
		else
			return 0;
	}

	unsigned long getDataFromIndex( int index )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( i == index )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		if ( tmp_item )
			return tmp_item->data;
		else
			return 0;
	}

	void editLabelFromIndex( char *label, int index )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( i == index )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		if ( tmp_item )
			strcpy ( tmp_item->name, label );
	}

	char * getLabelFromIndex( int index )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( i == index )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		if ( tmp_item )
			return tmp_item->name;
		else
			return empty_label;
	}

	char * getLabelFromValue( int value )
	{
		struct generic_node *tmp_item;

		tmp_item = element;
		while ( tmp_item ) {
			if ( tmp_item->value == value )
				break;
			tmp_item = tmp_item->next;
		}
		if ( tmp_item )
			return tmp_item->name;
		else
			return empty_label;
	}

	int getIndexFromLabel( const char *label )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			if ( !strcmp(tmp_item->name, label) )
				break;
			++i;
			tmp_item = tmp_item->next;
		}
		return i;
	}

	int getNumberOfItems( )
	{
		struct generic_node *tmp_item;
		int i = 0;

		tmp_item = element;
		while ( tmp_item ) {
			++i;
			tmp_item = tmp_item->next;
		}

		num_items = i;

		return i;
	}

	void addElement ( char *name, int value, int data ) {

		struct generic_node *tmp_item;
		struct generic_node *prev_item;
		char current_name[1024];
		char new_name[1024];
		int i;
		int done_flag = 0;

		num_items++;

		/*
		 * Add Element to List
		 */
		if ( tmp_item = element ) {

			switch ( sort_method ) {

				case GLIST_SORT_ALPHA:
				case GLIST_SORT_FILE_ALPHA:

				i = -1;
				while ( name[++i] )
					new_name[i] = tolower ( name[i] );
				new_name[i] = '\0';

				prev_item = 0;
				while ( tmp_item ) {

					i = -1;
					while ( tmp_item->name[++i] )
						current_name[i] = tolower ( tmp_item->name[i] );
					current_name[i] = '\0';

					/*
					 * Sort directories first, then alphabetical for Files
					 *    Or just plain old sort alphabetical
					 */
					if ( (value==1 && tmp_item->value==0 && 
					      sort_method==GLIST_SORT_FILE_ALPHA) ||
					     ((strcmp(new_name,current_name) < 0) && value==tmp_item->value) ) {

						if ( prev_item ) {
							prev_item->next = (struct generic_node *)malloc ( sizeof(struct generic_node) );
							prev_item->next->next = tmp_item;
							tmp_item = prev_item->next;
						}
						else {
							element = (struct generic_node *)malloc ( sizeof(struct generic_node) );
							element->next = tmp_item;
							tmp_item = element;
						}
						done_flag = 1;
						break;
					}

					prev_item = tmp_item;
					tmp_item = tmp_item->next;
				}
				if ( !done_flag ) {
					if ( !prev_item )
						prev_item = element;
					prev_item->next = (struct generic_node *)malloc ( sizeof(struct generic_node) );
					tmp_item = prev_item->next;
					tmp_item->next = 0;
				}
				break;

				case GLIST_SORT_VALUE:
					prev_item = 0;
					while ( tmp_item->next ) {
						if ( tmp_item->value < (unsigned int)value ) {
						}
						tmp_item = tmp_item->next;
					}
				break;

				case GLIST_SORT_NONE:
					while ( tmp_item->next ) {
						tmp_item = tmp_item->next;
					}
					tmp_item->next = (struct generic_node *)malloc ( sizeof(struct generic_node) );
					tmp_item = tmp_item->next;
					tmp_item->next = 0;
					break;

			} /* end switch sort method */

		} /* end if list elements exist */

		/*
		 * Or Create List
		 */
		else {
			element = (struct generic_node *)malloc ( sizeof(struct generic_node) );
			tmp_item = element;
			tmp_item->next = 0;
		}

		strcpy ( tmp_item->name, name );
		tmp_item->value = value;
		tmp_item->data = data;
	}

	void deleteAll ( void ) {

		struct generic_node *tmp_item, *prev_item;
		int i = 0;

		tmp_item = prev_item = element;
		while ( tmp_item ) {
			prev_item = tmp_item;
			tmp_item = tmp_item->next;
			free ( prev_item );
			if ( ++i == num_items )
				break;
		}

		element = 0;
		num_items = 0;
	}

	int getNextAvailableValue ( int start ) {

		struct generic_node *tmp_item;
		int number = -999;
		int i = start-1;

		tmp_item = element;
		while ( number == -999 ) {
			++i;
			number = i;
			tmp_item = element;
			while ( tmp_item ) {
				if ( tmp_item->value == i )
					number = -999;
				tmp_item = tmp_item->next;
			}
		}

		return number;
	}

	void sort ( void ) {

	}
};

#endif
