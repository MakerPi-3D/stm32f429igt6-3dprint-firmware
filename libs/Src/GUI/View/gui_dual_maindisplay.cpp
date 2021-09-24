#include "machinecustom.h"
#include "commonf.h"              //包含界面函数
#include "common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_common.h"
#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct _textrange  NozzleTempTextRange;    //喷嘴温度的显示区域
extern struct _textrange  NozzleTempTextRange1;    //喷嘴温度的显示区域
extern struct _textrange  HotBedTempTextRange;    //热床温度的显示区域
extern struct _textrange  NozzleTargetTempTextRange;   //喷嘴目标温度的显示区域
extern struct _textrange  NozzleTargetTempTextRange1;   //喷嘴目标温度的显示区域
extern struct _textrange  HotBedTargetTempTextRange;   //热床目标温度的显示区域



#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_HotBedTargetTemp ccm_param::TextRangeBuf_24_12_3_3
#define TextRangeBuf_NozzleTargetTemp ccm_param::TextRangeBuf_24_12_3_4
#define TextRangeBuf_HotBedTemp ccm_param::TextRangeBuf_24_12_3_5
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_6
#define TextRangeBuf_NozzleTemp1 ccm_param::TextRangeBuf_24_12_3_7
#define TextRangeBuf_NozzleTargetTemp1 ccm_param::TextRangeBuf_24_12_3_8

// 4.3寸屏幕触摸区域
#define L_43_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_43_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_43_PREPARE_TOUCH_AREA (126, 92, 202, 168)
#define L_43_SETTING_TOUCH_AREA (84, 174, 160, 250)
#define L_43_STATUS_TOUCH_AREA (174, 174, 260, 260)
#define L_43_PRINT_TOUCH_AREA (300, 96, 400, 256)
// 3.5寸屏幕触摸区域
#define L_35_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_35_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_35_PREPARE_TOUCH_AREA (126, 92, 202, 168)
#define L_35_SETTING_TOUCH_AREA (84, 174, 160, 250)
#define L_35_STATUS_TOUCH_AREA (174, 174, 260, 260)
#define L_35_PRINT_TOUCH_AREA (300, 96, 400, 256)
// 7.0寸屏幕触摸区域
#define L_70_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_70_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_70_PREPARE_TOUCH_AREA (126, 92, 202, 168)
#define L_70_SETTING_TOUCH_AREA (84, 174, 160, 250)
#define L_70_STATUS_TOUCH_AREA (174, 174, 260, 260)
#define L_70_PRINT_TOUCH_AREA (300, 96, 400, 256)

static int main_touch_check_dual(void)
{
  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_NOZZLE_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_NOZZLE_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_NOZZLE_TOUCH_AREA))
     )
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(PrintSet_NotM14_Left_dual);
    }
    else
    {
      gui::set_current_display(PrintSet_NotM14_Left);
    }

    return 1;
  }

  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_BED_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_BED_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_BED_TOUCH_AREA))
     )
  {
    gui::set_current_display(PrintSet_NotM14_Right);
    return 1;
  }

  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_PREPARE_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_PREPARE_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_PREPARE_TOUCH_AREA))
     )
  {
    gui::set_current_display(prepareF);
    return 1;
  }

  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_SETTING_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_SETTING_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_SETTING_TOUCH_AREA))
     )
  {
    gui::set_current_display(settingF);
    return 1;
  }

  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_STATUS_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_STATUS_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_STATUS_TOUCH_AREA))
     )
  {
    gui::set_current_display(statusF);
    return 1;
  }

  if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_PRINT_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_PRINT_TOUCH_AREA)) ||
      (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_PRINT_TOUCH_AREA))
     )
  {
    user_usb_host_update_status();
    OS_DELAY(200);

    if (user_usb_host_is_mount())
    {
      respond_gui_send_sem(OpenSDCardValue);
      gui::set_current_display(filescanF);
      return 1;
    }
    else
    {
      gui::set_current_display(NoUdiskF);
      return 1;
    }
  }

 if (t_power_off.flag == 1) //断电续打, 注意这个标志位是从flash中读出再赋值过来的，flash中数据默认是0xFF，所以写判断语句要写“== 1”
  {
    t_power_off.flag = 0;
    strcpy(printname, t_power_off.file_name);
    gui::set_current_display(PowerOffRecover);
    return 1;
  }

  return 0;
}


static void main_display_text_dual(void)
{
  char TextBuffer[20];
  //显示喷嘴1温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", NozzleTempTextRange.x + 12 * 3, NozzleTempTextRange.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //显示喷嘴2温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", NozzleTempTextRange1.x + 12 * 3, NozzleTempTextRange1.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
  //显示热床温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.hot_bed_temp);
  CopyTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, HotBedTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", HotBedTempTextRange.x + 12 * 3, HotBedTempTextRange.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_hot_bed_temp);
  CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
}

int main_display_dual(void)
{
  if (gui::is_refresh())
  {
    display_picture(151);

    if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
    {
      SetTextDisplayRangeNormal(152, 18, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(152, 52, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(330, 18, 12 * 3, 24, &HotBedTempTextRange);
    }
    else if (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE)
    {
      SetTextDisplayRangeNormal(152, 18, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(152, 52, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(330, 18, 12 * 3, 24, &HotBedTempTextRange);
    }
    else if (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE)
    {
      SetTextDisplayRangeNormal(350, 55, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(350, 130, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(720, 50, 12 * 3, 24, &HotBedTempTextRange);
    }

    SetTextDisplayRangeNormal(NozzleTempTextRange.x + 12 * 4, NozzleTempTextRange.y, 12 * 3, 24, &NozzleTargetTempTextRange);
    SetTextDisplayRangeNormal(NozzleTempTextRange1.x + 12 * 4, NozzleTempTextRange1.y, 12 * 3, 24, &NozzleTargetTempTextRange1);
    SetTextDisplayRangeNormal(HotBedTempTextRange.x + 12 * 4, HotBedTempTextRange.y, 12 * 3, 24, &HotBedTargetTempTextRange);
    ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp);
    ReadTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1);
    ReadTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp);
    ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp);
    ReadTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1);
    ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp);
    main_display_text_dual();
  }

  if (main_touch_check_dual())
    return 1;

  if (gui::is_refresh_rtc())
  {
    main_display_text_dual();
  }

  return 1;
}

#ifdef __cplusplus
} //extern "C" {
#endif

