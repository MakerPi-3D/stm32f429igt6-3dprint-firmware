#ifndef THREED_ENGINE_H
#define THREED_ENGINE_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#define F_CPU 16000000UL

#define HIGH 0x1
#define LOW  0x0

#ifndef min
  #define min(a,b) ((a)<(b)?(a):(b))
#endif // min

#ifndef max
  #define max(a,b) ((a)>(b)?(a):(b))
#endif // max

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define MAX_NUM_AXIS 7
#define XYZ_NUM_AXIS 5
enum AxisEnum {X_AXIS = 0, X2_AXIS = 1, Y_AXIS  = 2, Z_AXIS = 3, Z2_AXIS = 4, E_AXIS = 5, B_AXIS = 6};
extern const char axis_codes[MAX_NUM_AXIS];

//===========================================================================
//=============================Buffers           ============================
//===========================================================================

// The number of linear motions that can be in the plan at any give time.
// THE BLOCK_BUFFER_SIZE NEEDS TO BE A POWER OF 2, i.g. 8,16,32 because shifts and ors are used to do the ring-buffering.
#define BLOCK_BUFFER_SIZE 16 // maximize block buffer

#define MILLIS() xTaskGetTickCount()
#define TASK_ENTER_CRITICAL()  taskENTER_CRITICAL();
#define TASK_EXIT_CRITICAL()  taskEXIT_CRITICAL();
#define OS_DELAY(TICK_VALUE) osDelay(TICK_VALUE);
#endif

