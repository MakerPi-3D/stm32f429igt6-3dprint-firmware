#include "machinecustom.h"
#include "commonf.h"              //�������溯��
#include "common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_common.h"
#include "user_ccm.h"
#include "flashconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct _textrange  NozzleTempTextRange;    //�����¶ȵ���ʾ����
extern struct _textrange  NozzleTempTextRange1;    //�����¶ȵ���ʾ����
extern struct _textrange  HotBedTempTextRange;    //�ȴ��¶ȵ���ʾ����
extern struct _textrange  NozzleTargetTempTextRange;   //����Ŀ���¶ȵ���ʾ����
extern struct _textrange  NozzleTargetTempTextRange1;   //����Ŀ���¶ȵ���ʾ����
extern struct _textrange  HotBedTargetTempTextRange;   //�ȴ�Ŀ���¶ȵ���ʾ����
extern struct _textrange  PrintScheduleTextRange;    //��ӡ���ȵ���ʾ����
extern struct _textrange  PrintTimeTextRange;    //��ӡʱ�����ʾ����

extern int CheckIsPrintFinish(void);
extern int CheckIsHaveUdisk(void);

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_Time ccm_param::TextRangeBuf_24_12_9_2
#define TextRangeBuf_PrintSchedule ccm_param::TextRangeBuf_24_12_3_2
#define TextRangeBuf_HotBedTargetTemp ccm_param::TextRangeBuf_24_12_3_3
#define TextRangeBuf_NozzleTargetTemp ccm_param::TextRangeBuf_24_12_3_4
#define TextRangeBuf_HotBedTemp ccm_param::TextRangeBuf_24_12_3_5
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_6
#define TextRangeBuf_NozzleTemp1 ccm_param::TextRangeBuf_24_12_3_7
#define TextRangeBuf_NozzleTargetTemp1 ccm_param::TextRangeBuf_24_12_3_8
#define PauseInterface 1
#define ResumeInterface 2

// 4.3����Ļ��������
#define L_43_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_43_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_43_FILAMENT_PAUSE_RESUME_AREA (76, 180, 152, 256)
#define L_43_FILAMENT_CHANGE_AREA (200, 180, 276, 256)
#define L_43_FILAMENT_STOP_AREA (320, 180, 396, 256)
#define L_43_PAUSE_RESUME_AREA (112, 178, 188, 254)
#define L_43_STOP_AREA (278, 178, 354, 254)
// 3.5����Ļ��������
#define L_35_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_35_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_35_FILAMENT_PAUSE_RESUME_AREA (240, 0, 408, 66)
#define L_35_FILAMENT_CHANGE_AREA (240, 0, 408, 66)
#define L_35_FILAMENT_STOP_AREA (240, 0, 408, 66)
#define L_35_PAUSE_RESUME_AREA (240, 0, 408, 66)
#define L_35_STOP_AREA (240, 0, 408, 66)
// 7.0����Ļ��������
#define L_70_NOZZLE_TOUCH_AREA (72, 0, 240, 102)
#define L_70_BED_TOUCH_AREA (240, 0, 408, 66)
#define L_70_FILAMENT_PAUSE_RESUME_AREA (240, 0, 408, 66)
#define L_70_FILAMENT_CHANGE_AREA (240, 0, 408, 66)
#define L_70_FILAMENT_STOP_AREA (240, 0, 408, 66)
#define L_70_PAUSE_RESUME_AREA (240, 0, 408, 66)
#define L_70_STOP_AREA (240, 0, 408, 66)

static bool p_have_chg_filament = false;
static int p_print_status = PauseInterface;

static int print_touch_check_have_filament(void)
{
  if (p_print_status == PauseInterface)
  {
    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_PAUSE_RESUME_AREA))
       )
    {
      gui::set_current_display(PausePrintF);               //���ý���ˢ��
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_CHANGE_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_CHANGE_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_CHANGE_AREA))
       )
    {
      gui::set_current_display(ChangeFilamentConfirm);
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_STOP_AREA))
       )
    {
      gui::set_current_display(stopprintF);
      return 1;
    }
  }
  else if (p_print_status == ResumeInterface)
  {
    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_PAUSE_RESUME_AREA))
       )
    {
      gui::set_current_display(ResumePrintF);               //���ý���ˢ��
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_CHANGE_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_CHANGE_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_CHANGE_AREA))
       )
    {
      gui::set_current_display(ChangeFilamentConfirm);
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_FILAMENT_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_FILAMENT_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_FILAMENT_STOP_AREA))
       )
    {
      gui::set_current_display(stopprintF);
      return 1;
    }
  }

  return 0;
}

