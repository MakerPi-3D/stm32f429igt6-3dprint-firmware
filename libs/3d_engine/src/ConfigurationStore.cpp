//#include "globalvariables.h"
#include "user_ccm.h"
#include "planner.h"
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "ConfigurationStore.h"
#include "Configuration.h"
#include "config_model_tables.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"

int filament_load_unload_temp = 230;     ///< 进退丝温度
int pla_preheat_hotend_temp = 180 ;//170
int pla_preheat_hpb_temp = 70;


int abs_preheat_hotend_temp = 230 ;//170
int abs_preheat_hpb_temp = 100 ;//170

float z_home_retract_mm = 1.0f;

unsigned long axis_steps_per_sqr_second[MAX_NUM_AXIS];

const long dropsegments = DROP_SEGMENTS;

// Calculate the steps/s^2 acceleration rates, based on the mm/s^s
void reset_acceleration_rates()
{
  for (int8_t i = 0; i < ccm_param::motion_3d.axis_num; i++)
  {
    axis_steps_per_sqr_second[i] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[i] * planner_settings.axis_steps_per_mm[i]);
  }
}

void Config_ResetDefault()
{
  float tmp1[MAX_NUM_AXIS] = DEFAULT_AXIS_STEPS_PER_UNIT;
  float tmp2[MAX_NUM_AXIS] = DEFAULT_MAX_FEEDRATE;
  unsigned long tmp3[MAX_NUM_AXIS] = DEFAULT_MAX_ACCELERATION;
  bool buf[] = AXIS_RELATIVE_MODES_X;
  (void)memcpy((void *)planner_settings.axis_relative_modes, buf, sizeof(buf) / sizeof(bool));
  planner_settings.relative_mode = false;

  if (t_sys_data_current.enable_color_mixing)
  {
    float buf1[] = DEFAULT_AXIS_STEPS_PER_UNIT_X;
    float buf2[] = DEFAULT_MAX_FEEDRATE_X;
    long buf3[] = DEFAULT_MAX_ACCELERATION_X;
    (void)memcpy(tmp1, buf1, sizeof(buf1));
    (void)memcpy(tmp2, buf2, sizeof(buf2));
    (void)memcpy(tmp3, buf3, sizeof(buf3));

    if (P2_Pro == t_sys_data_current.model_id) //与默认一样
    {
      float buf2[] = DEFAULT_MAX_FEEDRATE_P2_PRO;
      long buf3[] = DEFAULT_MAX_ACCELERATION_P2_PRO;
      (void)memcpy(tmp2, buf2, sizeof(buf2));
      (void)memcpy(tmp3, buf3, sizeof(buf3));
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_P2_PRO;
      (void)memcpy(tmp1, buf4, sizeof(buf4));
      planner_settings.max_xy_jerk = 5.0f;
      planner_settings.max_z_jerk = 0.2f;
      planner_settings.max_e_jerk = 2.5f;
      planner_settings.max_b_jerk = 2.5f;
    }

    if (P3_Pro == t_sys_data_current.model_id) //与默认一样
    {
      float buf2[] = DEFAULT_MAX_FEEDRATE_P3_PRO;
      long buf3[] = DEFAULT_MAX_ACCELERATION_P3_PRO;
      (void)memcpy(tmp2, buf2, sizeof(buf2));
      (void)memcpy(tmp3, buf3, sizeof(buf3));
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_P3_PRO;
      (void)memcpy(tmp1, buf4, sizeof(buf4));
      planner_settings.max_xy_jerk = 5.0f;
      planner_settings.max_z_jerk = 0.2f;
      planner_settings.max_e_jerk = 2.5f;
      planner_settings.max_b_jerk = 2.5f;
    }

    if (F400TP == t_sys_data_current.model_id) //与默认一样
    {
      float buf2[] = DEFAULT_MAX_FEEDRATE_F400TP;
      long buf3[] = DEFAULT_MAX_ACCELERATION_F400TP;
      (void)memcpy(tmp2, buf2, sizeof(buf2));
      (void)memcpy(tmp3, buf3, sizeof(buf3));
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_F400TP;
      (void)memcpy(tmp1, buf4, sizeof(buf4));
      planner_settings.max_xy_jerk = 5.0f;
      planner_settings.max_z_jerk = 0.2f;
      planner_settings.max_e_jerk = 2.5f;
      planner_settings.max_b_jerk = 2.5f;
    }

    if (F1000TP == t_sys_data_current.model_id) //与默认一样
    {
      float buf2[] = DEFAULT_MAX_FEEDRATE_F1000TP;
      long buf3[] = DEFAULT_MAX_ACCELERATION_F1000TP;
      (void)memcpy(tmp2, buf2, sizeof(buf2));
      (void)memcpy(tmp3, buf3, sizeof(buf3));
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_F1000TP;
      (void)memcpy(tmp1, buf4, sizeof(buf4));
    }
  }

  if (t_sys_data_current.enable_soft_filament) //如果打开了软料功能，则需要改变挤出头电机齿轮直径；
  {
    tmp1[E_AXIS] = E_AXIS_STEPS_PER_UNIT_SOFT;//改变E电机为小齿轮

    if (t_sys_data_current.enable_color_mixing) tmp1[B_AXIS] = B_AXIS_STEPS_PER_UNIT_SOFT; //改变B电机为小齿轮

    USER_DbgLog("t_sys.enable_soft_filament.YES!\r\n");
  }
  else
  {
    USER_DbgLog("t_sys.enable_soft_filament.NO!\r\n");
  }

  for (short i = 0; i < ccm_param::motion_3d.axis_num; i++)
  {
    planner_settings.axis_steps_per_mm[i] = tmp1[i] * ccm_param::motion_3d.step;
    planner_settings.max_feedrate_mm_s[i] = tmp2[i];
    planner_settings.max_acceleration_mm_per_s2[i] = tmp3[i];
  }

  // steps per sq second need to be updated to agree with the units per sq second
  reset_acceleration_rates();
  planner_settings.acceleration = DEFAULT_ACCELERATION;
  planner_settings.retract_acceleration = DEFAULT_RETRACT_ACCELERATION;

  if (t_sys_data_current.enable_color_mixing)
  {
    if (P2_Pro == t_sys_data_current.model_id) //与默认一样
    {
      planner_settings.acceleration = 3000;
      planner_settings.retract_acceleration = 3000;
    }

    if (P3_Pro == t_sys_data_current.model_id) //与默认一样
    {
      planner_settings.acceleration = 1000;
      planner_settings.retract_acceleration = 1000;
    }

    if (F400TP == t_sys_data_current.model_id) //与默认一样
    {
      planner_settings.acceleration = 500;
      planner_settings.retract_acceleration = 500;
    }

    if (F1000TP == t_sys_data_current.model_id) //与默认一样
    {
      planner_settings.acceleration = 500;
      planner_settings.retract_acceleration = 500;
    }
  }

  planner_settings.min_feedrate_mm_s = DEFAULT_MINIMUMFEEDRATE;
  planner_settings.min_segment_time_us = DEFAULT_MINSEGMENTTIME;
  planner_settings.min_travel_feedrate_mm_s = DEFAULT_MINTRAVELFEEDRATE;
  planner_settings.max_xy_jerk = DEFAULT_XYJERK;
  planner_settings.max_z_jerk = DEFAULT_ZJERK;
  planner_settings.max_e_jerk = DEFAULT_EJERK;
  ccm_param::motion_3d.extrude_min_temp = EXTRUDE_MINTEMP;
  //    heater_0_maxtemp = HEATER_0_MAXTEMP;
  pla_preheat_hotend_temp = PLA_PREHEAT_HOTEND_TEMP ;//170
  pla_preheat_hpb_temp = PLA_PREHEAT_HPB_TEMP;
  z_home_retract_mm = Z_HOME_RETRACT_MM;
  abs_preheat_hotend_temp = ABS_PREHEAT_HOTEND_TEMP ;//170
  abs_preheat_hpb_temp = ABS_PREHEAT_HPB_TEMP ;//170

  if (1 == t_sys_data_current.enable_color_mixing)
  {
    planner_settings.max_b_jerk = DEFAULT_BJERK;
  }

  ccm_param::motion_3d.is_open_infrared_z_min_check = false;
}

#ifdef __cplusplus
} //extern "C" {
#endif


