#ifndef GENS_CPU_SH2_H
#define GENS_CPU_SH2_H

#ifdef __cplusplus
extern "C" {
#endif

extern int MSH2_Speed;
extern int SSH2_Speed;

int MSH2_Init(void);
int SSH2_Init(void);
void MSH2_Reset(void);
void SSH2_Reset(void);
void MSH2_Reset_CPU(void);
void SSH2_Reset_CPU(void);
void _32X_Set_FB(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CPU_SH2_H */
