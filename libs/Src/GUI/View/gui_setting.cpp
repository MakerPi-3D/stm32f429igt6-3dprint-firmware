#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include "interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "config_model_tables.h"
#include "user_ccm.h"
#ifdef __cplusplus
extern "C" {
#endif

//#ifdef ENABLE_AUTO_BED_LEVELING
//extern void gui_bed_level_nozzle_heat(void);
extern bool is_enable_bed_level(void);
//#endif // #ifdef ENABLE_AUTO_BED_LEVELING

extern void SettingInterface4_dual(void);

void SettingInterface5(void)
{
  if (gui::is_refresh())
  {
    //    USER_EchoLogStr("I5\r\n");//串口上传信息到上位机2017.7.6
    display_picture(13);//按键、警报、触摸校准

    if (t_sys.key_sound)
    {
      LCD_Fill(144, 128, 144 + 20, 128 + 12, (u16)testcolor);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill(263, 128, 263 + 20, 128 + 12, (u16)testcolor);
    }
  }

  if (touchxy(48, 145, 154, 228))
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
      LCD_Fill(144, 128, 144 + 20, 128 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(188, 145, 295, 228))
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
      LCD_Fill(263, 128, 263 + 20, 128 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(327, 145, 433, 228))
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }
}
/*2、3界面有断料校准，以前用红外模块时用到，现在的模块不用校准20171024*/
/*
void SettingInterface2(void)
{
  if(gui::is_refresh())
  {
    USER_EchoLogStr("I2\r\n");//串口上传信息到上位机2017.7.6
    display_picture(46);//按键、警报、屏幕校准、z轴校准

    if(t_sys.key_sound)
    {
      LCD_Fill(91+3,113+3,91+3+20,113+3+12,(u16)testcolor);
    }
    if(t_sys.alarm_sound)
    {
      LCD_Fill(203+3,113+3,203+3+20,113+3+12,(u16)testcolor);
    }
  }
  if(touchxy(22,113,118,186)) //按键声音设置键
  {
    if(t_sys.key_sound)
    {
      t_sys.key_sound=0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound=1;
      LCD_Fill(91+3,113+3,91+3+20,113+3+12,(u16)testcolor);
      (void)OS_DELAY(500);
    }
  }
  if(touchxy(135,113,230,186))  //报警声音设置键
  {
    if(t_sys.alarm_sound)
    {
      t_sys.alarm_sound=0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound=1;
      LCD_Fill(203+3,113+3,203+3+20,113+3+12,(u16)testcolor);
      (void)OS_DELAY(500);
    }
  }
  if(touchxy(248,113,345,186)) //触摸校正键
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }
  if(touchxy(365,113,461,186)) //Z行程测量键
  {
    gui::set_current_display(CalculatingZMaxLimit);

    SettingInfoToSYS.GUISempValue=CalculateZMaxPos;
    GUISendSempToSYS();
    return ;
  }

//  if(touchxy(22,219,118,294)) //断料校准键
//  {
//    gui::set_current_display(MatCheckCalibrateReady);
//    return ;
//  }
}

void SettingInterface3(void)
{
  if(gui::is_refresh())
  {
    USER_EchoLogStr("P2_Pro\r\n");//串口上传信息到上位机2017.7.6
    display_picture(13);//按键、警报、触摸校准

    if(t_sys.key_sound)
    {
      LCD_Fill(91+3,152+3,91+3+20,152+3+12,(u16)testcolor);
    }
    if(t_sys.alarm_sound)
    {
      LCD_Fill(203+3,152+3,203+3+20,152+3+12,(u16)testcolor);
    }
  }
  if(touchxy(22,152,118,226)) //按键声音设置键
  {
    if(t_sys.key_sound)
    {
      t_sys.key_sound=0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound=1;
      LCD_Fill(91+3,152+3,91+3+20,152+3+12,(u16)testcolor);
      (void)OS_DELAY(500);
    }
  }
  if(touchxy(135,152,230,226))  //报警声音设置键
  {
    if(t_sys.alarm_sound)
    {
      t_sys.alarm_sound=0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound=1;
      LCD_Fill(203+3,152+3,203+3+20,152+3+12,(u16)testcolor);
      (void)OS_DELAY(500);
    }
  }
  if(touchxy(248,152,345,226)) //触摸校正键
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }
//  if(touchxy(365,152,461,226)) //断料校准键
//  {
//    gui::set_current_display(MatCheckCalibrateReady);
//    return ;
//  }
}
*/
void SettingInterface4(void)
{
  if (gui::is_refresh())
  {
    //    USER_EchoLogStr("I4\r\n");//串口上传信息到上位机2017.7.6
    display_picture(46);//按键、警报、屏幕校准、z轴校准

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
    gui::set_current_display(CalculatingZMaxLimit);
    respond_gui_send_sem(CalculateZMaxPos);
    return ;
  }
}
//机器配置
void SettingInterface1(void)
{
  if (gui::is_refresh())
  {
    USER_EchoLogStr("I1\r\n");//串口上传信息到上位机2017.7.6
    display_picture(30);//按键、警报、屏幕校准、机器配置

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

  if (touchxy(365, 152, 461, 226)) //机器配置键
  {
    gui::set_current_display(MachineSetting);
    return ;
  }
}

void settingF(void)
{
  if (t_sys_data_current.model_id == P2_Pro)
  {
    settingF_p2_pro();
  }
  else if (t_sys_data_current.model_id == F400TP)
  {
    //   SettingInterface4_dual(); //显示没有机器配置选项，但有测量行程选项的界面
    gui_p3_pro_setting(); //和P3 Pro一样的界面
  }
  else if (t_sys_data_current.model_id == P3_Pro)
  {
    gui_p3_pro_setting();
  }
  else
  {
    if (!t_sys_data_current.have_set_machine) //还没设置机器
    {
      SettingInterface1(); //按键、警报、触摸校准、设置机型
    }
    //  else if((!motion_3d.disable_z_max_limit) && (t_sys_data_current.enable_material_check))  //已设置机器，且Z轴有下限位开关, 且有断料检测功能
    //  {
    //    SettingInterface2(); //显示没有机器配置选项，但有测量行程选项、有断料校准的界面
    //  }
    //  else if(t_sys_data_current.enable_material_check)  //已设置机器，且有断料检测功能
    //  {
    //    SettingInterface3(); //按键、警报、触摸校准
    //  }
    else if (!ccm_param::motion_3d.disable_z_max_limit) //已设置机器，且Z轴有下限位开关
    {
      SettingInterface4(); //显示没有机器配置选项，但有测量行程选项的界面
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
    }
    else //已设置机器，且Z轴没有下限位开关、没有断料检测功能
    {
      SettingInterface5(); //按键、警报、触摸校准
    }
  }

  if (touchxy(0, 0, 150, 70)) //返回键
  {
    SaveBezzerSound();
    gui::set_current_display(maindisplayF);
    return ;
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

