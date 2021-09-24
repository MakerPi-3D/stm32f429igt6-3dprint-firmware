#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
//#include "ConfigurationStore.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_ccm.h"

#define NoKey 0
#define NozzleTempDown 1
#define NozzleTempUp 2
#define BedTempDown 3
#define BedTempUp 4
#define PrintSpeedDown 5
#define PrintSpeedUp 6
#define FanSpeedDown 7
#define FanSpeedUp 8
#define CavityTempDown 9
#define CavityTempUp 10
#define CavityTempOnDown 11
#define CavityTempOnUp 12
#define Timeoutms 200
#define FirstTimeoutms 600


#ifdef __cplusplus
extern "C" {
#endif

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_0
#define TextRangeBuf_CavityTargetTemp ccm_param::TextRangeBuf_24_12_3_0
#define SetPrintSpeedValueSharpt ccm_param::TextRangeBuf_24_12_3_1
#define SetFanSpeedValueSharpt ccm_param::TextRangeBuf_24_12_3_2
#define SetCavityTempOnValueSharpt ccm_param::TextRangeBuf_24_12_3_3
#define TextRangeBuf_HotBedTemp ccm_param::TextRangeBuf_24_12_3_4
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_5

extern int temperature_get_heater_maxtemp(int axis);

static struct _textrange  NozzleTempTextSharp;
static struct _textrange  BedTempTextSharp;
static struct _textrange  PrintSpeedTextSharp;
static struct _textrange  FanSpeedTextSharp;
static struct _textrange  CavityTempTextSharp;
static struct _textrange  CavityTempOnTextSharp;

static uint16_t TextColor = 0xf800; //数值显示的颜色--红色

static uint8_t KeyValue = NoKey;

static unsigned long Timeout = 0;
static unsigned long FirstTimeout = 0;

static int SetNozzleTempValue = 0;
static int SetBedTempValue = 0;
static uint32_t SetPrintSpeedValue = 100;
static int SetFanSpeedValue = 0;
static int SetCavityTempValue = 100;
static int SetCavityTempOnValue = 0;

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t IsLoadFilamentInterface = 0; //是否是进料的调节温度界面
static uint8_t IsUnloadFilamentInterface = 0; //是否是退料的调节温度界面

static uint8_t IsPowerOffRecoverReady = 0; //是否是断电续打恢复准备界面

uint8_t ScanKey_M14(void)
{
  if (IstouchxyDown(364, 0, 480, 82))
  {
    return NozzleTempDown;
  }
  else if (IstouchxyDown(250, 0, 364, 82))
  {
    return NozzleTempUp;
  }
  else if (IstouchxyDown(364, 82, 480, 179))
  {
    return FanSpeedDown;
  }
  else if (IstouchxyDown(250, 82, 364, 179))
  {
    return FanSpeedUp;
  }
  else if (IstouchxyDown(364, 179, 480, 260))
  {
    return PrintSpeedDown;
  }
  else if (IstouchxyDown(250, 179, 364, 260))
  {
    return PrintSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

uint8_t ScanKey_NotM14_Left(void)
{
  if (IstouchxyDown((t_sys.lcd_ssd1963_43_480_272 ? 344 : 364), 40, 480, 129))
  {
    return NozzleTempDown;
  }
  else if (IstouchxyDown(255, 40, (t_sys.lcd_ssd1963_43_480_272 ? 344 : 364), 129))
  {
    return NozzleTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return PrintSpeedDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return PrintSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

uint8_t ScanKey_NotM14_Right(void)
{
  if (IstouchxyDown((t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 40, 480, 129))
  {
    return BedTempDown;
  }
  else if (IstouchxyDown(255, 40, (t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 129))
  {
    return BedTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return FanSpeedDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return FanSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

uint8_t ScanKey_Cavity(void)
{
  if (IstouchxyDown(364, 40, 480, 129))
  {
    return CavityTempDown;
  }
  else if (IstouchxyDown(255, 40, 364, 129))
  {
    return CavityTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return CavityTempOnDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return CavityTempOnUp;
  }
  else
  {
    return NoKey;
  }
}


void LongPress(void)
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

void ShortPress(void)
{
  SlowUpdateData = 1;
  FastUpdateData = 0;
}

void UpdateData(void)
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

  case BedTempDown:
    if (1 == FastUpdateData)
    {
      SetBedTempValue = SetBedTempValue - 10;
    }
    else
    {
      SetBedTempValue = SetBedTempValue - 1;
      KeyValue = NoKey;
    }

    break;

  case BedTempUp:
    if (1 == FastUpdateData)
    {
      SetBedTempValue = SetBedTempValue + 10;
    }
    else
    {
      SetBedTempValue = SetBedTempValue + 1;
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

  case FanSpeedDown:
    //      if(1==FastUpdateData)
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue-10;
    //      }
    //      else
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue-1;
    //        KeyValue=NoKey;
    //      }
    SetFanSpeedValue = 0;
    break;

  case FanSpeedUp:
    //      if(1==FastUpdateData)
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue+10;
    //      }
    //      else
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue+1;
    //        KeyValue=NoKey;
    //      }
    SetFanSpeedValue = 255;
    break;

  case CavityTempDown:
    if (1 == FastUpdateData)
    {
      SetCavityTempValue = SetCavityTempValue - 10; //0.4f;
    }
    else
    {
      SetCavityTempValue = SetCavityTempValue - 1; //0.1f;
      KeyValue = NoKey;
    }

    break;

  case CavityTempUp:
    if (1 == FastUpdateData)
    {
      SetCavityTempValue = SetCavityTempValue + 10; //0.4f;
    }
    else
    {
      SetCavityTempValue = SetCavityTempValue + 1; //0.1f;
      KeyValue = NoKey;
    }

    break;

  case CavityTempOnDown:
    SetCavityTempOnValue = 0;
    break;

  case CavityTempOnUp:
    SetCavityTempOnValue = 1;
    break;

  default:
    break;
  }
}

void DataRangeLimit(void)
{
  if (SetNozzleTempValue > temperature_get_heater_maxtemp(0) - 25)
    SetNozzleTempValue = temperature_get_heater_maxtemp(0) - 25;

  if (SetNozzleTempValue < 0)
    SetNozzleTempValue = 0;

  if (t_custom_services.disable_abs) //不能打印ABS,热床的可调最大温度限制
  {
    if (SetBedTempValue > 70)
      SetBedTempValue = 70;
  }
  else
  {
    if (SetBedTempValue > 120)
      SetBedTempValue = 120;

    if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id && M4040 == t_sys_data_current.model_id && SetBedTempValue > 100)
    {
      SetBedTempValue = 100;
    }
  }

  if (SetBedTempValue < 0)
    SetBedTempValue = 0;

  if (SetFanSpeedValue > 255)
    SetFanSpeedValue = 255;

  if (SetFanSpeedValue < 0)
    SetFanSpeedValue = 0;

  if (SetPrintSpeedValue > 200) //2.0f)
    SetPrintSpeedValue = 200; //2.0f;

  if (SetPrintSpeedValue < 10) //0.1f)
    SetPrintSpeedValue = 10; //0.1f;

  if (1 == t_sys.enable_cavity_temp)
  {
    if (SetCavityTempValue > 120)
    {
      SetCavityTempValue = 120;
    }
    else if (SetCavityTempValue < 0)
    {
      SetCavityTempValue = 0;
    }
  }

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    if (SetNozzleTempValue > 60)
      SetNozzleTempValue = 60;
  }
}

void DisplayText_M14(void)
{
  char Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTempValue);
  CopyTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleTempTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetFanSpeedValue);
  CopyTextDisplayRangeInfo(FanSpeedTextSharp, SetFanSpeedValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, FanSpeedTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", SetPrintSpeedValue / 100.0f);
  CopyTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, PrintSpeedTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
}

void DisplayText_NotM14_Left(void)
{
  char Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTempValue);
  CopyTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleTempTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", SetPrintSpeedValue / 100.0f);
  CopyTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, PrintSpeedTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
}

void DisplayText_NotM14_Right(void)
{
  char Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetBedTempValue);
  CopyTextDisplayRangeInfo(BedTempTextSharp, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, BedTempTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  snprintf(Textbuffer, sizeof(Textbuffer), "%s", SetFanSpeedValue > 0 ? "on" : "off");
  CopyTextDisplayRangeInfo(FanSpeedTextSharp, SetFanSpeedValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, FanSpeedTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
}

void DisplayText_Cavity(void)
{
  char Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetCavityTempValue);
  CopyTextDisplayRangeInfo(CavityTempTextSharp, TextRangeBuf_CavityTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, CavityTempTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  snprintf(Textbuffer, sizeof(Textbuffer), "%s", (SetCavityTempOnValue ? "on" : "off"));
  CopyTextDisplayRangeInfo(CavityTempOnTextSharp, SetCavityTempOnValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, CavityTempOnTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
}

void SetValue_M14(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = ScanKey_M14();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_M14();
  }
}

void SetValue_NotM14_Left(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = ScanKey_NotM14_Left();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_NotM14_Left();
  }
}

void SetValue_NotM14_Right(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = ScanKey_NotM14_Right();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_NotM14_Right();
  }
}

void GetPrintSpeedValue(void)
{
  SetPrintSpeedValue = t_gui.print_speed_value; ///100.0f;   //转换成小数  0-200   0-2.0
}

void GetFanSpeedValue(void)
{
  SetFanSpeedValue = t_gui.fan_speed_value;
}

void GetNozzleTargetTempValue(void)
{
  SetNozzleTempValue = t_gui.target_nozzle_temp[0];
}

void GetBedTargetTempValue(void)
{
  SetBedTempValue = t_gui.target_hot_bed_temp;
}

void RefreshTheInterface_M14(void)
{
  display_picture(35);
  SetTextDisplayRange(235, 35, 12 * 3, 24, &NozzleTempTextSharp);
  SetTextDisplayRange(235, 126, 12 * 3, 24, &FanSpeedTextSharp);
  ReadTextDisplayRangeInfo(FanSpeedTextSharp, SetFanSpeedValueSharpt);
  SetTextDisplayRange(235, 211, 12 * 3, 24, &PrintSpeedTextSharp);
  ReadTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp);
  ReadTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt);
  GetNozzleTargetTempValue();
  GetFanSpeedValue();
  GetPrintSpeedValue();
  DisplayText_M14();
}

void RefreshTheInterface_NotM14_Left(void)
{
  display_picture(18);
  SetTextDisplayRange(235, 75, 12 * 3, 24, &NozzleTempTextSharp);
  SetTextDisplayRange(235, 168, 12 * 3, 24, &PrintSpeedTextSharp);
  ReadTextDisplayRangeInfo(NozzleTempTextSharp, TextRangeBuf_NozzleTemp);
  ReadTextDisplayRangeInfo(PrintSpeedTextSharp, SetPrintSpeedValueSharpt);
  GetNozzleTargetTempValue();
  GetPrintSpeedValue();
  DisplayText_NotM14_Left();
}

void RefreshTheInterface_NotM14_Right(void)
{
  display_picture(41);
  SetTextDisplayRange(235, 75, 12 * 3, 24, &BedTempTextSharp);
  SetTextDisplayRange(235, 168, 12 * 3, 24, &FanSpeedTextSharp);
  ReadTextDisplayRangeInfo(BedTempTextSharp, TextRangeBuf_HotBedTemp);
  ReadTextDisplayRangeInfo(FanSpeedTextSharp, SetFanSpeedValueSharpt);
  GetBedTargetTempValue();
  GetFanSpeedValue();
  DisplayText_NotM14_Right();
}


void ConfirmKey_M14(void)
{
  SettingInfoToSYS.TargetNozzleTemp = SetNozzleTempValue;
  SettingInfoToSYS.FanSpeed = SetFanSpeedValue;
  SettingInfoToSYS.PrintSpeed = SetPrintSpeedValue;
  t_gui.print_speed_value = (uint16_t)SetPrintSpeedValue;
  respond_gui_send_sem(PrintSetValue_M14);

  if (IsLoadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(loadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(loadfilament0F);
    }
  }
  else if (IsUnloadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(unloadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(unloadfilament0F);
    }
  }
  else if (IsPowerOffRecoverReady)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}

void ConfirmKey_NotM14_Left(void)
{
  SettingInfoToSYS.TargetNozzleTemp = SetNozzleTempValue;
  SettingInfoToSYS.PrintSpeed = SetPrintSpeedValue;
  t_gui.print_speed_value = (uint16_t)(SetPrintSpeedValue);
  respond_gui_send_sem(PrintSetValue_NotM14_Left);

  if (IsLoadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(loadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(loadfilament0F);
    }
  }
  else if (IsUnloadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(unloadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(unloadfilament0F);
    }
  }
  else if (IsPowerOffRecoverReady)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}

void ConfirmKey_NotM14_Right(void)
{
  SettingInfoToSYS.TargetHotbedTemp = SetBedTempValue;
  SettingInfoToSYS.FanSpeed = SetFanSpeedValue;
  respond_gui_send_sem(PrintSetValue_NotM14_Right);

  if (IsPowerOffRecoverReady)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}

void CancelKey(void)
{
  if (IsLoadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(loadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(loadfilament0F);
    }
  }
  else if (IsUnloadFilamentInterface)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(unloadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(unloadfilament0F);
    }
  }
  else if (IsPowerOffRecoverReady)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}
