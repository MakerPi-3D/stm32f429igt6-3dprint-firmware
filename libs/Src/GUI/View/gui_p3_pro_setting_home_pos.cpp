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
#include "gcode.h"
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

static int dual_home_pos_x_value = 0;
static int dual_home_pos_y_value = 0;
static float dual_home_pos_z_value = 100;

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t dual_home_pos_scan_key(void)
{
  if (IstouchxyDown(284, 28, 336, 82))
  {
    return Nozzle_X_Up ;
  }
  else if (IstouchxyDown(358, 28, 412, 82))
  {
    return Nozzle_X_Down;
  }
  else if (IstouchxyDown(284, 96, 336, 146))
  {
    return Nozzle_Y_Up ;
  }
  else if (IstouchxyDown(358, 96, 412, 146))
  {
    return Nozzle_Y_Down;
  }
  else if (IstouchxyDown(284, 158, 336, 210))
  {
    return Nozzle_Z_Up ;
  }
  else if (IstouchxyDown(358, 158, 412, 210))
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
      dual_home_pos_x_value = dual_home_pos_x_value - 10.0f;
    }
    else
    {
      dual_home_pos_x_value = dual_home_pos_x_value - 1.0f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_X_Up:
    if (1 == FastUpdateData)
    {
      dual_home_pos_x_value = dual_home_pos_x_value + 10.0f;
    }
    else
    {
      dual_home_pos_x_value = dual_home_pos_x_value + 1.0f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Down:
    if (1 == FastUpdateData)
    {
      dual_home_pos_y_value = dual_home_pos_y_value - 10.0f;
    }
    else
    {
      dual_home_pos_y_value = dual_home_pos_y_value - 1.0f;
      KeyValue = NoKey;
    }

    break;

  case Nozzle_Y_Up:
    if (1 == FastUpdateData)
    {
      dual_home_pos_y_value = dual_home_pos_y_value + 10.0f;
    }
    else
    {
      dual_home_pos_y_value = dual_home_pos_y_value + 1.0f;
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
  if (dual_home_pos_x_value > 150.0f)
    dual_home_pos_x_value = 150.0f;

  if (dual_home_pos_x_value < 0.0f)
    dual_home_pos_x_value = 0.0f;

  if (dual_home_pos_y_value > 150.0f)
    dual_home_pos_y_value = 150.0f;

  if (dual_home_pos_y_value < 0.0f)
    dual_home_pos_y_value = 0.0f;

  if (dual_home_pos_z_value > 50.0f)
    dual_home_pos_z_value = 50.0f;

  if (dual_home_pos_z_value < -50.0f)
    dual_home_pos_z_value = -50.0f;
}

static void dual_home_pos_display_text(void)
{
  char Textbuffer[20];
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", dual_home_pos_x_value);
  CopyTextDisplayRangeInfo(dual_home_pos_x_text_range, DUAL_HOME_X_BUF, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, dual_home_pos_x_text_range, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", dual_home_pos_y_value);
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
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    flash_param_t.idex_ext0_home_pos_adding[0] = dual_home_pos_x_value;
    flash_param_t.idex_ext0_home_pos_adding[1] = dual_home_pos_y_value;
    flash_param_t.idex_ext0_home_pos_adding[2] = dual_home_pos_z_value;
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    flash_param_t.mix_ext0_home_pos_adding[0] = dual_home_pos_x_value;
    flash_param_t.mix_ext0_home_pos_adding[1] = dual_home_pos_y_value;
    flash_param_t.mix_ext0_home_pos_adding[2] = dual_home_pos_z_value;
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    flash_param_t.laser_ext0_home_pos_adding[0] = dual_home_pos_x_value;
    flash_param_t.laser_ext0_home_pos_adding[1] = dual_home_pos_y_value;
    flash_param_t.laser_ext0_home_pos_adding[2] = dual_home_pos_z_value;
  }
  else
  {
    flash_param_t.dual_home_pos_adding[0] = dual_home_pos_x_value;
    flash_param_t.dual_home_pos_adding[1] = dual_home_pos_y_value;
    flash_param_t.dual_home_pos_adding[2] = dual_home_pos_z_value;
  }

  flash_param_t.flag = 1;
  gui::set_current_display(gui_p3_pro_setting_model_set);
}
static char moveXYZ[55];
static void dual_home_pos_cancel_key(void)
{
  //  gui::set_current_display(gui_p3_pro_setting_model_set);
  memset(moveXYZ, 0, sizeof(moveXYZ));
  (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f Z%f", (float)dual_home_pos_x_value, (float)dual_home_pos_y_value, (float)dual_home_pos_z_value);
  user_send_internal_cmd((char *)moveXYZ);
}
extern bool IsPrintSDFile(void);

static void dual_home_pos_refresh_interface(void)
{
  display_picture(161);
  SetTextDisplayRangeNormal(234, 55, 12 * 5, 24, &dual_home_pos_x_text_range);
  SetTextDisplayRangeNormal(234, 115, 12 * 5, 24, &dual_home_pos_y_text_range);
  SetTextDisplayRangeNormal(234, 174, 12 * 5, 24, &dual_home_pos_z_text_range);
  ReadTextDisplayRangeInfo(dual_home_pos_x_text_range, DUAL_HOME_X_BUF);
  ReadTextDisplayRangeInfo(dual_home_pos_y_text_range, DUAL_HOME_Y_BUF);
  ReadTextDisplayRangeInfo(dual_home_pos_z_text_range, DUAL_HOME_Z_BUF);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    dual_home_pos_x_value = flash_param_t.idex_ext0_home_pos_adding[0];
    dual_home_pos_y_value = flash_param_t.idex_ext0_home_pos_adding[1];
    dual_home_pos_z_value = flash_param_t.idex_ext0_home_pos_adding[2];   //转换成小数  0-200   0-2.0
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    dual_home_pos_x_value = flash_param_t.mix_ext0_home_pos_adding[0];
    dual_home_pos_y_value = flash_param_t.mix_ext0_home_pos_adding[1];
    dual_home_pos_z_value = flash_param_t.mix_ext0_home_pos_adding[2];   //转换成小数  0-200   0-2.0
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    dual_home_pos_x_value = flash_param_t.laser_ext0_home_pos_adding[0];
    dual_home_pos_y_value = flash_param_t.laser_ext0_home_pos_adding[1];
    dual_home_pos_z_value = flash_param_t.laser_ext0_home_pos_adding[2];   //转换成小数  0-200   0-2.0
  }
  else
  {
    dual_home_pos_x_value = flash_param_t.dual_home_pos_adding[0];
    dual_home_pos_y_value = flash_param_t.dual_home_pos_adding[1];
    dual_home_pos_z_value = flash_param_t.dual_home_pos_adding[2];   //转换成小数  0-200   0-2.0
  }

  dual_home_pos_z_value = 5.0f; // Default elevation z height

  dual_home_pos_display_text();
  respond_gui_send_sem(BackZeroValue);
  float x, y, z;
  gcode::g28_get_home_pos_adding(-1, x, y, z);
  memset(moveXYZ, 0, sizeof(moveXYZ));
  (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f Z10", (float)dual_home_pos_x_value, (float)dual_home_pos_y_value);
  user_send_internal_cmd((char *)moveXYZ);
  memset(moveXYZ, 0, sizeof(moveXYZ));
  (void)snprintf(moveXYZ, sizeof(moveXYZ), "G92 Z%f", 10 + z);
  user_send_internal_cmd((char *)moveXYZ);
  memset(moveXYZ, 0, sizeof(moveXYZ));
  (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f Z%f", (float)dual_home_pos_x_value, (float)dual_home_pos_y_value, (float)dual_home_pos_z_value);
  user_send_internal_cmd((char *)moveXYZ);
}

void gui_p3_pro_setting_home_pos(void)
{
  if (gui::is_refresh())
  {
    dual_home_pos_refresh_interface();
    return;
  }

  if (touchxy(150, 220 + 20, 228, 252 + 20)) //确认键
  {
    dual_home_pos_confirm_key();
    return ;
  }

  if (touchxy(264, 220 + 20, 342, 252 + 20)) //取消键
  {
    dual_home_pos_cancel_key();
    return ;
  }

  dual_home_pos_set_value();
}

#ifdef __cplusplus
} //extern "C" {
#endif


