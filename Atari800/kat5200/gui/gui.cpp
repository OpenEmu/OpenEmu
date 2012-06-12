/******************************************************************************
*
* FILENAME: gui.cpp
*
* DESCRIPTION:  This is the *main* for the GUI
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/03/06  bberlin      Creation
******************************************************************************/
#include <iostream>
#include "guimain.hpp"

extern "C" int gui_show_main ( int error, int debug_flag )
{
	try
	{
		GuiMain *guimain = new GuiMain ( error, debug_flag );
		bool exit, load, mode;

		guimain->run();

		exit = guimain->getExit();
		load = guimain->getStateLoad();
		mode = guimain->getDebugMode();

		delete guimain;

		if (exit == true )
			return -1;
		else if ( load == true )
			return 2;
		else if ( mode == true )
			return 0;
		else
			return 1;
	}
	catch (gcn::Exception e)
	{
 		std::cout << e.getMessage() << std::endl;
		return 1;
	}
 	catch (std::exception e)
	{
 	  std::cout << "Std exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "Unknown exception" << std::endl;
		return 1;
	}

	return 0;
}
