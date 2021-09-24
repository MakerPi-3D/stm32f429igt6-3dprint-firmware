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
#include "RespondGUI.h"
#include "config_model_tables.h"
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

  static void dual_idex_set_xyz_dest(const int axis, const float value)
  {
    bool is_relative_mode = (bool)(planner_settings.axis_relative_modes[axis] || planner_settings.relative_mode);
    ccm_param::grbl_destination[axis] = value + (float)is_relative_mode * plan_get_current_save_xyz(axis);
  }

  static void dual_idex_set_eb_dest(const int axis, const float value)
  {
    bool is_relative_mode = (bool)(planner_settings.axis_relative_modes[axis] || planner_settings.relative_mode);
    ccm_param::grbl_destination[axis] = value + (float)is_relative_mode * ccm_param::grbl_current_position[axis];
  }

  static void dual_idex_axis_normal(const int axis, const float value)
  {
    if (axis == E_AXIS)
    {
      if (1 == gcode::active_extruder)
      {
        dual_idex_set_eb_dest(B_AXIS, value);
      }
      else if (0 == gcode::active_extruder)
      {
        dual_idex_set_eb_dest(E_AXIS, value);
      }
    }
    else if (axis == X_AXIS)
    {
      if (1 == gcode::active_extruder)
      {
        dual_idex_set_xyz_dest(X2_AXIS, value);

        if (IsPrint())
        {
          ccm_param::grbl_destination[X2_AXIS] += flash_param_t.idex_extruder_0_bed_offset[0] - flash_param_t.idex_extruder_1_bed_offset[0];
        }
      }
      else if (0 == gcode::active_extruder)
      {
        dual_idex_set_xyz_dest(X_AXIS, value);

        if (IsPrint())
        {
          ccm_param::grbl_destination[X_AXIS] += flash_param_t.idex_extruder_0_bed_offset[0];
        }
      }
    }
  }

  static void dual_idex_axis_copy(const int axis, const float value)
  {
    if (axis == E_AXIS)
    {
      dual_idex_set_eb_dest(E_AXIS, value);
      dual_idex_set_eb_dest(B_AXIS, value);
    }
    else if (axis == X_AXIS)
    {
      dual_idex_set_eb_dest(X_AXIS, value);
      dual_idex_set_eb_dest(X2_AXIS, value);

      if (IsPrint() || IsResumePrint() || IsPausePrint())
      {
        ccm_param::grbl_destination[X_AXIS] += flash_param_t.idex_extruder_0_bed_offset[0];
        ccm_param::grbl_destination[X2_AXIS] += flash_param_t.idex_extruder_1_bed_offset[0] + fabs(flash_param_t.idex_extruder_1_bed_offset[1] - flash_param_t.idex_extruder_1_bed_offset[0]) / 2;
      }
    }
  }

  static void dual_idex_axis_mirror(const int axis, const float value)
  {
    if (axis == E_AXIS)
    {
      dual_idex_set_eb_dest(E_AXIS, value);
      dual_idex_set_eb_dest(B_AXIS, value);
    }
    else if (axis == X_AXIS)
    {
      dual_idex_set_eb_dest(X_AXIS, value);
      dual_idex_set_eb_dest(X2_AXIS, value);

      if (IsPrint() || IsResumePrint() || IsPausePrint())
      {
        ccm_param::grbl_destination[X_AXIS] += flash_param_t.idex_extruder_0_bed_offset[0];
        ccm_param::grbl_destination[X2_AXIS] = flash_param_t.idex_extruder_1_bed_offset[1] - ccm_param::grbl_destination[X2_AXIS];
      }
    }
  }


  void get_coordinates_dual_idex(const int axis, const float value)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
    {
      if (axis == E_AXIS || axis == X_AXIS)
      {
        if (t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
        {
          dual_idex_axis_normal(axis, value);
        }
        else if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY)
        {
          dual_idex_axis_copy(axis, value);
        }
        else if (t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
        {
          dual_idex_axis_mirror(axis, value);
        }
      }
      else if (axis == B_AXIS)
      {
        if (t_sys.idex_print_type != IDEX_PRINT_TYPE_COPY && t_sys.idex_print_type != IDEX_PRINT_TYPE_MIRROR)
        {
          dual_idex_set_eb_dest(axis, value); // B
        }
      }
      else if (axis == X2_AXIS || axis == Z2_AXIS)  // 跳过
      {
      }
      else
      {
        dual_idex_set_xyz_dest(axis, value); // YZ
      }
    }
  }

  void clamp_to_software_endstops_dual_idex(volatile float (&target)[MAX_NUM_AXIS], int &IsPopWarningInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
    {
      if (t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
      {
        if (0 == gcode::active_extruder)
        {
          // 避免撞击喷头2
          if (target[X_AXIS] > flash_param_t.idex_extruder_0_bed_offset[1])
            IsPopWarningInterface = XMaxLimitWarning;
        }
        else if (1 == gcode::active_extruder)
        {
          // 避免撞击喷头1
          if (target[X2_AXIS] < flash_param_t.idex_extruder_1_bed_offset[0])
            IsPopWarningInterface = XMinLimitWarning;
        }
      }
      else if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY || t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
      {
        if (IsPrint())
        {
          // 避免撞击喷头
          if (target[X_AXIS] > flash_param_t.idex_extruder_0_bed_offset[0] + fabs(flash_param_t.idex_extruder_0_bed_offset[1] - flash_param_t.idex_extruder_0_bed_offset[0]) / 2)
            IsPopWarningInterface = XMaxLimitWarning;

          if (target[X2_AXIS] < flash_param_t.idex_extruder_1_bed_offset[0] + fabs(flash_param_t.idex_extruder_1_bed_offset[1] - flash_param_t.idex_extruder_1_bed_offset[0]) / 2)
            IsPopWarningInterface = XMinLimitWarning;
        }
      }

      if (IsPopWarningInterface != -1)
      {
        target[X_AXIS] = ccm_param::grbl_current_position[X_AXIS];
        target[X2_AXIS] = ccm_param::grbl_current_position[X2_AXIS];
        target[Y_AXIS] = ccm_param::grbl_current_position[Y_AXIS];
        target[Z_AXIS] = ccm_param::grbl_current_position[Z_AXIS];
      }

      if (IsPrint())
      {
        PopWarningInfo(IsPopWarningInterface);
      }
    }
  }

  // Steering gear, upper and lower z-axis
  static void compensation_destination_dual_basic(const int plus_or_minus, const int axis)
  {
    if (X_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * flash_param_t.dual_extruder_1_offset[0];
    }
    else if (Y_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * flash_param_t.dual_extruder_1_offset[1];
    }
    else if (Z_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * flash_param_t.dual_extruder_1_offset[2];
    }
  }

  static void compensation_destination_idex_basic(const int plus_or_minus, const int axis, const float x_offset)
  {
    if (X_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * (flash_param_t.idex_ext1_ext0_offset[0] + x_offset);
    }
    else if (X2_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * (flash_param_t.idex_ext1_ext0_offset[0] + x_offset);
    }
    else if (Y_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * flash_param_t.idex_ext1_ext0_offset[1];
    }
    else if (Z_AXIS == axis)
    {
      ccm_param::grbl_destination[axis] = ccm_param::grbl_destination[axis] + plus_or_minus * flash_param_t.idex_ext1_ext0_offset[2];
    }
  }


  void compensation_destination_idex(const int plus_or_minus, const int axis)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (t_sys.is_idex_extruder == 1)
      {
        if (P3_Pro == t_sys_data_current.model_id)
        {
          compensation_destination_idex_basic(plus_or_minus, axis, 0.0f);
        }
        else if (F400TP == t_sys_data_current.model_id)
        {
          compensation_destination_idex_basic(plus_or_minus, axis, 0.0f);
        }
      }
    }
  }

  void compensation_destination_dual(const int plus_or_minus, const int axis)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (t_sys.is_idex_extruder != 1)
      {
        if (P2_Pro == t_sys_data_current.model_id)
        {
          compensation_destination_dual_basic(plus_or_minus, axis);
        }
      }
    }
  }

  // IDEX double x structure, compensation XYZ value
  static void get_coordinates_idex_comp_ext1_dest(void)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1 && gcode::active_extruder == 1)
    {
      if (t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
      {
        if (gcode::active_extruder == 1)
        {
          if (parseGcodeBufHandle.codeSeen('X'))
          {
            compensation_destination_idex(1, X2_AXIS);
          }

          if (parseGcodeBufHandle.codeSeen('Y'))
          {
            compensation_destination_idex(1, Y_AXIS);
          }

          if (parseGcodeBufHandle.codeSeen('Z'))
          {
            compensation_destination_idex(1, Z_AXIS);
          }
        }
      }
    }
  }

  // Steering gear structure, compensation XYZ value
  static void get_coordinates_dual_comp_ext1_dest(void)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && gcode::active_extruder == 1)
    {
      if (parseGcodeBufHandle.codeSeen('X'))
      {
        compensation_destination_dual(1, X_AXIS);
      }

      if (parseGcodeBufHandle.codeSeen('Y'))
      {
        compensation_destination_dual(1, Y_AXIS);
      }

      if (parseGcodeBufHandle.codeSeen('Z'))
      {
        compensation_destination_dual(1, Z_AXIS);
      }
    }
  }

  void get_coordinates_comp_ext1_dest(void)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && gcode::active_extruder == 1)
    {
      if (t_sys.is_idex_extruder == 1)
      {
        gcode::get_coordinates_idex_comp_ext1_dest();
      }
      else
      {
        gcode::get_coordinates_dual_comp_ext1_dest();
      }
    }
  }


}

