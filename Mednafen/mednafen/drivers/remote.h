#ifndef __MDFN_DRIVERS_REMOTE_H
#define __MDFN_DRIVERS_REMOTE_H

void CheckForSTDIOMessages(void);
bool InitSTDIOInterface(void);
void Remote_SendStatusMessage(const char *message);
void Remote_SendErrorMessage(const char *message);

#endif
