/******************************************************************************
*
* FILENAME: infomessage.hpp
*
* DESCRIPTION:  This class will show a message pop-up with one button
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   08/01/07  bberlin      Creation
******************************************************************************/
#ifndef infomessage_hpp
#define infomessage_hpp

#include "genericmessage.hpp"

class InfoMessage : public GenericMessage
{
	public:
		InfoMessage( GuiMain *gMain );
		~InfoMessage();

		void show ( std::string message );
		void action(const gcn::ActionEvent &actionEvent);

	private:
};

#endif