static int print_touch_check_not_filament(void)
{
  if (p_print_status == PauseInterface)
  {
    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_PAUSE_RESUME_AREA))
       )
    {
      gui::set_current_display(PausePrintF);               //���ý���ˢ��
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_STOP_AREA))
       )
    {
      gui::set_current_display(stopprintF);
      return 1;
    }
  }
  else if (p_print_status == ResumeInterface)
  {
    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_PAUSE_RESUME_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_PAUSE_RESUME_AREA))
       )
    {
      gui::set_current_display(ResumePrintF);               //���ý���ˢ��
      return 1;
    }

    if ((t_sys.lcd_type == LCD_TYPE_43_480272_SIZE && (touchxy L_43_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE && (touchxy L_35_STOP_AREA)) ||
        (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE && (touchxy L_70_STOP_AREA))
       )
    {
      gui::set_current_display(stopprintF);
      return 1;
    }
  }

  return 0;
}

static int print_touch_check_dual(void)
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

  if (p_have_chg_filament)
  {
    if (print_touch_check_have_filament())
    {
      return 1;
    }
  }
  else
  {
    if (print_touch_check_not_filament())
    {
      return 1;
    }
  }

  return 0;
}


static void print_display_text_dual(void)
{
  char TextBuffer[20];
  int second, minute, hour;
  //��ʾ����1�¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", NozzleTempTextRange.x + 12 * 3, NozzleTempTextRange.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾ����2�¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", NozzleTempTextRange1.x + 12 * 3, NozzleTempTextRange1.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[1]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾ�ȴ��¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.hot_bed_temp);
  CopyTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, HotBedTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  DisplayTextNormal((unsigned char *)"/", HotBedTempTextRange.x + 12 * 3, HotBedTempTextRange.y, 24, (u16)testcolor);
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_hot_bed_temp);
  CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾ��ӡ����
  snprintf(TextBuffer, sizeof(TextBuffer), "%2d%%", (int)t_gui.print_percent);
  CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾ��ӡʱ��
  second = t_gui.printed_time_sec;
  hour = second / 3600;
  minute = (second - hour * 3600) / 60;
  second = (second - hour * 3600) % 60;
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d:%2d:%2d", hour, minute, second);
  CopyTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, PrintTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
}

int print_display_dual(int status, bool have_chg_filament)
{
  if (gui::is_refresh())
  {
    p_print_status = status;
    p_have_chg_filament = have_chg_filament;

    if (PauseInterface == status)
      display_picture(have_chg_filament ? 152 : 156);
    else if (ResumeInterface == status)
      display_picture(have_chg_filament ? 153 : 157);

    if (t_sys.lcd_type == LCD_TYPE_43_480272_SIZE)
    {
      SetTextDisplayRangeNormal(146, 30, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(146, 64, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(318, 30, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRangeNormal(146, 96, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRangeNormal(318, 96, 12 * 9, 24, &PrintTimeTextRange);
    }
    else if (t_sys.lcd_type == LCD_TYPE_35_480320_SIZE)
    {
      SetTextDisplayRangeNormal(152, 18, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(152, 52, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(330, 18, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRangeNormal(230, 147, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRangeNormal(324, 84, 12 * 9, 24, &PrintTimeTextRange);
    }
    else if (t_sys.lcd_type == LCD_TYPE_7_1024600_SIZE)
    {
      SetTextDisplayRangeNormal(152, 18, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRangeNormal(152, 52, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRangeNormal(330, 18, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRangeNormal(230, 147, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRangeNormal(324, 84, 12 * 9, 24, &PrintTimeTextRange);
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
    ReadTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule);
    ReadTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time);
    print_display_text_dual();
  }

  if (print_touch_check_dual())
    return 1;

  if (gui::is_refresh_rtc())
  {
    print_display_text_dual();

    if (CheckIsPrintFinish())
      return 1;

    if (status == PauseInterface)
    {
      if (!have_chg_filament && CheckIsHaveUdisk())
        return 1;
    }
    else if (status == ResumeInterface)
    {
      if (CheckIsHaveUdisk())
        return 1;
    }
  }

  return 1;
}

#ifdef __cplusplus
} //extern "C" {
#endif

