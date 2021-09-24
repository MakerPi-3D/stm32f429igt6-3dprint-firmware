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
#include "user_common.h"

#define NoKey 0
#define BED_MIN_Down 1
#define BED_MIN_Up 2
#define BED_MAX_Down 3
#define BED_MAX_Up 4

#define LEFT 1
#define RIGHT 2

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_0
#define SetNozzleXValueSharpt ccm_param::TextRangeBuf_24_12_9_1
#define SetNozzleYValueSharpt ccm_param::TextRangeBuf_24_12_9_2

#define Timeoutms 200
#define FirstTimeoutms 600

#ifdef __cplusplus
extern "C" {
#endif

extern bool is_auto_bed_level_zero(void);
extern int temperature_get_heater_maxtemp(int axis);

static struct _textrange  NozzleXTextSharp;
static struct _textrange  NozzleYTextSharp;

volatile uint8_t idex_ext_bed_index = 0;

static uint16_t TextColor = 0xf800; //数值显示的颜色--红色

static uint8_t KeyValue = NoKey;
static uint8_t left_or_right = NoKey;

static unsigned long Timeout = 0;
static unsigned long FirstTimeout = 0;

static int extruder_bed_value[2] = {0};
static int extruder_bed_value_bak[2] = {0};

static uint8_t SlowUpdateData = 0;
static uint8_t FastUpdateData = 0;

static uint8_t ScanKey_NotM14_Left_dual(void)
{
  if (IstouchxyDown(244, 86, 296, 138))
  {
    return BED_MIN_Up ;
  }
  else if (IstouchxyDown(312, 86, 362, 138))
  {
    return BED_MIN_Down;
  }
  else if (IstouchxyDown(244, 160, 296, 214))
  {
    return BED_MAX_Up ;
  }
  else if (IstouchxyDown(312, 162, 362, 214))
  {
    return BED_MAX_Down;
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
  case BED_MIN_Down:
    if (1 == FastUpdateData)
    {
      extruder_bed_value[0] = extruder_bed_value[0] - 10.0f;
    }
    else
    {
      extruder_bed_value[0] = extruder_bed_value[0] - 1.0f;
      KeyValue = NoKey;
    }

    left_or_right = LEFT;
    break;

  case BED_MIN_Up:
    if (1 == FastUpdateData)
    {
      extruder_bed_value[0] = extruder_bed_value[0] + 10.0f;
    }
    else
    {
      extruder_bed_value[0] = extruder_bed_value[0] + 1.0f;
      KeyValue = NoKey;
    }

    left_or_right = LEFT;
    break;

  case BED_MAX_Down:
    if (1 == FastUpdateData)
    {
      extruder_bed_value[1] = extruder_bed_value[1] - 10.0f;
    }
    else
    {
      extruder_bed_value[1] = extruder_bed_value[1] - 1.0f;
      KeyValue = NoKey;
    }

    left_or_right = RIGHT;
    break;

  case BED_MAX_Up:
    if (1 == FastUpdateData)
    {
      extruder_bed_value[1] = extruder_bed_value[1] + 10.0f;
    }
    else
    {
      extruder_bed_value[1] = extruder_bed_value[1] + 1.0f;
      KeyValue = NoKey;
    }

    left_or_right = RIGHT;
    break;

  default:
    left_or_right = 0;
    break;
  }
}

/*
* 从屏幕上获取的值并经过限制处理后赋值到flash_param_t结构体
*
*/
static void update_data_to_flash_buff(void)
{
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    if (idex_ext_bed_index == 0)
    {
      if (left_or_right == LEFT)
        flash_param_t.idex_extruder_0_bed_offset[0] = extruder_bed_value[0];
      else if (left_or_right == RIGHT)
        flash_param_t.idex_extruder_0_bed_offset[1] = extruder_bed_value[1];
    }
    else if (idex_ext_bed_index == 1)
    {
      if (left_or_right == LEFT)
        flash_param_t.idex_extruder_1_bed_offset[0] = extruder_bed_value[0];
      else if (left_or_right == RIGHT)
        flash_param_t.idex_extruder_1_bed_offset[1] = extruder_bed_value[1];
    }
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    if (left_or_right == LEFT)
      flash_param_t.mix_extruder_0_bed_offset[0] = extruder_bed_value[0];
    else if (left_or_right == RIGHT)
      flash_param_t.mix_extruder_0_bed_offset[1] = extruder_bed_value[1];
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    if (left_or_right == LEFT)
      flash_param_t.laser_extruder_0_bed_offset[0] = extruder_bed_value[0];
    else if (left_or_right == RIGHT)
      flash_param_t.laser_extruder_0_bed_offset[1] = extruder_bed_value[1];
  }
}

static void DataRangeLimit_dual(void)
{
  if (idex_ext_bed_index == 0)
  {
    if (extruder_bed_value[0] > ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS])
      extruder_bed_value[0] = ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS];

    if (extruder_bed_value[1] > ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS])
      extruder_bed_value[1] = ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS];
  }
  else if (idex_ext_bed_index == 1)
  {
    if (extruder_bed_value[0] > ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS])
      extruder_bed_value[0] = ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS];

    if (extruder_bed_value[1] > ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS])
      extruder_bed_value[1] = ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS];
  }

  if (extruder_bed_value[0] < 0.0f)
    extruder_bed_value[0] = 0.0f;

  if (extruder_bed_value[1] < 0.0f)
    extruder_bed_value[1] = 0.0f;

  update_data_to_flash_buff();
}

