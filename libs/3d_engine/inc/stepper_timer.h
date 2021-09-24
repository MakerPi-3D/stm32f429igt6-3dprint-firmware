#ifndef STEPPER_TIMER_H
#define STEPPER_TIMER_H

#include "threed_engine.h"
#include "user_common.h"
extern TIM_HandleTypeDef htim4;

inline void stepper_timer_start(void)
{
  HAL_TIM_Base_Start_IT(&htim4);
}

inline void stepper_timer_stop(void)
{
  HAL_TIM_Base_Stop_IT(&htim4);
}

inline void stepper_timer_set_period(uint32_t period_value)
{
//  if (mcu_id == MCU_GD32F450IIH6)
//  {
//    htim4.Instance->ARR = period_value*2;
//    htim4.Init.Period = period_value*2;
//  } else
  {
    htim4.Instance->ARR = period_value;
    htim4.Init.Period = period_value;
  }

}

#endif // STEPPER_TIMER_H

