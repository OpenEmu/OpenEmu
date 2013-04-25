#ifndef CYCLE_MAP_H_
#define CYCLE_MAP_H_

#define CYCLE_MAP_SIZE (114 + 9)
extern int CYCLE_MAP_cpu2antic[CYCLE_MAP_SIZE * (17 * 7 + 1)];
extern int CYCLE_MAP_antic2cpu[CYCLE_MAP_SIZE * (17 * 7 + 1)];
void CYCLE_MAP_Create(void);

#endif /* CYCLE_MAP_H_ */
