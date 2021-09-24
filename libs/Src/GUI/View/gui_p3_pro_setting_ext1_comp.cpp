#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "flashconfig.h"
#include "user_ccm.h"

#define NoKey 0
#define Nozzle_X_Down 1
#define Nozzle_X_Up 2
#define Nozzle_Y_Down 3
#define Nozzle_Y_Up 4
#define Nozzle_Z_Down 5
#define Nozzle_Z_Up 6

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_0
#define SetNozzleXValueSharpt ccm_param::TextRangeBuf_24_12_9_1
#define SetNozzleYValueSharpt ccm_param::TextRangeBuf_24_12_9_2
#define SetNozzleZValueSharpt ccm_param::TextRangeBuf_24_12_9_3

#define Timeoutms 200
#define FirstTimeoutms 600

#ifdef __cplusplus
extern "C" {
#endif

extern bool is_auto_bed_level_zero(void);
extern int temperature_get_heater_maxtemp(int axis);

static struct _textrange  NozzleXTextSharp;
static struct _textrange  NozzleYTextSharp;
static struct _textrange  NozzleZTextSharp;

static uint16_t TextColor = 0xf800; //数值显示的颜色--红色

static uint8_t KeyValue = NoKey;

static unsigned long Timeout = 0;
static unsigned long FirstTimeout = 0;

static float SetNozzleXValue = 0;
static float SetNozzleYValue = 0;
static float SetNozzleZValue = 100;

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t ScanKey_NotM14_Left_dual(void)
{
  if (IstouchxyDown(284, 14, 336, 68))
  {
    return Nozzle_X_Up ;
  }
  else if (IstouchxyDown(358, 14, 412, 68))
  {
    return Nozzle_X_Down;
  }
  else if (IstouchxyDown(284, 82, 336, 132))
  {
    return Nozzle_Y_Up ;
  }
  else if (IstouchxyDown(358, 82, 412, 132))
  {
    return Nozzle_Y_Down;
  }
  else if (IstouchxyDown(284, 144, 336, 196))
  {
    return Nozzle_Z_Up ;
  }
  else if (IstouchxyDown(358, 144, 412, 196))
  {
    return Nozzle_Z_Down;
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
  case Nozzle_X_Down:
    if (1 == FastUpdateData)
    {
      SetNozzleXValue = SetNozzleXValue - 0.1f;
    }
    else
    {
      SetNozzleXValue = SetNozzleXValue - 0.01f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_X_Up:
    if (1 == FastUpdateData)
    {
      SetNozzleXValue = SetNozzleXValue + 0.1f;
    }
    else
    {
      SetNozzleXValue = SetNozzleXValue + 0.01f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Down:
    if (1 == FastUpdateData)
    {
      SetNozzleYValue = SetNozzleYValue - 0.1f;
    }
    else
    {
      SetNozzleYValue = SetNozzleYValue - 0.01f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Up:
    if (1 == FastUpdateData)
    {
      SetNozzleYValue = SetNozzleYValue + 0.1f;
    }
    else
    {
      SetNozzleYValue = SetNozzleYValue + 0.01f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Z_Down:
    if (1 == FastUpdateData)
    {
      SetNozzleZValue = SetNozzleZValue - 0.1f; //0.4f;
    }
    else
    {
      SetNozzleZValue = SetNozzleZValue - 0.01f; //0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Z_Up:
    if (1 == FastUpdateData)
    {
      SetNozzleZValue = SetNozzleZValue + 0.1f; //0.4f;
    }
    else
    {
      SetNozzleZValue = SetNozzleZValue + 0.01f; //0.1f;
      KeyValue = NoKey;
    }

    break;

  default:
    break;
  }
}

static void DataRangeLimit_dual(void)
{
  if (SetNozzleXValue > 50.0f)
    SetNozzleXValue = 50.0f;

  if (SetNozzleXValue < -50.0f)
    SetNozzleXValue = -50.0f;

  if (SetNozzleYValue > 50.0f)
    SetNozzleYValue = 50.0f;

  if (SetNozzleYValue < -50.0f)
    SetNozzleYValue = -50.0f;

  if (SetNozzleZValue > 50.0f)
    SetNozzleZValue = 50.0f;

  if (SetNozzleZValue < -50.0f)
    SetNozzleZValue = -50.0f;
}

static void DisplayText_NotM14_Left_dual(void)
{
  char Textbuffer[20];
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.2f", SetNozzleXValue);
  CopyTextDisplayRangeInfo(NozzleXTextSharp, SetNozzleXValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleXTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.2f", SetNozzleYValue);
  CopyTextDisplayRangeInfo(NozzleYTextSharp, SetNozzleYValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleYTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.2f", SetNozzleZValue);
  CopyTextDisplayRangeInfo(NozzleZTextSharp, SetNozzleZValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleZTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
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
  flash_param_t.idex_ext1_ext0_offset[0] = SetNozzleXValue;
  flash_param_t.idex_ext1_ext0_offset[1] = SetNozzleYValue;
  flash_param_t.idex_ext1_ext0_offset[2] = SetNozzleZValue;
  flash_param_t.flag = 1;
  gui::set_current_display(gui_p3_pro_setting_model_set);
}

static void CancelKey_dual(void)
{
  gui::set_current_display(gui_p3_pro_setting_model_set);
}
extern bool IsPrintSDFile(void);

static void RefreshTheInterface_NotM14_Left_dual(void)
{
  display_picture(158);
  SetTextDisplayRangeNormal(234, 32, 12 * 5, 24, &NozzleXTextSharp);
  SetTextDisplayRangeNormal(234, 96, 12 * 5, 24, &NozzleYTextSharp);
  SetTextDisplayRangeNormal(234, 158, 12 * 5, 24, &NozzleZTextSharp);
  ReadTextDisplayRangeInfo(NozzleXTextSharp, SetNozzleXValueSharpt);
  ReadTextDisplayRangeInfo(NozzleYTextSharp, SetNozzleYValueSharpt);
  ReadTextDisplayRangeInfo(NozzleZTextSharp, SetNozzleZValueSharpt);
  SetNozzleXValue = flash_param_t.idex_ext1_ext0_offset[0];
  SetNozzleYValue = flash_param_t.idex_ext1_ext0_offset[1];
  SetNozzleZValue = flash_param_t.idex_ext1_ext0_offset[2]; ///100.0f;   //转换成小数  0-200   0-2.0
  DisplayText_NotM14_Left_dual();
}

void gui_p3_pro_setting_ext1_comp(void)
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
}

#ifdef __cplusplus
} //extern "C" {
#endif


