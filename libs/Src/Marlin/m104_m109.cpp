#include "gcodebufferhandle.h"
#include "sysconfig_data.h"
#include "temperature.h"
#include "user_common.h"
#include "globalvariables.h"
#include "stepper.h"
#include "flashconfig.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

static bool m109_heating_complete = false;             /*!< M109是否加热完成 */

extern uint8_t tmp_extruder;
extern unsigned long previous_xTaskGetTickCount_cmd;
extern void manage_synchronize(void);

bool setTargetedHotend(int16_t code);

bool isM109HeatingComplete(void)
{
  return m109_heating_complete;
}

void resetM109HeatingComplete(void)
{
  m109_heating_complete = false;
}


#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  static bool extruder_is_cool_down_not_wait = true;              /*!< 冷却不等待 */
  static bool extruder_target_direction;                          /*!<  */
  static bool extruder_cancel_heatup = false;                     /*!< 是否取消加热 */
  static unsigned long extruder_respond_temp_status_peroid = 0;
  static unsigned long extruder_respond_temp_status_time_count = 0;


  void m104_process(void)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      st_synchronize();//等待上一条消息执行完
    }

    if (setTargetedHotend(104))
      return;

    if (parseGcodeBufHandle.codeSeen('S'))
    {
      if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY || t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
      {
        if (tmp_extruder == 1) return;

        temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), 1);
      }

      temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
    }
  }

  void m109_process(void)
  {
    if (setTargetedHotend(109))
      return;

    m109_heating_complete = false;

    if (parseGcodeBufHandle.codeSeen('S'))
    {
      if (t_sys.idex_print_type == IDEX_PRINT_TYPE_COPY || t_sys.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
      {
        if (tmp_extruder == 1) return;

        temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), 1);
      }

      temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
      extruder_is_cool_down_not_wait = true;
    }
    else if (parseGcodeBufHandle.codeSeen('R'))
    {
      temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
      extruder_is_cool_down_not_wait = false;
    }

    //  unsigned long codenum = xTaskGetTickCount();
    /* See if we are heating up or cooling down */
    extruder_target_direction = temperature_is_extruder_heating(tmp_extruder); // true if heating, false if cooling
    extruder_cancel_heatup = false;

    while (extruder_target_direction ? (temperature_is_extruder_heating(tmp_extruder)) :
           (temperature_is_extruder_cooling(tmp_extruder) && (!extruder_is_cool_down_not_wait)))
    {
      // 秒数累加
      if (extruder_respond_temp_status_peroid < MILLIS())
      {
        extruder_respond_temp_status_peroid = MILLIS() + 1000;
        extruder_respond_temp_status_time_count++;
      }

      if (2 < extruder_respond_temp_status_time_count)
      {
        if (is_serial_full)
          m105_process();

        extruder_respond_temp_status_time_count = 0;
      }

      manage_synchronize();
      (void)OS_DELAY(100);
    }

    m109_heating_complete = true;
    //        starttime=xTaskGetTickCount();
    previous_xTaskGetTickCount_cmd = xTaskGetTickCount();
  }


}

