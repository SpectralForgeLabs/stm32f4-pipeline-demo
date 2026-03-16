#ifndef __INCtimer_h
#define __INCtimer_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */

static inline __attribute__((always_inline)) uint32_t usec_time_get(void)
{
    return TIM2->CNT;
}

void Timer2Init(uint32_t frequency);
void Timer2Stop(void);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCtimer_h */