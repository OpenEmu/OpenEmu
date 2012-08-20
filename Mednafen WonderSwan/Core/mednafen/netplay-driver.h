/* Network interface */

void MDFNI_NetplayChangeNick(UTF8 *newnick);

/* Call when network play needs to stop. */
void MDFNI_NetplayStop(void);

/* Note:  YOU MUST NOT CALL ANY MDFNI_* FUNCTIONS WHILE IN MDFND_SendData() or
   MDFND_RecvData().
*/

void MDFND_SendData(const void *data, uint32 len);	// thrown std::exception will be handled
void MDFND_RecvData(void *data, uint32 len);		// thrown std::exception will be handled

/* Display text received over the network. */
/* NetEcho will be set to TRUE if the displayed text is a network
   echo of what we typed.
*/
void MDFND_NetplayText(const uint8 *text, bool NetEcho);

/* Encode and send text over the network. */
void MDFNI_NetplayText(const uint8 *text);


void MDFNI_NetplayQuit(const char *quit_message);
void MDFNI_NetplaySwap(uint8 a, uint8 b);
void MDFNI_NetplayTake(uint32 mask);
void MDFNI_NetplayDrop(uint32 mask);
void MDFNI_NetplayDupe(uint32 mask);

void MDFNI_NetplayList(void);

/* Starts a process to determine the integrity(synchronization) of all players
   in the current game session on the server.  Outputs result via MDFND_NetplayText()
   when they are received from the server(not in this function).
*/
void MDFNI_NetplayIntegrity(void);

/* Pings the server.  Outputs result via MDFND_NetplayText() when received from the server(not in this function). */
void MDFNI_NetplayPing(void);

/* Called when a fatal error occurred and network play can't continue.  This function
   should call MDFNI_NetplayStop() after it has deinitialized the network on the driver
   side.
*/
void MDFND_NetworkClose(void);
