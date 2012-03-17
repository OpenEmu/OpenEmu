#ifndef GENS_PWM_H
#define GENS_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_BUF_SIZE 4
extern unsigned char PWM_FULL_TAB[PWM_BUF_SIZE * PWM_BUF_SIZE];

extern unsigned short PWM_FIFO_R[8];
extern unsigned short PWM_FIFO_L[8];
extern unsigned int PWM_RP_R;
extern unsigned int PWM_WP_R;
extern unsigned int PWM_RP_L;
extern unsigned int PWM_WP_L;
extern unsigned int PWM_Cycles;
extern unsigned int PWM_Cycle;
extern unsigned int PWM_Cycle_Cnt;
extern unsigned int PWM_Int;
extern unsigned int PWM_Int_Cnt;
extern unsigned int PWM_Mode;
extern unsigned int PWM_Enable;
extern unsigned int PWM_Out_R;
extern unsigned int PWM_Out_L;

void PWM_Init(void);
void PWM_Set_Cycle(unsigned int cycle);
void PWM_Set_Int(unsigned int int_time);
void PWM_Clear_Timer(void);
void PWM_Update_Timer(unsigned int cycle);
void PWM_Update(int **buf, int length);

#ifdef __cplusplus
}
#endif

#endif /* GENS_PWM_H */
