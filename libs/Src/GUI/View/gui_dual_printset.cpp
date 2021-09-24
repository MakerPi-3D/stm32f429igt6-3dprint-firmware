#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_ccm.h"

#define NoKey 0
#define NozzleTempDown 1
#define NozzleTempUp 2
#define NozzleTemp1Down 3
#define NozzleTemp1Up 4
#define PrintSpeedDown 5
#define PrintSpeedUp 6

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_6
#define TextRangeBuf_NozzleTemp1 ccm_param::TextRangeBuf_24_12_3_7
#define SetPrintSpeedValueSharpt ccm_param::TextRangeBuf_24_12_3_0

#define Timeoutms 200
#define FirstTimeoutms 600

#ifdef __cplusplus
extern "C" {
#endif

extern int temperature_get_heater_maxtemp(int axis);

static struct _textrange  NozzleTempTextSharp;
static struct _textrange  NozzleTemp1TextSharp;
static struct _textrange  PrintSpeedTextSharp;

static uint16_t TextColor = 0xf800; //数值显示的颜色--红色

static uint8_t KeyValue = NoKey;

static unsigned long Timeout = 0;
static unsigned long FirstTimeout = 0;

static int SetNozzleTempValue = 0;
static int SetNozzleTemp1Value = 0;
static uint32_t SetPrintSpeedValue = 100;

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t IsLoadFilamentInterface_dual = 0; //是否是进料的调节温度界面
static uint8_t IsUnloadFilamentInterface_dual = 0; //是否是退料的调节温度界面

static uint8_t IsPowerOffRecoverReady_dual = 0; //是否是断电续打恢复准备界面

static uint8_t ScanKey_NotM14_Left_dual(void)
{
  if (IstouchxyDown(284, 14, 336, 68))
  {
    return NozzleTempUp ;
  }
  else if (IstouchxyDown(358, 14, 412, 68))
  {
    return NozzleTempDown;
  }
  else if (IstouchxyDown(284, 82, 336, 132))
  {
    return NozzleTemp1Up ;
  }
  else if (IstouchxyDown(358, 82, 412, 132))
  {
    return NozzleTemp1Down;
  }
  else if (IstouchxyDown(284, 144, 336, 196))
  {
    return PrintSpeedUp ;
  }
  else if (IstouchxyDown(358, 144, 412, 196))
  {
    return PrintSpeedDown;
  }
  else
  {
    return NoKey;
  }
}

static void LongPress_dual(void)
{
  if (IstouchxyUp())
  {
    KeyValue = 0;
    (void)OS_DELAY(200);
  }
  else if (Timeout < xTaskGetTickCount())
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    SlowUpdateData = 0;
    FastUpdateData = 1;
  }
}

static void ShortPress_dual(void)
{
  SlowUpdateData = 1;
  FastUpdateData = 0;
}

static void UpdateData_dual(void)
{
  switch (KeyValue)
  {
  case NozzleTempDown:
    if (1 == FastUpdateData)
    {
      SetNozzleTempValue = SetNozzleTempValue - 10;
    }
    else
    {
      SetNozzleTempValue = SetNozzleTempValue - 1;
      KeyValue = NoKey;
    }

    break;

  case NozzleTempUp:
    if (1 == FastUpdateData)
    {
      SetNozzleTempValue = SetNozzleTempValue + 10;
    }
    else
    {
      SetNozzleTempValue = SetNozzleTempValue + 1;
      KeyValue = NoKey;
    }

    break;

  case NozzleTemp1Down:
    if (1 == FastUpdateData)
    {
      SetNozzleTemp1Value = SetNozzleTemp1Value - 10;
    }
    else
    {
      SetNozzleTemp1Value = SetNozzleTemp1Value - 1;
      KeyValue = NoKey;
    }

    break;

  case NozzleTemp1Up:
    if (1 == FastUpdateData)
    {
      SetNozzleTemp1Value = SetNozzleTemp1Value + 10;
    }
    else
    {
      SetNozzleTemp1Value = SetNozzleTemp1Value + 1;
      KeyValue = NoKey;
    }

    break;

  case PrintSpeedDown:
    if (1 == FastUpdateData)
    {
      SetPrintSpeedValue = SetPrintSpeedValue - 40; //0.4f;
    }
    else
    {
      SetPrintSpeedValue = SetPrintSpeedValue - 10; //0.1f;
      KeyValue = NoKey;
    }

    break;

  case PrintSpeedUp:
    if (1 == FastUpdateData)
    {
      SetPrintSpeedValue = SetPrintSpeedValue + 40; //0.4f;
    }
    else
    {
      SetPrintSpeedValue = SetPrintSpeedValue + 10; //0.1f;
      KeyValue = NoKey;
    }

    break;

  default:
    break;
  }
}

