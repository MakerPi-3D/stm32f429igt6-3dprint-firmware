#ifndef TEMPERATURE_TIMER_H
#define TEMPERATURE_TIMER_H

#include "threed_engine.h"
extern TIM_HandleTypeDef htim3;

inline void temperature_timer_start(void)
{
  HAL_TIM_Base_Start_IT(&htim3);
}

inline void temperature_timer_stop(void)
{
  HAL_TIM_Base_Stop_IT(&htim3);
}

#endif // TEMPERATURE_TIMER_H

