#include  "common.h"
#include  "commonf.h"
#include "globalvariables.h"
#include "user_common.h"
#include "sysconfig_data.h"
#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern char* call_sysconfig_get_model_info(void);
//extern char* call_sysconfig_get_function_info(void);
//extern char* call_sysconfig_get_version_info(void);

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_Time ccm_param::TextRangeBuf_24_12_9_2

static void Hide_SettingInterface(uint8_t isclear)
{
  static uint8_t TouchCount1 = 0;
  static uint8_t TouchCount2 = 0;
  int i;

  if (isclear)
  {
    if (t_sys.lcd_ssd1963_43_480_272)
    {
      //      static uint8_t test[4]= {12,2,13,3};
      for (i = 0; i < 2; i++)
      {
      }
    }

    TouchCount1 = 0;
    TouchCount2 = 0;
  }

  /*******************************************************************************/
  if (TouchXY_NoBeep(0, 250, 80, 320)) //点击3次
  {
    TouchCount1++;

    if (TouchCount1 >= 3)
      TouchCount1 = 3;
  }

  if (TouchXY_NoBeep(400, 250, 480, 320)) //点击3次
  {
    TouchCount2++;

    if (TouchCount2 >= 3)
      TouchCount2 = 3;
  }

  if ((TouchCount1 >= 3) && (TouchCount2 >= 3)) //右上角和右下角各点击5次，进入机器配置界面
  {
    TouchCount1 = 0;
    TouchCount2 = 0;
    gui::set_current_display(MachineSetting);
  }

  /*******************************************************************************/
}

extern struct _textrange  RunTimeTextRange;
void statusF(void)
{
  char buffer[20];
  int second;

  if (gui::is_refresh())
  {
    display_picture(14);
    DisplayText((uint8_t *)t_sys.model_str, 148, 103, 24, (u16)testcolor); //机型
    DisplayText((uint8_t *)t_sys.function_str, 148, 156, 24, (u16)testcolor); //功能配置
    //DisplayText ((uint8_t*)"Base",148,156,24,(u16)testcolor); //功能配置
    DisplayText((uint8_t *)t_sys.version_str, 148, 213, 24, (u16)testcolor); //版本
    //串口上传信息到上位机2017.7.6
    USER_EchoLogStr("M:%s\r\n", (uint8_t *)t_sys.model_str);
    USER_EchoLogStr("F:%s\r\n", (uint8_t *)t_sys.function_str);
    USER_EchoLogStr("V:%s\r\n", (uint8_t *)t_sys.version_str);
    SetTextDisplayRange(148, 268, 12 * 9, 24, &RunTimeTextRange);
    ReadTextDisplayRangeInfo(RunTimeTextRange, TextRangeBuf_Time);
    snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d", t_gui.machine_run_time / 3600, t_gui.machine_run_time % 3600 / 60, t_gui.machine_run_time % 3600 % 60);
    CopyTextDisplayRangeInfo(RunTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
    DisplayTextInRange((uint8_t *)buffer, RunTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    Hide_SettingInterface(1);
  }

  if (touchxy(0, 0, 160, 70))
  {
    gui::set_current_display(maindisplayF);
    return ;
  }

  /*******************************************************************************/
  Hide_SettingInterface(0);//隐藏的设置界面

  /*******************************************************************************/
  if (second < t_gui.machine_run_time)
  {
    second = t_gui.machine_run_time + 60;
    //显示运行时间
    snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d", t_gui.machine_run_time / 3600, t_gui.machine_run_time % 3600 / 60, t_gui.machine_run_time % 3600 % 60);
    CopyTextDisplayRangeInfo(RunTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
    DisplayTextInRange((uint8_t *)buffer, RunTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    //串口上传信息到上位机2017.7.6
    //开发商要求不上传2017.9.6
    //    USER_EchoLogStr("T:%s\r\n",(uint8_t*)buffer);
  }

  //  if(gui::is_refresh_rtc())
  //  {
  //    second=t_gui.machine_run_time;
  //    hour=second/3600;
  //    minute = (second-hour*3600)/60;
  //    second = (second-hour*3600)%60;
  //    //显示运行时间
  //    snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d",hour,minute,second);
  //    CopyTextDisplayRangeInfo(RunTimeTextRange,TextRangeBuf_Time, TextRangeBuf_Str);
  //    DisplayTextInRange((uint8_t*)buffer, RunTimeTextRange,TextRangeBuf_Str,24,(u16)testcolor);
  //  }
}

#ifdef __cplusplus
} //extern "C" {
#endif
