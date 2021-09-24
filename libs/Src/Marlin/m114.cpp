#include "gcodebufferhandle.h"
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
  void m114_process(void)
  {
    float e = st_get_position_length(E_AXIS);
    float b = 0.0f;

    if (t_sys_data_current.enable_color_mixing)
    {
      b = st_get_position_length(B_AXIS);
    }

    USER_EchoLogStr("ok X:%.2f X2:%.2f Y:%.2f Z:%.2f Z2:%.2f ", st_get_position_length(X_AXIS), st_get_position_length(X2_AXIS), \
                    st_get_position_length(Y_AXIS), st_get_position_length(Z_AXIS), st_get_position_length(Z2_AXIS));
    osDelay(1);
    USER_EchoLogStr("E:%.2f B:%.2f ", e, b);
    USER_EchoLogStr("\r\n");
  }
}






