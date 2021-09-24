#include "gcodebufferhandle.h"
#include "user_ccm.h"
#include "planner.h"
#include "gcode.h"
#include "ConfigurationStore.h"
#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t tmp_extruder;

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  void m200_process(void)
  {
    //float area = 0.0f;
    float radius = 0.0f;

    if (parseGcodeBufHandle.codeSeen('D'))
    {
      radius = (float)parseGcodeBufHandle.codeValue() * 0.5f;

      if (radius == 0)
      {
        //area = 1;
      }
      else
      {
        //area = PI * pow(radius, 2);
      }
    }
    else
    {
      //reserved for setting filament diameter via UFID or filament measuring device
      return;
    }

    tmp_extruder = gcode::active_extruder;

    if (parseGcodeBufHandle.codeSeen('T'))
    {
      tmp_extruder = (unsigned char)parseGcodeBufHandle.codeValue();

      if (tmp_extruder >= EXTRUDERS)
      {
        //            SERIAL_ECHO_START;
        //            SERIAL_ECHO(MSG_M200_INVALID_EXTRUDER);
        return;
      }
    }

    //volumetric_multiplier[tmp_extruder] = 1 / area;
  }

  void m201_process(void)
  {
    for (int8_t i = 0; i < ccm_param::motion_3d.axis_num; i++)
    {
      if (parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        planner_settings.max_acceleration_mm_per_s2[i] = (unsigned long)parseGcodeBufHandle.codeValue();
      }
    }

    // steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
    reset_acceleration_rates();
  }

  void m203_process(void)
  {
    for (int8_t i = 0; i < ccm_param::motion_3d.axis_num; i++)
    {
      if (parseGcodeBufHandle.codeSeen(axis_codes[i])) planner_settings.max_feedrate_mm_s[i] = parseGcodeBufHandle.codeValue();
    }
  }

  void m204_process(void)
  {
    if (parseGcodeBufHandle.codeSeen('S')) planner_settings.acceleration = parseGcodeBufHandle.codeValue() ;

    if (parseGcodeBufHandle.codeSeen('T')) planner_settings.retract_acceleration = parseGcodeBufHandle.codeValue() ;
  }

  void m205_process(void)
  {
    if (parseGcodeBufHandle.codeSeen('S')) planner_settings.min_feedrate_mm_s = parseGcodeBufHandle.codeValue();

    if (parseGcodeBufHandle.codeSeen('T')) planner_settings.min_travel_feedrate_mm_s = parseGcodeBufHandle.codeValue();

    if (parseGcodeBufHandle.codeSeen('B')) planner_settings.min_segment_time_us = (unsigned long)parseGcodeBufHandle.codeValue() ;

    if (parseGcodeBufHandle.codeSeen('X')) planner_settings.max_xy_jerk = parseGcodeBufHandle.codeValue() ;

    if (parseGcodeBufHandle.codeSeen('Z')) planner_settings.max_z_jerk = parseGcodeBufHandle.codeValue() ;

    if (parseGcodeBufHandle.codeSeen('E'))
    {
      planner_settings.max_e_jerk = parseGcodeBufHandle.codeValue() ;
      planner_settings.max_b_jerk = planner_settings.max_e_jerk;
    }
  }



}

