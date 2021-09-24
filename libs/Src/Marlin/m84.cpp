#include "gcodebufferhandle.h"
#include "threed_engine.h"
#include "stepper.h"
#include "stepper_pin.h"
#include "flashconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long stepper_inactive_time;

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  /**
  * [m84_process 解锁电机]
  * M84 X Y Z E B S1\r\n
  * 参数X、Y、Z、E、B 代表对应的电机
  * 参数S1 代表1S后解锁电机，不带S参数代表立即解锁电机
  */
  void m84_process(void)
  {
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      stepper_inactive_time = (unsigned long)parseGcodeBufHandle.codeValue() * 1000; // 电机解锁延时
    }
    else
    {
      bool codeSeenAllAxis = !parseGcodeBufHandle.codeSeen(axis_codes[0]); // 是否查询到所有电机参数

      for (int i = 1; i < MAX_NUM_AXIS; i++)
      {
        codeSeenAllAxis = (codeSeenAllAxis && !parseGcodeBufHandle.codeSeen(axis_codes[i]));
      }

      if (codeSeenAllAxis)
      {
        st_synchronize(); // 等待运动队列清空

        for (int i = 0; i < MAX_NUM_AXIS; i++)
        {
          stepper_axis_enable(i, false); // 解锁电机
        }
      }
      else
      {
        st_synchronize(); // 等待运动队列清空

        for (int i = 0; i < MAX_NUM_AXIS; i++)
        {
          if (parseGcodeBufHandle.codeSeen(axis_codes[i]))
          {
            if (X_AXIS == i)
            {
              stepper_axis_enable(i, false); // 解锁电机

              if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
                  flash_param_t.extruder_type == EXTRUDER_TYPE_LASER ||
                  flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
                stepper_axis_enable(X2_AXIS, false); // 解锁电机
            }
            else
            {
              stepper_axis_enable(i, false); // 解锁电机
            }
          }
        }
      }
    }
  }


}

