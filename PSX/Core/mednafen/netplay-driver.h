/* Network interface */

/* Parse and handle a line of UI text(may include / commands) */
void MDFNI_NetplayLine(const char *text, bool &inputable, bool &viewable);

/* Call when network play needs to stop. */
void MDFNI_NetplayStop(void);

/* Note:  YOU MUST NOT CALL ANY MDFNI_* FUNCTIONS WHILE IN MDFND_SendData() or
   MDFND_RecvData().
*/

int MDFND_NetworkConnect(void);

/* Called when a fatal error occurred and network play can't continue.  This function
   should call MDFNI_NetplayStop() after it has deinitialized the network on the driver
   side.
*/
void MDFND_NetworkClose(void);

void MDFND_SendData(const void *data, uint32 len);	// thrown std::exception will be handled
void MDFND_RecvData(void *data, uint32 len);		// thrown std::exception will be handled

/* Display text received over the network. */
/* NetEcho will be set to TRUE if the displayed text is a network
   echo of what we typed.
*/
void MDFND_NetplayText(const uint8 *text, bool NetEcho);

int MDFND_NetworkConnect(void);
