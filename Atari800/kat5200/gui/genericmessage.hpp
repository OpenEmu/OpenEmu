/******************************************************************************
*
* FILENAME: genericmessage.hpp
*
* DESCRIPTION:  This class will show a message pop-up with optional buttons
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/01/06  bberlin      Creation
******************************************************************************/
#ifndef genericmessage_hpp
#define genericmessage_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"

class GenericMessage : public gcn::GuiWindow,
                       public GuiObject,
                       public gcn::ActionListener
{
	public:
		GenericMessage( GuiMain *gMain );
		~GenericMessage();

		void show ( std::string message, std::string button0, bool show0, 
		                                 std::string button1, bool show1,
		                                 std::string button2, bool show2, 
										 bool showinput );
		void hide ( );
		int getButton ( );
		void setButton ( int button );
		void deleteActions ( );
		std::string getInput();
		void setInput( std::string input );
		void action(const gcn::ActionEvent &actionEvent);

	protected:
		gcn::Button mButton[3];
		gcn::Label mMessage;
		gcn::TextField mInput;

	private:
		int mButtonIndex;
};

#endif