extern bool IsPrintSDFile(void);
void PrintSet_M14(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_M14();
    return;
  }

  if (touchxy(110, 255, 240, 320)) //确认键
  {
    ConfirmKey_M14();
    return ;
  }

  if (touchxy(240, 265, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_M14();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

void PrintSet_NotM14_Left(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_NotM14_Left();
    return;
  }

  if (lcddev.height == 272)
  {
    if (touchxy(110, 220, 240, 272)) //确认键
    {
      ConfirmKey_NotM14_Left();
      return ;
    }

    if (touchxy(240, 225, 380, 272)) //取消键
    {
      CancelKey();
      return ;
    }
  }
  else
  {
    if (touchxy(110, 230, 240, 320)) //确认键
    {
      ConfirmKey_NotM14_Left();
      return ;
    }

    if (touchxy(240, 235, 380, 320)) //取消键
    {
      CancelKey();
      return ;
    }
  }

  SetValue_NotM14_Left();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

void PrintSet_NotM14_Right(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_NotM14_Right();
    return;
  }

  if (lcddev.height == 272)
  {
    if (touchxy(110, 220, 240, 272)) //确认键
    {
      ConfirmKey_NotM14_Right();
      return ;
    }

    if (touchxy(240, 225, 380, 272)) //取消键
    {
      CancelKey();
      return ;
    }
  }
  else
  {
    if (touchxy(110, 230, 240, 320)) //确认键
    {
      ConfirmKey_NotM14_Right();
      return ;
    }

    if (touchxy(240, 235, 380, 320)) //取消键
    {
      CancelKey();
      return ;
    }
  }

  SetValue_NotM14_Right();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

void ConfirmKey_Cavity(void)
{
  SettingInfoToSYS.TargetCavityTemp = SetCavityTempValue;
  SettingInfoToSYS.TargetCavityOnTemp = SetCavityTempOnValue;
  respond_gui_send_sem(PrintSetValue_Cavity);

  if (IsPowerOffRecoverReady)
  {
    gui::set_current_display(PowerOffRecoverReady);
  }
  else
  {
    gui::set_current_display(maindisplayF);
  }
}

void SetValue_Cavity(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = ScanKey_Cavity();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_Cavity();
  }
}

void RefreshTheInterface_Cavity(void)
{
  display_picture(109);
  SetTextDisplayRange(235, 75, 12 * 3, 24, &CavityTempTextSharp);
  SetTextDisplayRange(235, 168, 12 * 3, 24, &CavityTempOnTextSharp);
  ReadTextDisplayRangeInfo(CavityTempTextSharp, TextRangeBuf_CavityTargetTemp);
  ReadTextDisplayRangeInfo(CavityTempOnTextSharp, SetCavityTempOnValueSharpt);
  SetCavityTempValue = t_gui.target_cavity_temp;
  SetCavityTempOnValue = t_gui.target_cavity_temp_on;
  DisplayText_Cavity();
}

void PrintSet_Cavity(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_Cavity();
    return;
  }

  if (touchxy(110, 230, 240, 320)) //确认键
  {
    ConfirmKey_Cavity();
    return ;
  }

  if (touchxy(240, 235, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_Cavity();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

void SetLoadFilamentNozzleTemp(void)
{
  IsLoadFilamentInterface = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsLoadFilamentInterface = 0;
}

void SetUnLoadFilamentNozzleTemp(void)
{
  IsUnloadFilamentInterface = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsUnloadFilamentInterface = 0;
}

void SetPowerOffRecoverNozzleTemp(void)
{
  IsPowerOffRecoverReady = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsPowerOffRecoverReady = 0;
}

void SetPowerOffRecoverHotBedTemp(void)
{
  IsPowerOffRecoverReady = 1;
  PrintSet_NotM14_Right();
  IsPowerOffRecoverReady = 0;
}

#ifdef __cplusplus
} //extern "C" {
#endif


