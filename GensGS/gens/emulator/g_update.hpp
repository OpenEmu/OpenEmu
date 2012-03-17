/**
 * Gens: Update_Emulation functions.
 */

#ifndef GENS_UPDATE_HPP
#define GENS_UPDATE_HPP

#ifdef __cplusplus
extern "C" {
#endif

extern int Sleep_Time;

void Reset_Update_Timers(void);

int Update_Emulation(void);
int Update_Emulation_One(void);

#if 0
extern int Kaillera_Error;
int Update_Emulation_Netplay(int player, int num_player);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GENS_UPDATE_H */
