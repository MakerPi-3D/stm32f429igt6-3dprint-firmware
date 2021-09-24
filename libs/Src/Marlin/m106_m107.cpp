#include "gcodebufferhandle.h"
#include "flashconfig.h"
#include "stepper.h"
#include "sysconfig_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{
  void m106_process(void)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      st_synchronize();//等待上一条消息执行完
    }

    int fanSpeed = 0;

    if (parseGcodeBufHandle.codeSeen('S'))
    {
      int fanspeed_codevalue = (int)parseGcodeBufHandle.codeValue();
      fanSpeed = constrain(fanspeed_codevalue, 0, 255);
    }
    else
    {
      fanSpeed = 255;
    }

    feature_set_extruder_fan_speed(fanSpeed);
  }

  void m107_process(void)
  {
    feature_set_extruder_fan_speed(0);;
  }
}










