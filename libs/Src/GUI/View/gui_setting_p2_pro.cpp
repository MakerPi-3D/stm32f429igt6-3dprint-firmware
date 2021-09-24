#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void dual_extruder_setting(void);
extern void dual_home_pos_setting(void);

void settingF_p2_pro(void)
{
  if (gui::is_refresh())
  {
    display_picture(163);//按键、警报、触摸校准、断电、校准平台

    if (t_sys.key_sound)
    {
      LCD_Fill(115, 96, 115 + 20, 96 + 12, (u16)testcolor);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill(209, 96, 209 + 20, 96 + 12, (u16)testcolor);
    }
  }

  if (touchxy(22, 113, 118, 186)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill(115, 96, 115 + 20, 96 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(135, 113, 230, 186)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill(209, 96, 209 + 20, 96 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(248, 113, 345, 186)) //触摸校正键
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }

  if (touchxy(365, 113, 461, 186)) //Z行程测量键
  {
    gui::set_current_display(dual_extruder_setting);
    return ;
  }

  if (touchxy(22, 219, 118, 294)) //计算平台
  {
    gcode::extruder_0_1_toggle();
    return ;
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif


