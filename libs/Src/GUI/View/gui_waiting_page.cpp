#include "common.h"
#include "commonf.h"
#include "sysconfig_data.h"
#include "interface.h"
#include "globalvariables.h"
#ifdef __cplusplus
extern "C" {
#endif

#define LCD_HOMING_STR_CN "¹éÁãÖÐ£¬ÇëµÈ´ý..."
#define LCD_HOMING_STR_EN "Homing, please wait..."
#define LCD_HOMING_STR_TC "¹éÁãÖÐ£¬ÇëµÈ´ý..."

#define LCD_XYZ_MOVING_STR_CN "ÒÆ¶¯ÖÐ£¬ÇëµÈ´ý..."
#define LCD_XYZ_MOVING_STR_EN "Moving, please wait..."
#define LCD_XYZ_MOVING_STR_TC "ÒÆ¶¯ÖÐ£¬ÇëµÈ´ý..."

#define LCD_PAUSING_STR_CN "ÔÝÍ£ÖÐ£¬ÇëµÈ´ý..."
#define LCD_PAUSING_STR_EN "Pausing, please wait..."
#define LCD_PAUSING_STR_TC "ÔÝÍ£ÖÐ£¬ÇëµÈ´ý..."

#define LCD_RESUMING_STR_CN "»Ö¸´ÖÐ£¬ÇëµÈ´ý..."
#define LCD_RESUMING_STR_EN "Resuming, please wait..."
#define LCD_RESUMING_STR_TC "»Ö¸´ÖÐ£¬ÇëµÈ´ý..."

#define LCD_STOPPING_STR_CN "Í£Ö¹ÖÐ£¬ÇëµÈ´ý..."
#define LCD_STOPPING_STR_EN "Stopping, please wait..."
#define LCD_STOPPING_STR_TC "Í£Ö¹ÖÐ£¬ÇëµÈ´ý..."

enum GUI_WAITING_STATUS
{
  GUI_WAITING_HOMING = 0,
  GUI_WAITING_XYZ_MOVING,
  GUI_WAITING_PAUSING,
  GUI_WAITING_RESUMING,
  GUI_WAITING_STOPPING
};

volatile GUI_WAITING_STATUS gui_waiting_status;
volatile uint8_t is_gui_waiting_done = false;

void gui_waiting_page(void)
{
  char TextBuffer[96];
  int x, y, font_size;

  if (gui::is_refresh())
  {
    display_picture(164);

    if (t_sys_data_current.ui_number == 0) // 3.5
    {
      x = 120;
      y = 166;
      font_size = 24;
    }
    else if (t_sys_data_current.ui_number == 1 || t_sys_data_current.ui_number == 1)  // 4.3
    {
      x = 120;
      y = 142;
      font_size = 24;

      if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id)
      {
        x = 80;
      }
    }
    else
    {
      x = 120;
      y = 166;
      font_size = 24;
    }

    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id)
    {
      if (gui_waiting_status == GUI_WAITING_HOMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_HOMING_STR_CN);
      }
      else if (gui_waiting_status == GUI_WAITING_XYZ_MOVING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_XYZ_MOVING_STR_CN);
      }
      else if (gui_waiting_status == GUI_WAITING_PAUSING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_PAUSING_STR_CN);
      }
      else if (gui_waiting_status == GUI_WAITING_RESUMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_RESUMING_STR_CN);
      }
      else if (gui_waiting_status == GUI_WAITING_STOPPING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_STOPPING_STR_CN);
      }
    }
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id)
    {
      if (gui_waiting_status == GUI_WAITING_HOMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_HOMING_STR_EN);
      }
      else if (gui_waiting_status == GUI_WAITING_XYZ_MOVING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_XYZ_MOVING_STR_EN);
      }
      else if (gui_waiting_status == GUI_WAITING_PAUSING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_PAUSING_STR_EN);
      }
      else if (gui_waiting_status == GUI_WAITING_RESUMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_RESUMING_STR_EN);
      }
      else if (gui_waiting_status == GUI_WAITING_STOPPING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_STOPPING_STR_EN);
      }
    }
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id)
    {
      if (gui_waiting_status == GUI_WAITING_HOMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_HOMING_STR_TC);
      }
      else if (gui_waiting_status == GUI_WAITING_XYZ_MOVING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_XYZ_MOVING_STR_TC);
      }
      else if (gui_waiting_status == GUI_WAITING_PAUSING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_PAUSING_STR_TC);
      }
      else if (gui_waiting_status == GUI_WAITING_RESUMING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_RESUMING_STR_TC);
      }
      else if (gui_waiting_status == GUI_WAITING_STOPPING)
      {
        snprintf(TextBuffer, sizeof(TextBuffer), "%s", LCD_STOPPING_STR_TC);
      }
    }

    DisplayText((uint8_t *)TextBuffer, x, y, font_size, (u16)YELLOW);
  }

  if (is_gui_waiting_done)
  {
    if (gui_waiting_status == GUI_WAITING_HOMING)
    {
      gui::set_current_display(prepareF);
    }
    else if (gui_waiting_status == GUI_WAITING_XYZ_MOVING)
    {
      gui::set_current_display(prepareF);
    }
    else if (gui_waiting_status == GUI_WAITING_PAUSING)
    {
      osDelay(50);

      if (IsNotHaveMatInPrint == 1)
      {
        gui::set_current_display(NoHaveMatWaringInterface);
      }
      else
      {
        gui::set_current_display(maindisplayF);
      }
    }
    else if (gui_waiting_status == GUI_WAITING_RESUMING)
    {
      osDelay(50);
      gui::set_current_display(maindisplayF);
    }
    else if (gui_waiting_status == GUI_WAITING_STOPPING)
    {
      osDelay(50);
      gui::set_current_display(maindisplayF);
    }
  }
}

void waiting_for_stopping(void)
{
  is_gui_waiting_done = false;
  gui_waiting_status = GUI_WAITING_STOPPING;
  gui::set_current_display(gui_waiting_page);
  respond_gui_send_sem(StopPrintValue);
}

void waiting_for_resuming(void)
{
  is_gui_waiting_done = false;
  gui_waiting_status = GUI_WAITING_RESUMING;
  gui::set_current_display(gui_waiting_page);
  respond_gui_send_sem(ResumePrintValue);
}

void waiting_for_pausing(void)
{
  is_gui_waiting_done = false;
  gui_waiting_status = GUI_WAITING_PAUSING;
  gui::set_current_display(gui_waiting_page);
  respond_gui_send_sem(PausePrintValue);
}

void waiting_for_xyz_moving(void)
{
  is_gui_waiting_done = false;
  gui_waiting_status = GUI_WAITING_XYZ_MOVING;
  gui::set_current_display(gui_waiting_page);
  respond_gui_send_sem(MoveXYZValue);
}
#ifdef __cplusplus
} //extern "C" {
#endif

