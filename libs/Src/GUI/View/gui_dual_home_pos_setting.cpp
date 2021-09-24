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
#define DUAL_HOME_X_BUF ccm_param::TextRangeBuf_24_12_9_1
#define DUAL_HOME_Y_BUF ccm_param::TextRangeBuf_24_12_9_2
#define DUAL_HOME_Z_BUF ccm_param::TextRangeBuf_24_12_9_3

#define Timeoutms 200
#define FirstTimeoutms 600

#ifdef __cplusplus
extern "C" {
#endif

extern bool is_auto_bed_level_zero(void);
extern int temperature_get_heater_maxtemp(int axis);

static struct _textrange  dual_home_pos_x_text_range;
static struct _textrange  dual_home_pos_y_text_range;
static struct _textrange  dual_home_pos_z_text_range;

static uint16_t TextColor = 0xf800; //数值显示的颜色--红色

static uint8_t KeyValue = NoKey;

static unsigned long Timeout = 0;
static unsigned long FirstTimeout = 0;

static float dual_home_pos_x_value = 0;
static float dual_home_pos_y_value = 0;
static float dual_home_pos_z_value = 100;

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t dual_home_pos_scan_key(void)
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

static void dual_home_pos_long_press(void)
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

static void dual_home_pos_short_press(void)
{
  SlowUpdateData = 1;
  FastUpdateData = 0;
}

static void dual_home_pos_update_value(void)
{
  switch (KeyValue)
  {
  case Nozzle_X_Down:
    if (1 == FastUpdateData)
    {
      dual_home_pos_x_value = dual_home_pos_x_value - 1.0f;
    }
    else
    {
      dual_home_pos_x_value = dual_home_pos_x_value - 0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_X_Up:
    if (1 == FastUpdateData)
    {
      dual_home_pos_x_value = dual_home_pos_x_value + 1.0f;
    }
    else
    {
      dual_home_pos_x_value = dual_home_pos_x_value + 0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Down:
    if (1 == FastUpdateData)
    {
      dual_home_pos_y_value = dual_home_pos_y_value - 1.0f;
    }
    else
    {
      dual_home_pos_y_value = dual_home_pos_y_value - 0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Up:
    if (1 == FastUpdateData)
    {
      dual_home_pos_y_value = dual_home_pos_y_value + 1.0f;
    }
    else
    {
      dual_home_pos_y_value = dual_home_pos_y_value + 0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Z_Down:
    if (1 == FastUpdateData)
    {
      dual_home_pos_z_value = dual_home_pos_z_value - 1.0f; //0.4f;
    }
    else
    {
      dual_home_pos_z_value = dual_home_pos_z_value - 0.1f; //0.1f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Z_Up:
    if (1 == FastUpdateData)
    {
      dual_home_pos_z_value = dual_home_pos_z_value + 1.0f; //0.4f;
    }
    else
    {
      dual_home_pos_z_value = dual_home_pos_z_value + 0.1f; //0.1f;
      KeyValue = NoKey;
    }

    break;

  default:
    break;
  }
}

static void dual_home_pos_data_range_limit(void)
{
  if (dual_home_pos_x_value > 10.0f)
    dual_home_pos_x_value = 10.0f;

  if (dual_home_pos_x_value < 0.0f)
    dual_home_pos_x_value = 0.0f;

  if (dual_home_pos_y_value > 10.0f)
    dual_home_pos_y_value = 10.0f;

  if (dual_home_pos_y_value < 0.0f)
    dual_home_pos_y_value = 0.0f;

  if (dual_home_pos_z_value > 10.0f)
    dual_home_pos_z_value = 10.0f;

  if (dual_home_pos_z_value < 0.0f)
    dual_home_pos_z_value = 0.0f;
}

static void dual_home_pos_display_text(void)
{
  char Textbuffer[20];
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", dual_home_pos_x_value);
  CopyTextDisplayRangeInfo(dual_home_pos_x_text_range, DUAL_HOME_X_BUF, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, dual_home_pos_x_text_range, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", dual_home_pos_y_value);
  CopyTextDisplayRangeInfo(dual_home_pos_y_text_range, DUAL_HOME_Y_BUF, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, dual_home_pos_y_text_range, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", dual_home_pos_z_value);
  CopyTextDisplayRangeInfo(dual_home_pos_z_text_range, DUAL_HOME_Z_BUF, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, dual_home_pos_z_text_range, TextRangeBuf_Str, 24, (u16)TextColor);
}

static void dual_home_pos_set_value(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + xTaskGetTickCount();
    FirstTimeout = FirstTimeoutms + xTaskGetTickCount();
    KeyValue = dual_home_pos_scan_key();
  }
  else
  {
    if (FirstTimeout < xTaskGetTickCount())
    {
      dual_home_pos_long_press();
    }
    else if (IstouchxyUp())
    {
      dual_home_pos_short_press();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    dual_home_pos_update_value();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    dual_home_pos_data_range_limit();
    dual_home_pos_display_text();
  }
}

static void dual_home_pos_confirm_key(void)
{
  flash_param_t.dual_home_pos_adding[0] = dual_home_pos_x_value;
  flash_param_t.dual_home_pos_adding[1] = dual_home_pos_y_value;
  flash_param_t.dual_home_pos_adding[2] = dual_home_pos_z_value;
  flash_param_t.flag = 1;
  gui::set_current_display(maindisplayF);
}

static void dual_home_pos_cancel_key(void)
{
  gui::set_current_display(maindisplayF);
}
extern bool IsPrintSDFile(void);

static void dual_home_pos_refresh_interface(void)
{
  display_picture(161);
  SetTextDisplayRangeNormal(234, 32, 12 * 5, 24, &dual_home_pos_x_text_range);
  SetTextDisplayRangeNormal(234, 96, 12 * 5, 24, &dual_home_pos_y_text_range);
  SetTextDisplayRangeNormal(234, 158, 12 * 5, 24, &dual_home_pos_z_text_range);
  ReadTextDisplayRangeInfo(dual_home_pos_x_text_range, DUAL_HOME_X_BUF);
  ReadTextDisplayRangeInfo(dual_home_pos_y_text_range, DUAL_HOME_Y_BUF);
  ReadTextDisplayRangeInfo(dual_home_pos_z_text_range, DUAL_HOME_Z_BUF);
  dual_home_pos_x_value = flash_param_t.dual_home_pos_adding[0];
  dual_home_pos_y_value = flash_param_t.dual_home_pos_adding[1];
  dual_home_pos_z_value = flash_param_t.dual_home_pos_adding[2];   //转换成小数  0-200   0-2.0
  dual_home_pos_display_text();
}

void dual_home_pos_setting(void)
{
  if (gui::is_refresh())
  {
    dual_home_pos_refresh_interface();
    return;
  }

  if (touchxy(150, 210 + 20, 228, 252 + 20)) //确认键
  {
    dual_home_pos_confirm_key();
    return ;
  }

  if (touchxy(264, 210 + 20, 342, 252 + 20)) //取消键
  {
    dual_home_pos_cancel_key();
    return ;
  }

  dual_home_pos_set_value();

  if ((pauseprint && print_flage) || (print_flage && (ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif


