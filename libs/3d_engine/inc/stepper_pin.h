#ifndef STEPPER_PIN_H
#define STEPPER_PIN_H
#include "user_common.h"
#include "threed_engine.h"
#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "globalvariables.h"
#include "user_ccm.h"
#include "user_board_pin.h"
#include "gcode.h"
// coarse Endstop Settings
#define ENDSTOPPULLUPS // Comment this out (using // at the start of the line) to disable the endstop pullup resistors
#ifdef ENDSTOPPULLUPS
  #define ENDSTOPPULLUP_XMAX
  #define ENDSTOPPULLUP_X2MAX
  #define ENDSTOPPULLUP_YMAX
  #define ENDSTOPPULLUP_ZMAX
  #define ENDSTOPPULLUP_XMIN
  #define ENDSTOPPULLUP_X2MIN
  #define ENDSTOPPULLUP_YMIN
  #define ENDSTOPPULLUP_ZMIN
#endif

// The pullups are needed if you directly connect a mechanical endswitch between the signal and ground pins.
#define X_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define X2_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Y_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z2_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define X_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define X2_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Y_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z2_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.

// For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1
#define X_ENABLE_ON 0
#define X2_ENABLE_ON 0
#define Y_ENABLE_ON 0
#define Z_ENABLE_ON 0
#define Z2_ENABLE_ON 0
#define E_ENABLE_ON 0
#define B_ENABLE_ON 0 // For all extruders

#define X_ENABLE_OFF 1
#define X2_ENABLE_OFF 1
#define Y_ENABLE_OFF 1
#define Z_ENABLE_OFF 1
#define Z2_ENABLE_OFF 1
#define E_ENABLE_OFF 1
#define B_ENABLE_OFF 1 // For all extruders

// Disables axis when it's not being used.
#define DISABLE_X false
#define DISABLE_X2 false
#define DISABLE_Y false
#define DISABLE_Z false
#define DISABLE_Z2 false
#define DISABLE_E false
#define DISABLE_B false// For all extruders
#define DISABLE_INACTIVE_EXTRUDER true //disable only inactive extruders and keep active extruder enabled

//By default pololu step drivers require an active high signal. However, some high power drivers require an active low signal as step.
#define INVERT_X_STEP_PIN false
#define INVERT_X2_STEP_PIN false
#define INVERT_Y_STEP_PIN false
#define INVERT_Z_STEP_PIN false
#define INVERT_Z2_STEP_PIN false
#define INVERT_E_STEP_PIN false
#define INVERT_B_STEP_PIN false//COLOR_MIXING

#ifdef __cplusplus
extern "C" {
#endif

const static bool axis_enable[MAX_NUM_AXIS] = {X_ENABLE_ON, X2_ENABLE_ON, Y_ENABLE_ON, Z_ENABLE_ON, Z2_ENABLE_ON, E_ENABLE_ON, B_ENABLE_ON};
const static bool axis_disable[MAX_NUM_AXIS] = {X_ENABLE_OFF, X2_ENABLE_OFF, Y_ENABLE_OFF, Z_ENABLE_OFF, Z2_ENABLE_OFF, E_ENABLE_OFF, B_ENABLE_OFF};
const static bool endstop_min_inverting[XYZ_NUM_AXIS] = {X_MIN_ENDSTOP_INVERTING, X2_MIN_ENDSTOP_INVERTING, Y_MIN_ENDSTOP_INVERTING, Z_MIN_ENDSTOP_INVERTING, Z2_MIN_ENDSTOP_INVERTING};
const static bool endstop_max_inverting[XYZ_NUM_AXIS] = {X_MAX_ENDSTOP_INVERTING, X2_MAX_ENDSTOP_INVERTING, Y_MAX_ENDSTOP_INVERTING, Z_MAX_ENDSTOP_INVERTING, Z2_MAX_ENDSTOP_INVERTING};
const static bool invert_axis_step_pin[MAX_NUM_AXIS] = {INVERT_X_STEP_PIN, INVERT_X2_STEP_PIN, INVERT_Y_STEP_PIN, INVERT_Z_STEP_PIN, INVERT_Z2_STEP_PIN, INVERT_E_STEP_PIN, INVERT_B_STEP_PIN};

inline void stepper_axis_enable(int axis, bool isEnable)
{
  bool value = (isEnable ? axis_enable[axis] : axis_disable[axis]);
  USER_MOTOR_WRITE(MOTOR_EN_PIN, axis, value);
}

inline void stepper_axis_write_step(int axis, bool value)
{
  USER_MOTOR_WRITE(MOTOR_STEP_PIN, axis, value);
}

inline void stepper_axis_write_dir(int axis, bool isInvert)
{
  GPIO_PinState value = (GPIO_PinState)(isInvert ? ccm_param::motion_3d_model.enable_invert_dir[axis] : !ccm_param::motion_3d_model.enable_invert_dir[axis]);
  USER_MOTOR_WRITE(MOTOR_DIR_PIN, axis, value);
}

inline bool stepper_axis_xyz_read_max(int axis)
{
  if (axis < XYZ_NUM_AXIS)
  {
    USER_ENDSTOP_READ(MOTOR_LIMIT_MAX_PIN, axis, endstop_max_inverting[axis]);
  }

  return false;
}

inline bool stepper_axis_xyz_read_min(int axis)
{
  if (axis < XYZ_NUM_AXIS)
  {
    if (Z_AXIS == axis)
    {
      if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
      {
        if (gcode::g29_open_laser_check_flag)
        {
          USER_ENDSTOP_READ(MOTOR_LIMIT_MIN_PIN, axis, endstop_min_inverting[axis]);
        }
        else
        {
          USER_ENDSTOP_READ(MOTOR_LIMIT_MAX_PIN, axis, endstop_min_inverting[axis]);
        }
      }
      else
      {
        USER_ENDSTOP_READ(MOTOR_LIMIT_MAX_PIN, axis, endstop_min_inverting[axis]);
      }
    }
    else
    {
      USER_ENDSTOP_READ(MOTOR_LIMIT_MIN_PIN, axis, endstop_min_inverting[axis]);
    }
  }

  return false;
}

inline void stepper_pin_init(void)
{
  //endstops and pullups
  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    USER_MOTOR_WRITE(MOTOR_LIMIT_MIN_PIN, i, GPIO_PIN_SET);
  }

  //Initialize Step Pins
  for (int i = 0; i < MAX_NUM_AXIS; i++)
  {
    stepper_axis_enable(i, true);
    stepper_axis_write_step(i, invert_axis_step_pin[i]);
    stepper_axis_enable(i, false);
  }
}

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // STEPPER_PIN_H

