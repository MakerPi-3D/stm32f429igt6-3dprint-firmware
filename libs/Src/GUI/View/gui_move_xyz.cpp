#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_motion_3d.h"
#include "user_ccm.h"

#define NoClick 0
#define XPosDown 1
#define XPosUp 2
#define YPosDown 3
#define YPosUp 4
#define ZPosDown 5
#define ZPosUp 6

#define ClickTimeoutms 200
#define ClickFirstTimeoutms 600


#ifdef __cplusplus
extern "C" {
#endif

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1

#define TextRangeBuf_XPosValue ccm_param::TextRangeBuf_24_12_3_0
#define TextRangeBuf_YPosValue ccm_param::TextRangeBuf_24_12_3_1
#define TextRangeBuf_ZPosValue ccm_param::TextRangeBuf_24_12_3_2

static struct _textrange  XPosTextShape;
static struct _textrange  YPosTextShape;
static struct _textrange  ZPosTextShape;

static uint16_t XYZPosTextColor = 0xf800; //数值显示的颜色--红色

static char TextXYZPosbuffer[20];

static uint8_t ClickKeyValue = NoClick;
static unsigned long ClickTimeout = 0;
static unsigned long ClickFirstTimeout = 0;
static uint8_t SlowUpdateXYZPos = 0;
static uint8_t FastUpdateXYZPos = 0;

void DisplayXYZPosText(void)
{
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[X_AXIS]);
  CopyTextDisplayRangeInfo(XPosTextShape, TextRangeBuf_XPosValue, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextXYZPosbuffer, XPosTextShape, TextRangeBuf_Str, 24, (u16)XYZPosTextColor);
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[Y_AXIS]);
  CopyTextDisplayRangeInfo(YPosTextShape, TextRangeBuf_YPosValue, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextXYZPosbuffer, YPosTextShape, TextRangeBuf_Str, 24, (u16)XYZPosTextColor);
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[Z_AXIS]);
  CopyTextDisplayRangeInfo(ZPosTextShape, TextRangeBuf_ZPosValue, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextXYZPosbuffer, ZPosTextShape, TextRangeBuf_Str, 24, (u16)XYZPosTextColor);
}


void RefreshMoveXYZInterface(void)
{
  display_picture(26);
  SetTextDisplayRange(235, 35, 12 * 3, 24, &XPosTextShape);
  ReadTextDisplayRangeInfo(XPosTextShape, TextRangeBuf_XPosValue);
  SetTextDisplayRange(235, 126, 12 * 3, 24, &YPosTextShape);
  ReadTextDisplayRangeInfo(YPosTextShape, TextRangeBuf_YPosValue);
  SetTextDisplayRange(235, 211, 12 * 3, 24, &ZPosTextShape);
  ReadTextDisplayRangeInfo(ZPosTextShape, TextRangeBuf_ZPosValue);
  DisplayXYZPosText();
}

uint8_t ScanClickKeyValue(void)
{
  if (IstouchxyDown(364, 0, 480, 82))
  {
    return XPosDown;
  }
  else if (IstouchxyDown(250, 0, 364, 82))
  {
    return XPosUp;
  }
  else if (IstouchxyDown(364, 82, 480, 179))
  {
    return YPosDown;
  }
  else if (IstouchxyDown(250, 82, 364, 179))
  {
    return YPosUp;
  }
  else if (IstouchxyDown(364, 179, 480, 260))
  {
    return ZPosDown;
  }
  else if (IstouchxyDown(250, 179, 364, 260))
  {
    return ZPosUp;
  }
  else
  {
    return NoClick;
  }
}

void LongClickPress(void)
{
  if (IstouchxyUp())
  {
    ClickKeyValue = 0;
    (void)OS_DELAY(200);
  }
  else if (ClickTimeout < xTaskGetTickCount())
  {
    ClickTimeout = ClickTimeoutms + xTaskGetTickCount();
    SlowUpdateXYZPos = 0;
    FastUpdateXYZPos = 1;
  }
}

void ShortClickPress(void)
{
  SlowUpdateXYZPos = 1;
  FastUpdateXYZPos = 0;
}

void UpdateXYZPos(void)
{
  switch (ClickKeyValue)
  {
  case XPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case XPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  case YPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case YPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  case ZPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case ZPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  default:
    break;
  }
}

void XYZPosRangeLimit(void)
{
//  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
//  {
//    if (t_gui.move_xyz_pos[X_AXIS] > flash_param_t.idex_extruder_0_bed_offset[1])
//      t_gui.move_xyz_pos[X_AXIS] = flash_param_t.idex_extruder_0_bed_offset[1];
//  }
//  else
//  {
//    if (t_gui.move_xyz_pos[X_AXIS] > ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS])
//      t_gui.move_xyz_pos[X_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS];
//  }

  if (t_gui.move_xyz_pos[X_AXIS] > ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS])
    t_gui.move_xyz_pos[X_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS];

  if (t_gui.move_xyz_pos[X_AXIS] < 0)
    t_gui.move_xyz_pos[X_AXIS] = 0;

  if (t_gui.move_xyz_pos[Y_AXIS] > ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS])
    t_gui.move_xyz_pos[Y_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS];

  if (t_gui.move_xyz_pos[Y_AXIS] < 0)
    t_gui.move_xyz_pos[Y_AXIS] = 0;

  if (t_gui.move_xyz_pos[Z_AXIS] > ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS])
    t_gui.move_xyz_pos[Z_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS];

  if (t_gui.move_xyz_pos[Z_AXIS] < 0)
    t_gui.move_xyz_pos[Z_AXIS] = 0;
}

void SetXYZPosValue(void)
{
  if (NoClick == ClickKeyValue)
  {
    ClickTimeout = ClickTimeoutms + xTaskGetTickCount();
    ClickFirstTimeout = ClickFirstTimeoutms + xTaskGetTickCount();
    ClickKeyValue = ScanClickKeyValue();
  }
  else
  {
    if (ClickFirstTimeout < xTaskGetTickCount())
    {
      LongClickPress();
    }
    else if (IstouchxyUp())
    {
      ShortClickPress();
    }
  }

  if ((1 == FastUpdateXYZPos) || (1 == SlowUpdateXYZPos))
  {
    UpdateXYZPos();
    FastUpdateXYZPos = 0;
    SlowUpdateXYZPos = 0;
    XYZPosRangeLimit();
    DisplayXYZPosText();
  }
}

void MoveXYZCancelKey(void)
{
  gui::set_current_display(prepareF);
}


void MoveXYZ(void)
{
  if (gui::is_refresh())
  {
    RefreshMoveXYZInterface();
    return;
  }

  if (lcddev.height == 272)
  {
    if (touchxy(110, 228, 240, 272))
    {
      waiting_for_xyz_moving();
      return ;
    }

    if (touchxy(240, 228, 380, 272))
    {
      MoveXYZCancelKey();
      return ;
    }
  }
  else
  {
    if (touchxy(110, 255, 240, 320))
    {
      waiting_for_xyz_moving();
      return ;
    }

    if (touchxy(240, 265, 380, 320))
    {
      MoveXYZCancelKey();
      return ;
    }
  }

  SetXYZPosValue();
}

#ifdef __cplusplus
} //extern "C" {
#endif


