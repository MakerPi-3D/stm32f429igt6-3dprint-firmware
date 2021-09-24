#include "gcodebufferhandle.h"
#include "planner.h"
#include "ConfigurationStore.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  void m92_process(void)
  {
    for (int8_t i = 0; i < ccm_param::motion_3d.axis_num; i++)
    {
      if (parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        if (i == E_AXIS || i == B_AXIS)
        {
          // E
          float value = parseGcodeBufHandle.codeValue();

          if (value < 20.0f)
          {
            float factor = planner_settings.axis_steps_per_mm[i] / value; // increase e constants if M92 E14 is given for netfab.
            planner_settings.max_e_jerk *= factor;
            planner_settings.max_feedrate_mm_s[i] *= factor;
            axis_steps_per_sqr_second[i] *= (unsigned long)factor;
          }

          planner_settings.axis_steps_per_mm[i] = value;
        }
        else
        {
          planner_settings.axis_steps_per_mm[i] = parseGcodeBufHandle.codeValue();
        }
      }
    }
  }
}






