#include "gcodebufferhandle.h"
#include "temperature.h"
#include "globalvariables.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

static bool m190_heating_complete = false;             /*!< M190是否热床加热完成 */

extern unsigned long previous_xTaskGetTickCount_cmd;
extern void manage_synchronize(void);

bool isM190HeatingComplete(void)
{
  return m190_heating_complete;
}

void resetM190HeatingComplete(void)
{
  m190_heating_complete = false;
}


#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{

  static bool bed_is_cool_down_not_wait = true;              /*!< 冷却不等待 */
  static bool bed_target_direction;                          /*!<  */
  static bool bed_cancel_heatup = false;                     /*!< 是否取消加热 */
  static unsigned long bed_respond_temp_status_peroid = 0;
  static unsigned long bed_respond_temp_status_time_count = 0;

  void m140_process(void)
  {
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      temperature_set_bed_target(parseGcodeBufHandle.codeValue());
    }
  }

  void m190_process(void)
  {
    m190_heating_complete = false;
    {
      if (parseGcodeBufHandle.codeSeen('S'))
      {
        temperature_set_bed_target(parseGcodeBufHandle.codeValue());
        bed_is_cool_down_not_wait = true;
      }
      else if (parseGcodeBufHandle.codeSeen('R'))
      {
        temperature_set_bed_target(parseGcodeBufHandle.codeValue());
        bed_is_cool_down_not_wait = false;
      }

      bed_cancel_heatup = false;
      bed_target_direction = temperature_is_bed_heating(); // true if heating, false if cooling

      while ((bed_target_direction) && (!bed_cancel_heatup) ? (temperature_is_bed_heating()) : (temperature_is_bed_cooling() && (!bed_is_cool_down_not_wait)))
      {
        // 秒数累加
        if (bed_respond_temp_status_peroid < MILLIS())
        {
          bed_respond_temp_status_peroid = MILLIS() + 1000;
          bed_respond_temp_status_time_count++;
        }

        if (2 < bed_respond_temp_status_time_count)
        {
          if (is_serial_full)
            m105_process();

          bed_respond_temp_status_time_count = 0;
        }

        manage_synchronize();
        (void)OS_DELAY(100);
      }

      previous_xTaskGetTickCount_cmd = xTaskGetTickCount();
    }
    m190_heating_complete = true;
  }


}

