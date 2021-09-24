#include  "common.h"
#include  "commonf.h"
#include "globalvariables.h"
#include "sysconfig_data.h"
#include "functioncustom.h"
#include  "interface.h"
#define mstime_def  10000

#ifdef __cplusplus
extern "C" {
#endif


void printfinishF(void)
{
  char buffer[20];
  int length;
  int hour, minute, second;
  static unsigned int mstime;

  if (gui::is_refresh())
  {
    char printnameb[_MAX_LFN];
    display_picture(27);
    //    poweroff_reset_flag(); // 娓呴櫎鏂數鏍囧織
    flash_erase_poweroff_data();
    strcpy(printnameb, printname);

    if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
    }

    length = (int)strlen(printnameb);
    DisplayText((uint8_t *)printnameb, 240 - (length / 2) * 12, 95, 24, (u16)testcolor); //240-(length/2)*12   是为了让文字显示在中间
    second = t_gui.printed_time_sec;
    hour = second / 3600;
    minute = (second - hour * 3600) / 60;
    second = (second - hour * 3600) % 60;
    snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d", hour, minute, second);
    DisplayText((uint8_t *)buffer, 195, 155, 24, (u16)testcolor);

    if (t_gui.used_total_material >= 100000)
    {
      snprintf(buffer, sizeof(buffer), "%9d m", t_gui.used_total_material / 1000);
    }
    else
    {
      snprintf(buffer, sizeof(buffer), "%9d mm", t_gui.used_total_material);
    }

    DisplayText((uint8_t *)buffer, 195, 200, 24, (u16)testcolor);
    mstime = xTaskGetTickCount();

    if (t_sys.alarm_sound)
    {
      respond_gui_send_sem(OpenBeep);
    }
  }

  if (touchxy(160, 230, 325, 320))
  {
    if (t_sys.alarm_sound)
    {
      respond_gui_send_sem(CloseBeep);
    }

    gui::set_current_display(maindisplayF);
    return;
  }

  if (gui::is_refresh_rtc())
  {
    if (xTaskGetTickCount() - mstime > mstime_def)
    {
      if (t_sys.alarm_sound)
      {
        respond_gui_send_sem(CloseBeep);
      }
    }
  }
}
#ifdef __cplusplus
} //extern "C" {
#endif


