#ifndef __INCclock_stm32f4xx_h
#define __INCclock_stm32f4xx_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

void InitSysClock(void);
void EnablePortClocks(void);
void SetClockDivisors(void);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCclock_stm32f4xx_h */