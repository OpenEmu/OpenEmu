#ifdef NETWORK
/* Network interface */

void MDFNI_NetplayChangeNick(UTF8 *newnick);

/* Call when network play needs to stop. */
void MDFNI_NetplayStop(void);

/* Note:  YOU MUST NOT CALL ANY MDFNI_* FUNCTIONS WHILE IN MDFND_SendData() or
   MDFND_RecvData().
*/

/* Return 0 on failure, 1 on success. */
int MDFND_SendData(const void *data, uint32 len);
int MDFND_RecvData(void *data, uint32 len);

/* Display text received over the network. */
/* NetEcho will be set to TRUE if the displayed text is a network
   echo of what we typed.
*/
void MDFND_NetplayText(const uint8 *text, bool NetEcho);

/* Encode and send text over the network. */
void MDFNI_NetplayText(const uint8 *text);

/* Called when a fatal error occurred and network play can't continue.  This function
   should call MDFNI_NetplayStop() after it has deinitialized the network on the driver
   side.
*/
void MDFND_NetworkClose(void);
#endif

