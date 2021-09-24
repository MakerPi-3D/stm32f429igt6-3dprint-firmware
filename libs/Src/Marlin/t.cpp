#include "gcodebufferhandle.h"
#include "Configuration.h"
#include "threed_engine.h"
#include <math.h>
#include "stepper.h"
#include "user_ccm.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "flashconfig.h"
#include "process_command.h"
#include "planner.h"
#include "user_common.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EXTRUDER_1_Z_MOVE_F400TP 0.0f
#define EXTRUDER_1_Z_MOVE_P2_PRO 0.0f
#define EXTRUDER_1_Z_MOVE_P3_PRO 0.0f

extern float feedrate;
extern uint8_t tmp_extruder;


#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  volatile uint8_t active_extruder = 0;
  volatile uint8_t extruder_toggle = 0;

  static void t0_process(void);
  static void t1_process(void);

  extern void home_axis_process(int axis, bool is_sync_z);

  static void TIM_SetTIM2Compare2(uint32_t compare)
  {
    if (mcu_id == MCU_GD32F450IIH6)
    {
      TIM10->CCR1 = compare;
    }
    else
    {
      //compare = (compare / 0.9 + 50) * 10;
      TIM2->CCR2 = compare;
    }
  }

  void extruder_1_up(void)
  {
    extruder_toggle = 0;

    if (t_sys.is_idex_extruder == 1)
      active_extruder = 0;
    else
    {
      if (P2_Pro == t_sys_data_current.model_id)
      {
        TIM_SetTIM2Compare2(880);
      }
      else if (F400TP == t_sys_data_current.model_id)
      {
        TIM_SetTIM2Compare2(2120);
      }
    }
  }

  void extruder_1_down(void)
  {
    extruder_toggle = 1;

    if (t_sys.is_idex_extruder == 1)
      active_extruder = 1;
    else
    {
      if (P2_Pro == t_sys_data_current.model_id)
      {
        TIM_SetTIM2Compare2(2120);
      }
      else if (F400TP == t_sys_data_current.model_id)
      {
        TIM_SetTIM2Compare2(880);
      }
    }
  }

  void extruder_0_1_toggle(void)
  {
    if (0 == extruder_toggle)
    {
      if (t_sys.is_idex_extruder == 1)
      {
        st_enable_endstops(true);
        home_axis_process(X_AXIS, true);
        st_enable_endstops(false);
        extruder_1_down();
        t1_process();
      }
      else
      {
        extruder_1_down();
      }
    }
    else if (1 == extruder_toggle)
    {
      if (t_sys.is_idex_extruder == 1)
      {
        st_enable_endstops(true);
        home_axis_process(X_AXIS, true);
        st_enable_endstops(false);
        extruder_1_up();
        t0_process();
      }
      else
      {
        extruder_1_up();
      }
    }
  }



  static void t0_process(void)
  {
    if (t_sys.is_idex_extruder == 1 && gcode::g28_complete_flag)
    {
      ccm_param::grbl_destination[X_AXIS] = 0;
      ccm_param::grbl_current_position[X_AXIS] = ccm_param::grbl_destination[X_AXIS];
      planner_set_position(ccm_param::grbl_current_position);
    }

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      st_synchronize();
      extruder_1_up();
      osDelay(1000);
      //      plan_set_process_auto_bed_level_status(false);
      compensation_destination_dual(-1, X_AXIS);
      compensation_destination_dual(-1, Y_AXIS);
      compensation_destination_dual(-1, Z_AXIS);
      feedrate = homing_feedrate[Z_AXIS];
      process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
      st_synchronize();

      for (int i = 0; i < XYZ_NUM_AXIS; i++)
      {
        ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
      }

      if (P2_Pro == t_sys_data_current.model_id)
      {
        feedrate = homing_feedrate[X_AXIS];
        float x = ccm_param::grbl_destination[0];
        ccm_param::grbl_destination[0] = 245;//ccm_param::motion_3d_model.xyz_max_pos[X_AXIS];
        process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
        st_synchronize();
        osDelay(1000);
        ccm_param::grbl_destination[0] = x;
        process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
        st_synchronize();
      }

      //      planner_set_position(ccm_param::grbl_current_position);
      //      plan_set_process_auto_bed_level_status(true);
    }
  }


  static void t1_process(void)
  {
    if (t_sys.is_idex_extruder == 1 && gcode::g28_complete_flag)
    {
      ccm_param::grbl_destination[X_AXIS] = ccm_param::motion_3d_model.xyz_max_pos[X_AXIS];
      ccm_param::grbl_current_position[X_AXIS] = ccm_param::grbl_destination[X_AXIS];
      planner_set_position(ccm_param::grbl_current_position);
    }

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      //      plan_set_process_auto_bed_level_status(false);
      compensation_destination_dual(1, X_AXIS);
      compensation_destination_dual(1, Y_AXIS);
      compensation_destination_dual(1, Z_AXIS);
      feedrate = homing_feedrate[Z_AXIS];
      process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
      st_synchronize();

      for (int i = 0; i < XYZ_NUM_AXIS; i++)
      {
        ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
      }

      if (P2_Pro == t_sys_data_current.model_id)
      {
        feedrate = homing_feedrate[X_AXIS];
        float x = ccm_param::grbl_destination[0];
        ccm_param::grbl_destination[0] = 0;
        process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
        st_synchronize();
        osDelay(1000);
        ccm_param::grbl_destination[0] = x;
        process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
        st_synchronize();
      }

      //      planner_set_position(ccm_param::grbl_current_position);
      //      plan_set_process_auto_bed_level_status(true);
      extruder_1_down();
      osDelay(1000);
    }
  }

  static void idex_x_move_to_home(int axis)
  {
    st_synchronize();

    for (int i = 0; i < MAX_NUM_AXIS; i++)
    {
      ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
    }

//    planner_set_position(ccm_param::grbl_destination);
    // 喷嘴远离平台，避免调平导致刮平台
    ccm_param::grbl_destination[Z_AXIS] = ccm_param::grbl_destination[Z_AXIS] + 5;
    feedrate = homing_feedrate[Z_AXIS];
    process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
    st_synchronize();
    // X轴回零
    st_enable_endstops(true) ;
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    feedrate = homing_feedrate[axis];
    process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
    st_synchronize();
    st_enable_endstops(false) ;
    // 喷嘴返回Z位置
    ccm_param::grbl_destination[Z_AXIS] = ccm_param::grbl_destination[Z_AXIS] - 5;
    feedrate = homing_feedrate[Z_AXIS];
    process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
    st_synchronize();
  }

  bool t_process(uint8_t switch_extruder, bool is_process_t)
  {
    if (switch_extruder == active_extruder) return false;

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
    {
      if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY ||
          t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR) // idex复制模式、镜像模式不切换喷嘴id
      {
        return false;
      }
    }

    active_extruder = switch_extruder;

    if (is_process_t)
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        if (t_sys.is_idex_extruder == 1 && t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL) // idex结构
        {
          if (0 == switch_extruder)
          {
            idex_x_move_to_home(X2_AXIS);
          }
          else if (1 == switch_extruder)
          {
            idex_x_move_to_home(X_AXIS);
          }

          for (int i = 0; i < XYZ_NUM_AXIS; i++)
          {
            ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
          }
        }
        else
        {
          if (0 == switch_extruder)
          {
            st_enable_endstops(true);
            home_axis_process(X_AXIS, true);
            st_enable_endstops(false);
            t0_process();
          }
          else if (1 == switch_extruder)
          {
            //      st_enable_endstops(true);
            //      home_axis_process(X_AXIS, true);
            //      st_enable_endstops(false);
            ccm_param::grbl_destination[X_AXIS] = 0;
            feedrate = homing_feedrate[X_AXIS];
            process_buffer_line_normal(ccm_param::grbl_destination, feedrate / 60);
            st_synchronize();
            t1_process();
          }
        }
      }
    }

    return true;
  }


  void t_process(void)
  {
    bool is_process_t = true;
    tmp_extruder = (unsigned char)parseGcodeBufHandle.codeValue();

    // S-1 只变更active_extruder
    if (parseGcodeBufHandle.codeSeen('S') && parseGcodeBufHandle.codeValue() == -1)
    {
      is_process_t = false;
    }

    switch (tmp_extruder)
    {
    case 0:
    case 1:
      if (!t_process(tmp_extruder, is_process_t)) break;

      break;

    default:
      break;
    }
  }

}

