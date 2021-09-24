#include "commonf.h"
#include "common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct _textrange  NozzleTempTextRange;
extern struct _textrange  NozzleTempTextRange1;
#define TRB_NOZZLE_TEMP  ccm_param::TextRangeBuf_24_12_3_0
#define TRB_NOZZLE_TEMP1 ccm_param::TextRangeBuf_24_12_3_1
#define TRB_STRING       ccm_param::TextRangeBuf_24_12_9_0

void loadfilament_display_text_dual(void)
{
  char buffer[20];
  //显示喷嘴1温度
  snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
  DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
  snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[0]);
  DisplayTextNormal((uint8_t *)buffer, NozzleTempTextRange.x + 12 * 3, NozzleTempTextRange.y, 24, (u16)testcolor);
  //显示喷嘴2温度
  snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1, TRB_STRING);
  DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange1, TRB_STRING, 24, (u16)testcolor);
  snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[1]);
  DisplayTextNormal((uint8_t *)buffer, NozzleTempTextRange1.x + 12 * 3, NozzleTempTextRange1.y, 24, (u16)testcolor);
}

void loadfilament0F_dual(void)
{
  if (gui::is_refresh())
  {
    display_picture(154);

    if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
    {
      SetTextDisplayRangeNormal(156, 40, 12 * 3, 24, &NozzleTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
      SetTextDisplayRangeNormal(324, 40, 12 * 3, 24, &NozzleTempTextRange1);
      ReadTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1);
    }
    else
    {
      SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
      SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange1);
      ReadTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1);
    }

    loadfilament_display_text_dual();
  }

  if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
  {
    if (touchxy(194, 194, 288, 240))
    {
      respond_gui_send_sem(StopFeedFilamentValue);
      gui::set_current_display(prepareF);
      return ;
    }

    if (touchxy(90, 0, 392, 86))
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        gui::set_current_display(SetLoadFilamentNozzleTemp_dual);
      }
      else
      {
        gui::set_current_display(SetLoadFilamentNozzleTemp);
      }

      return ;
    }
  }
  else
  {
    if (touchxy(160, 210, 310, 300))
    {
      respond_gui_send_sem(StopFeedFilamentValue);
      gui::set_current_display(prepareF);
      return ;
    }

    if (touchxy(165, 35, 310, 85))
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        gui::set_current_display(SetLoadFilamentNozzleTemp_dual);
      }
      else
      {
        gui::set_current_display(SetLoadFilamentNozzleTemp);
      }

      return ;
    }
  }

  if (gui::is_refresh_rtc())
  {
    if (IsFinishedFilamentHeat)
    {
      gui::set_current_display(loadfilament1F);
      return;
    }

    loadfilament_display_text_dual();
  }
}

void loadfilament0F(void)
{
  char buffer[20];

  if (gui::is_refresh())
  {
    display_picture(8);
    SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
    ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
    //显示喷嘴温度
    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
    //显示喷嘴目标温度
    snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[0]);
    DisplayText((uint8_t *)buffer, 244 + 12 * 3, 49, 24, (u16)testcolor);
  }

  if (touchxy(160, 210, 310, 300))
  {
    respond_gui_send_sem(StopFeedFilamentValue);
    gui::set_current_display(prepareF);
    return ;
  }

  if (touchxy(165, 35, 310, 85))
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(SetLoadFilamentNozzleTemp_dual);
    }
    else
    {
      gui::set_current_display(SetLoadFilamentNozzleTemp);
    }

    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (IsFinishedFilamentHeat)
    {
      gui::set_current_display(loadfilament1F);
      return;
    }

    //显示喷嘴温度
    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
    return;
  }
}

void loadfilament1F(void)
{
  if (gui::is_refresh())
  {
    display_picture(9);
  }

  if (touchxy(160, 210, 310, 300))
  {
    respond_gui_send_sem(StopFeedFilamentValue);
    gui::set_current_display(prepareF);
    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (IsSuccessFilament)
    {
      gui::set_current_display(loadfilament2F);
      return;
    }

    return;
  }
}

void loadfilament2F(void)
{
  if (gui::is_refresh())
  {
    display_picture(10);
  }

  if (touchxy(160, 200, 320, 300))
  {
    gui::set_current_display(prepareF);
    return ;
  }
}

