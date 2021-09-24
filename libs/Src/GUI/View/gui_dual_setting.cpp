#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool is_enable_bed_level(void);

extern void dual_extruder_setting(void);

void SettingInterface4_dual(void)
{
  if (gui::is_refresh())
  {
    display_picture(159);//按键、警报、屏幕校准、z轴校准

    if (t_sys.key_sound)
    {
      LCD_Fill(115, 131, 115 + 20, 131 + 12, (u16)testcolor);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill(209, 131, 209 + 20, 131 + 12, (u16)testcolor);
    }
  }

  if (touchxy(22, 152, 118, 226)) //按键声音设置键
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
      LCD_Fill(115, 131, 115 + 20, 131 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(135, 152, 230, 226)) //报警声音设置键
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
      LCD_Fill(209, 131, 209 + 20, 131 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(248, 152, 345, 226)) //触摸校正键
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }

  if (touchxy(365, 152, 461, 226)) //测量行程键
  {
    gui::set_current_display(dual_extruder_setting);
    return ;
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif


