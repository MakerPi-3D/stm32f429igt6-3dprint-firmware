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
#include "gcode.h"
#include "flashconfig.h"
#include "PrintControl.h"
#ifdef __cplusplus
extern "C" {
#endif

extern float add_homeing[XYZ_NUM_AXIS];

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  static void g92_set_axis_value(const int axis)
  {
    //    t_gui.used_total_material += (unsigned long)ccm_param::grbl_current_position[axis];
    ccm_param::grbl_current_position[axis] = parseGcodeBufHandle.codeValue();
    ccm_param::grbl_destination[axis] = parseGcodeBufHandle.codeValue();

    if (axis == Z_AXIS)
    {
      ccm_param::grbl_current_position[Z2_AXIS] = parseGcodeBufHandle.codeValue();
      ccm_param::grbl_destination[Z2_AXIS] = parseGcodeBufHandle.codeValue();
    }

    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level && axis < XYZ_NUM_AXIS)
    {
      planner_set_position(ccm_param::grbl_current_position);
    }
    else
    {
      planner_set_axis_position(ccm_param::grbl_current_position[axis], axis);
    }
  }

  static void g92_process_idex_normal(const int axis)
  {
    if (axis == (int)E_AXIS)
    {
      if (1 == gcode::active_extruder)
      {
        g92_set_axis_value(B_AXIS);
      }
      else if (0 == gcode::active_extruder)
      {
        g92_set_axis_value(axis);
      }
    }
    else  if (axis == (int)B_AXIS)
    {
      if (!IsPrint())
      {
        g92_set_axis_value(axis);
      }
    }
    else if (axis == (int)X_AXIS)
    {
      if (1 == gcode::active_extruder)
      {
        g92_set_axis_value(X2_AXIS);
      }
      else if (0 == gcode::active_extruder)
      {
        g92_set_axis_value(axis);
      }
    }
    else if (axis == (int)Z_AXIS)
    {
      g92_set_axis_value(Z_AXIS);
    }
    else
    {
      ccm_param::grbl_current_position[axis] = parseGcodeBufHandle.codeValue() + add_homeing[axis];
      planner_set_position(ccm_param::grbl_current_position);
    }
  }

  static void g92_process_idex_copy_mirror(const int axis)
  {
    if (axis == (int)E_AXIS)
    {
      if (0 == gcode::active_extruder)
      {
        g92_set_axis_value(axis);
        g92_set_axis_value(B_AXIS);
      }
    }
    else if (axis == (int)X_AXIS)
    {
      if (0 == gcode::active_extruder)
      {
        g92_set_axis_value(axis);
      }
    }
    else if (axis == (int)Z_AXIS)
    {
      g92_set_axis_value(Z_AXIS);
    }
    else
    {
      ccm_param::grbl_current_position[axis] = parseGcodeBufHandle.codeValue() + add_homeing[axis];
      planner_set_position(ccm_param::grbl_current_position);
    }
  }


  static void g92_process_idex(const int axis)
  {
    if (t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
    {
      g92_process_idex_normal(axis);
    }
    else if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY || t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
    {
      g92_process_idex_copy_mirror(axis);
    }
  }


  static void g92_process_mix(const int axis)
  {
    if (axis == (int)E_AXIS)
    {
      if (IsPrint())
      {
        g92_set_axis_value(axis);
        g92_set_axis_value(B_AXIS);
      }
      else
      {
        g92_set_axis_value(axis); //load and unload filament
      }
    }
    else if (axis == (int)B_AXIS)
    {
      if (!IsPrint())
      {
        g92_set_axis_value(axis); //load and unload filament
      }
    }
    else if (axis == (int)Z_AXIS)
    {
      g92_set_axis_value(Z_AXIS);
    }
    else
    {
      ccm_param::grbl_current_position[axis] = parseGcodeBufHandle.codeValue() + add_homeing[axis];
      planner_set_position(ccm_param::grbl_current_position);
    }
  }

  void g92_process(void)
  {
    if (!parseGcodeBufHandle.codeSeen(axis_codes[E_AXIS]) && !parseGcodeBufHandle.codeSeen(axis_codes[B_AXIS]))
    {
      while (planner_moves_planned() > 0)
        OS_DELAY(50);
    }

    for (int8_t i = 0; i < ccm_param::motion_3d.axis_num; i++)
    {
      if (parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
        {
          g92_process_idex(i);
        }
        else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
        {
          g92_process_mix(i);
        }
        else
        {
          if (i == (int)E_AXIS)
          {
            if (1 == gcode::active_extruder)
            {
              g92_set_axis_value(B_AXIS);
            }
            else if (0 == gcode::active_extruder)
            {
              g92_set_axis_value(i);
            }
          }
          else if (i == (int)B_AXIS)
          {
            g92_set_axis_value(i);
          }
          else if (i == (int)Z_AXIS)
          {
            g92_set_axis_value(Z_AXIS);
          }
          else
          {
            ccm_param::grbl_current_position[i] = parseGcodeBufHandle.codeValue() + add_homeing[i];
            planner_set_position(ccm_param::grbl_current_position);
          }
        }
      }
    }
  }


}

