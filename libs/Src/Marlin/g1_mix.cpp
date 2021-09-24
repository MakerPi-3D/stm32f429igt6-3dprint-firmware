#include "gcodebufferhandle.h"
#include "Configuration.h"
#include "threed_engine.h"
#include <math.h>
#include "sysconfig_data.h"
#include "user_ccm.h"
#include "config_motion_3d.h"
#include "stepper.h"
#include "process_command.h"
#include "planner.h"
#include "globalvariables.h"
#include "ConfigurationStore.h"
#include "gcode.h"
#include "flashconfig.h"
#include "PrintControl.h"
#include "user_common.h"
#include "RespondGUI.h"
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long previous_xTaskGetTickCount_cmd;
extern float feedrate;
extern int feedmultiply;
extern int extrudemultiply;
extern uint32_t SDPos;
extern float add_homeing[XYZ_NUM_AXIS];
#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  static void mix_set_xyz_dest(const int axis, const float value)
  {
    bool is_relative_mode = (bool)(planner_settings.axis_relative_modes[axis] || planner_settings.relative_mode);
    ccm_param::grbl_destination[axis] = value + (float)is_relative_mode * plan_get_current_save_xyz(axis);
  }

  static void mix_set_eb_dest_value(const int axis, const float value)
  {
    bool is_relative_mode = (bool)(planner_settings.axis_relative_modes[axis] || planner_settings.relative_mode);
    ccm_param::grbl_destination[axis] = value + (float)is_relative_mode * ccm_param::grbl_current_position[axis];
  }

  static volatile int current_layer_save = 0;
  static volatile float factor_save = 0.0f;
  static volatile float e_factor_save = 0.0f;
  static volatile float b_factor_save = 0.0f;
  static volatile float value_save = 0.0f;
  static volatile float e_value_save = 0.0f;
  static volatile float b_value_save = 0.0f;
  static void mix_axis_gradient_color(const int axis, const float value)
  {
    if (gcode::layer_count != 0)
    {
      if (gcode::current_layer < 0) // 底座
      {
        e_value_save = value * 0.5f;
        b_value_save = value * 0.5f;
        mix_set_eb_dest_value(E_AXIS, e_value_save);
        mix_set_eb_dest_value(B_AXIS, b_value_save);
        current_layer_save = gcode::current_layer;
        value_save = value;
      }
      else // 本体
      {
        if (current_layer_save != gcode::current_layer) // 换层，重设比例
        {
          factor_save = (float)gcode::current_layer / (float)gcode::layer_count;

          // 喷头比例不可小于5%，避免堵头
          if (factor_save < 0.05f) factor_save = 0.05f;
          else if (factor_save > 0.95f) factor_save = 0.95f;

          // 设置EB比例
          e_factor_save = factor_save;
          b_factor_save = 1.0f - factor_save;
          // 按当前比例，重设EB当前坐标
          ccm_param::grbl_destination[(int)E_AXIS] = value_save * e_factor_save;
          ccm_param::grbl_destination[(int)B_AXIS] = value_save * b_factor_save;
          ccm_param::grbl_current_position[(int)E_AXIS] = ccm_param::grbl_destination[(int)E_AXIS];
          ccm_param::grbl_current_position[(int)B_AXIS] = ccm_param::grbl_destination[(int)B_AXIS];
          planner_set_axis_position(ccm_param::grbl_current_position[E_AXIS], E_AXIS);
          planner_set_axis_position(ccm_param::grbl_current_position[B_AXIS], B_AXIS);
          st_synchronize();
          current_layer_save = gcode::current_layer;
        }

        e_value_save = value * e_factor_save;
        b_value_save = value * b_factor_save;
        mix_set_eb_dest_value(E_AXIS, e_value_save);
        mix_set_eb_dest_value(B_AXIS, b_value_save);
        value_save = value;
      }
    }
    else
    {
      e_value_save = value * 0.5f;
      b_value_save = value * 0.5f;
      mix_set_eb_dest_value(E_AXIS, e_value_save);
      mix_set_eb_dest_value(B_AXIS, b_value_save);
      current_layer_save = -1;
      value_save = value;
    }
  }

  static void mix_axis_fix_proportion(const int axis, const float value)
  {
    float factor = 0.5f;

    if (gcode::current_layer < 0) // 底座
    {
      e_value_save = value * 0.5f;
      b_value_save = value * 0.5f;
      mix_set_eb_dest_value(E_AXIS, e_value_save);
      mix_set_eb_dest_value(B_AXIS, b_value_save);
      value_save = value;
    }
    else // 本体
    {
      e_value_save = value * factor;
      b_value_save = value * (1 - factor);
      mix_set_eb_dest_value(E_AXIS, e_value_save);
      mix_set_eb_dest_value(B_AXIS, b_value_save);
      value_save = value;
    }
  }

  static void mix_axis_fix_random(const int axis, const float value)
  {
    static volatile float value_save = 0.0f;

    if (gcode::layer_count != 0)
    {
      if (gcode::current_layer < 0) // 底座
      {
        e_value_save = value * 0.5f;
        b_value_save = value * 0.5f;
        mix_set_eb_dest_value(E_AXIS, e_value_save);
        mix_set_eb_dest_value(B_AXIS, b_value_save);
        value_save = value;
      }
      else // 本体
      {
        int max = 80;
        int min = 20;
        float factor = (float)(rand() % (max - min) + min) / 100.0f;
        ccm_param::grbl_destination[(int)E_AXIS] = value_save * factor;
        ccm_param::grbl_destination[(int)B_AXIS] = value_save * (1 - factor);
        ccm_param::grbl_current_position[(int)E_AXIS] = ccm_param::grbl_destination[(int)E_AXIS];
        ccm_param::grbl_current_position[(int)B_AXIS] = ccm_param::grbl_destination[(int)B_AXIS];
        planner_set_axis_position(ccm_param::grbl_current_position[E_AXIS], E_AXIS);
        planner_set_axis_position(ccm_param::grbl_current_position[B_AXIS], B_AXIS);
        e_value_save = value * factor;
        b_value_save = value * (1 - factor);
        mix_set_eb_dest_value(E_AXIS, e_value_save);
        mix_set_eb_dest_value(B_AXIS, b_value_save);
        value_save = value;
      }
    }
    else
    {
      e_value_save = value * 0.5f;
      b_value_save = value * 0.5f;
      mix_set_eb_dest_value(E_AXIS, e_value_save);
      mix_set_eb_dest_value(B_AXIS, b_value_save);
      value_save = value;
    }
  }


  void get_coordinates_mix(const int axis, const float value)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      if (axis == E_AXIS)
      {
        if (!IsPrint())
        {
          mix_set_eb_dest_value(axis, value); // E,进丝退丝
        }
        else
        {
          if (t_sys.mix_print_type == MIX_PRINT_TYPE_GRADIENT_COLOR)
          {
            mix_axis_gradient_color(axis, value);
          }
          else if (t_sys.mix_print_type == MIX_PRINT_TYPE_FIX_PROPORTION)
          {
            mix_axis_fix_proportion(axis, value);
          }
          else if (t_sys.mix_print_type == MIX_PRINT_TYPE_RANDOM)
          {
            mix_axis_fix_random(axis, value);
          }
        }
      }
      else if (axis == B_AXIS)
      {
        if (!IsPrint())
        {
          mix_set_eb_dest_value(axis, value); // B,进丝退丝
        }
      }
      else if (axis == X_AXIS)
      {
        mix_set_xyz_dest(X_AXIS, value);

        if (IsPrint())
        {
          ccm_param::grbl_destination[X_AXIS] += flash_param_t.mix_extruder_0_bed_offset[0];
        }
      }
      else if (axis == X2_AXIS || axis == Z2_AXIS)  // 跳过
      {
      }
      else
      {
        mix_set_xyz_dest(axis, value); // XYZ
      }
    }
  }

  void clamp_to_software_endstops_mix(volatile float (&target)[MAX_NUM_AXIS], int &IsPopWarningInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      if (0 == gcode::active_extruder)
      {
        // 避免撞击
        if (target[X_AXIS] > flash_param_t.mix_extruder_0_bed_offset[1])
          IsPopWarningInterface = XMaxLimitWarning;
      }

      if (IsPopWarningInterface != -1)
      {
        target[X_AXIS] = ccm_param::grbl_current_position[X_AXIS];
        target[Y_AXIS] = ccm_param::grbl_current_position[Y_AXIS];
        target[Z_AXIS] = ccm_param::grbl_current_position[Z_AXIS];
      }

      if (IsPrint())
      {
        PopWarningInfo(IsPopWarningInterface);
      }
    }
  }



}

