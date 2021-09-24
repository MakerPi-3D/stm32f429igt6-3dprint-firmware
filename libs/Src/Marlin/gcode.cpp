#include "user_common.h"
#include "user_ccm.h"
#include "stepper.h"
#include "planner.h"
#include "temperature.h"
#include "sysconfig_data.h"
#include "gcode.h"
#include "ConfigurationStore.h"
#include "config_model_tables.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim10;

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{
  extern void g29_init(void);

  void init(void)
  {
    planner_init();
    temperature_init(1 == t_sys.enable_cavity_temp);
    st_init();
    //打印机的一些参数初始化
    Config_ResetDefault();

    for (int8_t i = 0; i < MAX_NUM_AXIS; i++)
    {
      if (i < XYZ_NUM_AXIS)
      {
        ccm_param::grbl_destination[i] = ccm_param::motion_3d_model.xyz_home_pos[i];
      }
      else
      {
        ccm_param::grbl_destination[i] = 0.0f;
      }

      ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
    }

    g29_init(); // Bed level init

    // Start steering gear control double head printing on the P2_Pro machine
    if (P2_Pro == t_sys_data_current.model_id)
    {
      if (mcu_id == MCU_GD32F450IIH6)
      {
        HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
        gcode::extruder_1_up();
      }
      else
      {
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
        gcode::extruder_1_up();
      }
    }
  }


}










