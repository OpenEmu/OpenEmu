
#ifndef GENS_INPUT_OSX_HPP
#define GENS_INPUT_OSX_HPP

#include "input.hpp"

class Input_OSX: public Input
	{
	public:
		Input_OSX();
		~Input_OSX();
		
		// Update the input subsystem.
		void update(void);
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Get a key. (Used for controller configuration.)
		unsigned int getKey(void);
		
        void pressKey(unsigned player, unsigned key);
        void releaseKey(unsigned player, unsigned key);
	protected:
        uint32 playerMasks[8];
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
	};

#endif
