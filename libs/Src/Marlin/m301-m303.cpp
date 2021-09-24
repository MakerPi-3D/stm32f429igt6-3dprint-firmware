#include "gcodebufferhandle.h"
#include "globalvariables.h"
#include "Configuration.h"
#include "temperature_pid_temp.h"
#include "temperature.h"
#include "user_ccm.h"
#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{


  void m303_process(void)
  {
    float temp = 150.0;
    int16_t e = 0;
    int16_t c = 5;

    if (parseGcodeBufHandle.codeSeen('E')) e = (short)parseGcodeBufHandle.codeValue();

    if (e < EXTRUDERS && e >= 0)
    {
      if (parseGcodeBufHandle.codeSeen('S')) temp = parseGcodeBufHandle.codeValue();

      if (parseGcodeBufHandle.codeSeen('C')) c = (short)parseGcodeBufHandle.codeValue();

      grbl_temp::pid_autotune(temp, e, c);
    }
  }

  void m302_process(void)
  {
    #ifdef PREVENT_DANGEROUS_EXTRUDE
    float temp = 0.0;

    if (parseGcodeBufHandle.codeSeen('S'))
    {
      temp = parseGcodeBufHandle.codeValue();
      ccm_param::motion_3d.extrude_min_temp = temp;
    }

    //  else //输出当前最小温度
    #endif
  }


  void m301_process(void)
  {
    #ifdef PIDTEMP

    if (parseGcodeBufHandle.codeSeen('P')) temp_pid_extruder_set_kp(parseGcodeBufHandle.codeValue());

    if (parseGcodeBufHandle.codeSeen('I')) temp_pid_extruder_set_ki(parseGcodeBufHandle.codeValue());

    if (parseGcodeBufHandle.codeSeen('D')) temp_pid_extruder_set_kd(parseGcodeBufHandle.codeValue());

    #ifdef PID_ADD_EXTRUSION_RATE

    if (parseGcodeBufHandle.codeSeen('C')) temp_pid_extruder_set_kc(parseGcodeBufHandle.codeValue());

    #endif
    temp_pid_extruder_update();
    #endif // #ifdef PIDTEMP
  }

}