void unloadfilament_display_text_dual(void)
{
  char buffer[20];
  //显示喷嘴1温度
  snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
  DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
  snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[0]);
  DisplayTextNormal((uint8_t *)buffer, NozzleTempTextRange.x + 12 * 3, NozzleTempTextRange.y, 24, (u16)testcolor);
  //显示喷嘴2温度
  snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1, TRB_STRING);
  DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange1, TRB_STRING, 24, (u16)testcolor);
  snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[1]);
  DisplayTextNormal((uint8_t *)buffer, NozzleTempTextRange1.x + 12 * 3, NozzleTempTextRange1.y, 24, (u16)testcolor);
}

void unloadfilament0F_dual(void)
{
  if (gui::is_refresh())
  {
    display_picture(154);

    if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
    {
      SetTextDisplayRangeNormal(156, 40, 12 * 3, 24, &NozzleTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
      SetTextDisplayRangeNormal(324, 40, 12 * 3, 24, &NozzleTempTextRange1);
      ReadTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1);
    }
    else
    {
      SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
      SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange1);
      ReadTextDisplayRangeInfo(NozzleTempTextRange1, TRB_NOZZLE_TEMP1);
    }

    unloadfilament_display_text_dual();
  }

  if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
  {
    if (touchxy(194, 194, 288, 240))
    {
      respond_gui_send_sem(StopBackFilamentValue);
      gui::set_current_display(prepareF);
      return ;
    }

    if (touchxy(90, 0, 392, 86))
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        gui::set_current_display(SetUnLoadFilamentNozzleTemp_dual);
      }
      else
      {
        gui::set_current_display(SetUnLoadFilamentNozzleTemp);
      }

      return ;
    }
  }
  else
  {
    if (touchxy(160, 210, 310, 300))
    {
      respond_gui_send_sem(StopBackFilamentValue);
      gui::set_current_display(prepareF);
      return ;
    }

    if (touchxy(165, 35, 310, 85))
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        gui::set_current_display(SetUnLoadFilamentNozzleTemp_dual);
      }
      else
      {
        gui::set_current_display(SetUnLoadFilamentNozzleTemp);
      }

      return ;
    }
  }

  if (gui::is_refresh_rtc())
  {
    if (IsFinishedFilamentHeat)
    {
      gui::set_current_display(unloadfilament1F);
      return;
    }

    unloadfilament_display_text_dual();
  }
}


void unloadfilament0F(void)
{
  char buffer[20];

  if (gui::is_refresh())
  {
    display_picture(8);
    SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
    ReadTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP);
    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
    snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[0]);
    DisplayText((uint8_t *)buffer, 244 + 12 * 3, 49, 24, (u16)testcolor);
  }

  if (touchxy(160, 210, 310, 300))
  {
    respond_gui_send_sem(StopBackFilamentValue);
    gui::set_current_display(prepareF);
    return ;
  }

  if (touchxy(165, 35, 310, 85))
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(SetUnLoadFilamentNozzleTemp_dual);
    }
    else
    {
      gui::set_current_display(SetUnLoadFilamentNozzleTemp);
    }

    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (IsFinishedFilamentHeat)
    {
      gui::set_current_display(unloadfilament1F);
      return;
    }

    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TRB_NOZZLE_TEMP, TRB_STRING);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TRB_STRING, 24, (u16)testcolor);
    return;
  }
}

void unloadfilament1F(void)
{
  if (gui::is_refresh())
  {
    display_picture(11);
  }

  if (touchxy(160, 210, 310, 300))
  {
    respond_gui_send_sem(StopBackFilamentValue);
    gui::set_current_display(prepareF);
    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (IsSuccessFilament)
    {
      gui::set_current_display(unloadfilament2F);
      return;
    }

    return;
  }
}

void unloadfilament2F(void)
{
  if (gui::is_refresh())
  {
    display_picture(12);
  }

  if (touchxy(160, 200, 320, 300))
  {
    gui::set_current_display(prepareF);
    return ;
  }
}


#ifdef __cplusplus
} //extern "C" {
#endif