static void DisplayText_NotM14_Left_dual(void)
{
  char Textbuffer[20];
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", extruder_bed_value[0]);
  CopyTextDisplayRangeInfo(NozzleXTextSharp, SetNozzleXValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleXTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
  //
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", extruder_bed_value[1]);
  CopyTextDisplayRangeInfo(NozzleYTextSharp, SetNozzleYValueSharpt, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)Textbuffer, NozzleYTextSharp, TextRangeBuf_Str, 24, (u16)TextColor);
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

static void key_move_to_bed(int index)
{
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    if (idex_ext_bed_index == 0)
    {
      flash_param_t.idex_extruder_0_bed_offset[index] = extruder_bed_value[index];
    }
    else if (idex_ext_bed_index == 1)
    {
      flash_param_t.idex_extruder_1_bed_offset[index] = extruder_bed_value[index];
    }
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    flash_param_t.mix_extruder_0_bed_offset[index] = extruder_bed_value[index];
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    if (idex_ext_bed_index == 0)
    {
      flash_param_t.laser_extruder_0_bed_offset[index] = extruder_bed_value[index];
    }
    else if (idex_ext_bed_index == 1)
    {
      flash_param_t.laser_extruder_1_bed_offset[index] = extruder_bed_value[index];
    }
  }

  if (idex_ext_bed_index == 0)
  {
    gui_respond_data.active_extruder = 0;
    gui_respond_data.x_move_value = extruder_bed_value[index];
    respond_gui_send_sem(IdexMoveX);
  }
  else if (idex_ext_bed_index == 1)
  {
    gui_respond_data.active_extruder = 1;
    gui_respond_data.x_move_value = extruder_bed_value[index];
    respond_gui_send_sem(IdexMoveX);
  }

  osDelay(50);
}

static void key_move_to_bed_min(void)
{
  key_move_to_bed(0);
}

static void key_move_to_bed_max(void)
{
  key_move_to_bed(1);
}

static void RefreshTheInterface_NotM14_Left_dual(void)
{
  display_picture(167);
  respond_gui_send_sem(BackZeroValue);

//  if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER && idex_ext_bed_index == 1) //激光头在2号头，移动需要先上升10mm，避免撞击
  {
    respond_gui_send_sem(LaserMoveZUp);
  }

  SetTextDisplayRangeNormal(195, 100, 12 * 5, 24, &NozzleXTextSharp);
  SetTextDisplayRangeNormal(195, 174, 12 * 5, 24, &NozzleYTextSharp);
  ReadTextDisplayRangeInfo(NozzleXTextSharp, SetNozzleXValueSharpt);
  ReadTextDisplayRangeInfo(NozzleYTextSharp, SetNozzleYValueSharpt);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    if (idex_ext_bed_index == 0)
    {
      extruder_bed_value[0] = flash_param_t.idex_extruder_0_bed_offset[0];
      extruder_bed_value[1] = flash_param_t.idex_extruder_0_bed_offset[1];
    }
    else if (idex_ext_bed_index == 1)
    {
      extruder_bed_value[0] = flash_param_t.idex_extruder_1_bed_offset[0];
      extruder_bed_value[1] = flash_param_t.idex_extruder_1_bed_offset[1];
    }
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    extruder_bed_value[0] = flash_param_t.mix_extruder_0_bed_offset[0];
    extruder_bed_value[1] = flash_param_t.mix_extruder_0_bed_offset[1];
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    if (idex_ext_bed_index == 0)
    {
      extruder_bed_value[0] = flash_param_t.laser_extruder_0_bed_offset[0];
      extruder_bed_value[1] = flash_param_t.laser_extruder_0_bed_offset[1];
    }
    else if (idex_ext_bed_index == 1)
    {
      extruder_bed_value[0] = flash_param_t.laser_extruder_1_bed_offset[0];
      extruder_bed_value[1] = flash_param_t.laser_extruder_1_bed_offset[1];
    }
  }

  extruder_bed_value_bak[0] = extruder_bed_value[0];
  extruder_bed_value_bak[1] = extruder_bed_value[1];
  DisplayText_NotM14_Left_dual();
}

void gui_p3_pro_setting_ext1_bed(void)
{
  if (gui::is_refresh())
  {
    RefreshTheInterface_NotM14_Left_dual();
    return;
  }

  if (touchxy(374, 88, 458, 134)) //移动左边
  {
    key_move_to_bed_min();
    return ;
  }

  if (touchxy(374, 162, 458, 210)) //移动右边
  {
    key_move_to_bed_max();
    return ;
  }

  if (touchxy(36, 28, 116, 60)) //返回键
  {
    if (extruder_bed_value[0] != extruder_bed_value_bak[0] || extruder_bed_value[1] != extruder_bed_value_bak[1])
    {
      flash_param_t.flag = 1;
    }

    gui::set_current_display(gui_p3_pro_setting_model_set);
    return ;
  }

  SetValue_NotM14_Left_dual();
}

#ifdef __cplusplus
} //extern "C" {
#endif