static void DataRangeLimit_dual(void)
{
  if (SetNozzleTempValue > temperature_get_heater_maxtemp(0) - 25)
    SetNozzleTempValue = temperature_get_heater_maxtemp(0) - 25;

  if (SetNozzleTempValue < 0)
    SetNozzleTempValue = 0;

  if (SetNozzleTemp1Value > temperature_get_heater_maxtemp(1) - 25)
    SetNozzleTemp1Value = temperature_get_heater_maxtemp(1) - 25;

  if (SetNozzleTemp1Value < 0)
    SetNozzleTemp1Value = 0;

  if (SetPrintSpeedValue > 200) //2.0f)
    SetPrintSpeedValue = 200; //2.0f;

  if (SetPrintSpeedValue < 10) //0.1f)
    SetPrintSpeedValue = 10; //0.1f;
}

static void DisplayText_NotM14_Left_dual(void)
{
  char Textbuffer[20];
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTempValue);
  CopyTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleTempTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTemp1Value);
  CopyTextDisplayRangeInfo(NozzleTemp1TextSharp, TextRangeBuf_NozzleTemp1, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleTemp1TextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", SetPrintSpeedValue / 100.0f);
  CopyTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, PrintSpeedTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
}

static void SetValue_NotM14_Left_dual(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = ScanKey_NotM14_Left_dual();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      LongPress_dual();
    }
    else if (IstouchxyUp())
    {
      ShortPress_dual();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData_dual();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit_dual();
    DisplayText_NotM14_Left_dual();
  }
}

static void ConfirmKey_NotM14_Left_dual(void)
{
  SettingInfoToSYS.TargetNozzleTemp = SetNozzleTempValue;
  SettingInfoToSYS.TargetNozzleTemp1 = SetNozzleTemp1Value;
  SettingInfoToSYS.PrintSpeed = SetPrintSpeedValue;
  t_gui.print_speed_value = (uint16_t)(SetPrintSpeedValue);
  respond_gui_send_sem(PrintSetValue_NotM14_Left);

  if (IsLoadFilamentInterface_dual)
  {
    gui::set_current_display(loadfilament0F_dual);
  }
  else if (IsUnloadFilamentInterface_dual)
  {
    gui::set_current_display(unloadfilament0F_dual);
  }
  else if (IsPowerOffRecoverReady_dual)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}

static void CancelKey_dual(void)
{
  if (IsLoadFilamentInterface_dual)
  {
    gui::set_current_display(loadfilament0F_dual);
  }
  else if (IsUnloadFilamentInterface_dual)
  {
    gui::set_current_display(unloadfilament0F_dual);
  }
  else if (IsPowerOffRecoverReady_dual)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}
extern bool IsPrintSDFile(void);

static void RefreshTheInterface_NotM14_Left_dual(void)
{
  display_picture(155);
  SetTextDisplayRangeNormal(236, 32, 12 * 3, 24, &NozzleTempTextSharp);
  SetTextDisplayRangeNormal(236, 96, 12 * 3, 24, &NozzleTemp1TextSharp);
  SetTextDisplayRangeNormal(236, 158, 12 * 3, 24, &PrintSpeedTextSharp);
  ReadTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp);
  ReadTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp1);
  ReadTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt);
  SetNozzleTempValue = t_gui.target_nozzle_temp[0];
  SetNozzleTemp1Value = t_gui.target_nozzle_temp[1];
  SetPrintSpeedValue = t_gui.print_speed_value; ///100.0f;   //转换成小数  0-200   0-2.0
  DisplayText_NotM14_Left_dual();
}

void PrintSet_NotM14_Left_dual(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_NotM14_Left_dual();
    return;
  }

  if (touchxy(150, 210 + 20, 228, 252 + 20)) //确认键
  {
    ConfirmKey_NotM14_Left_dual();
    return ;
  }

  if (touchxy(264, 210 + 20, 342, 252 + 20)) //取消键
  {
    CancelKey_dual();
    return ;
  }

  SetValue_NotM14_Left_dual();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

void SetLoadFilamentNozzleTemp_dual(void)
{
  IsLoadFilamentInterface_dual = 1;
  PrintSet_NotM14_Left_dual();
  IsLoadFilamentInterface_dual = 0;
}

void SetUnLoadFilamentNozzleTemp_dual(void)
{
  IsUnloadFilamentInterface_dual = 1;
  PrintSet_NotM14_Left_dual();
  IsUnloadFilamentInterface_dual = 0;
}

void SetPowerOffRecoverNozzleTemp_dual(void)
{
  IsPowerOffRecoverReady_dual = 1;
  PrintSet_NotM14_Left_dual();
  IsPowerOffRecoverReady_dual = 0;
}


#ifdef __cplusplus
} //extern "C" {
#endif

