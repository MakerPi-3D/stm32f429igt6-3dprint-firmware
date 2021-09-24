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

  static void laser_set_xyz_dest(const int axis, const float value)
  {
    bool is_relative_mode = (bool)(planner_settings.axis_relative_modes[axis] || planner_settings.relative_mode);
    ccm_param::grbl_destination[axis] = value + (float)is_relative_mode * plan_get_current_save_xyz(axis);
  }

  void get_coordinates_laser(const int axis, const float value)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      if (axis == X_AXIS)
      {
        if (1 == gcode::active_extruder)
        {
          laser_set_xyz_dest(X2_AXIS, value);

          if (IsPrint())
          {
            ccm_param::grbl_destination[X2_AXIS] += flash_param_t.laser_extruder_1_bed_offset[0];
          }
        }
        else if (0 == gcode::active_extruder)
        {
          if (!IsPrint())
          {
            laser_set_xyz_dest(X_AXIS, value);
          }
        }
      }
      else if (axis == X2_AXIS || axis == Z2_AXIS || axis == E_AXIS || axis == B_AXIS) // 跳过
      {
      }
      else if (axis == Y_AXIS)
      {
        laser_set_xyz_dest(Y_AXIS, value);

        if (IsPrint())
        {
          ccm_param::grbl_destination[Y_AXIS] += flash_param_t.laser_ext0_home_pos_adding[1];
        }
      }
      else
      {
        laser_set_xyz_dest(axis, value); // Z
      }
    }
  }

  void clamp_to_software_endstops_laser(volatile float (&target)[MAX_NUM_AXIS], int &IsPopWarningInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      if (1 == gcode::active_extruder)
      {
        // 避免撞击
        if (target[X2_AXIS] < flash_param_t.laser_extruder_1_bed_offset[0])
          IsPopWarningInterface = XMinLimitWarning;
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

