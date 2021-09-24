#include "machinecustom.h"
#include "commonf.h"              //�������溯��
#include "common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
//#include "ConfigurationStore.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_common.h"
#include "user_ccm.h"
#include "gcode.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _textrange  NozzleTempTextRange;    //�����¶ȵ���ʾ����
struct _textrange  NozzleTempTextRange1;    //�����¶ȵ���ʾ����
struct _textrange  HotBedTempTextRange;    //�ȴ��¶ȵ���ʾ����
struct _textrange  NozzleTargetTempTextRange;   //����Ŀ���¶ȵ���ʾ����
struct _textrange  NozzleTargetTempTextRange1;   //����Ŀ���¶ȵ���ʾ����
struct _textrange  HotBedTargetTempTextRange;   //�ȴ�Ŀ���¶ȵ���ʾ����
struct _textrange  PrintScheduleTextRange;    //��ӡ���ȵ���ʾ����
struct _textrange  PrintTimeTextRange;    //��ӡʱ�����ʾ����
struct _textrange  SpeedRange;    //�ٶȵ���ʾ����
struct _textrange  CavityTempTextRange;    //ǻ���¶ȵ���ʾ����
struct _textrange  CavityTargetTempTextRange;   //ǻ��Ŀ���¶ȵ���ʾ����
struct _textrange  RunTimeTextRange; // ����ʱ�����ʾ����
struct _textrange  PrintSpeedTextSharp; //��ӡ�ٶȵ���ʾ����
struct _textrange  FanSpeedTextSharp; //�����ٶȵ���ʾ����
struct _textrange  LedSwitchTextSharp;
struct _textrange  CavityTempOnTextSharp; //ǻ���¶ȿ��ص���ʾ����
struct _textrange  ZOffsetZeroTextSharp; //Z���ƫ�Ƶ���ʾ����
struct _textrange  XPosTextShape; //Xλ�õ���ʾ����
struct _textrange  YPosTextShape; //Yλ�õ���ʾ����
struct _textrange  ZPosTextShape; //Zλ�õ���ʾ����

#define TextRangeBuf_CavityTargetTemp ccm_param::TextRangeBuf_24_12_3_0
#define TextRangeBuf_Speed ccm_param::TextRangeBuf_24_12_9_0
#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_Time ccm_param::TextRangeBuf_24_12_9_2
#define TextRangeBuf_CavityTemp ccm_param::TextRangeBuf_24_12_3_1
#define TextRangeBuf_PrintSchedule ccm_param::TextRangeBuf_24_12_3_2
#define TextRangeBuf_HotBedTargetTemp ccm_param::TextRangeBuf_24_12_3_3
#define TextRangeBuf_NozzleTargetTemp ccm_param::TextRangeBuf_24_12_3_4
#define TextRangeBuf_HotBedTemp ccm_param::TextRangeBuf_24_12_3_5
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_6
#define TextRangeBuf_NozzleTemp1 ccm_param::TextRangeBuf_24_12_3_7
#define TextRangeBuf_NozzleTargetTemp1 ccm_param::TextRangeBuf_24_12_3_8

extern bool IsFinishedPrint(void);
extern bool IsPrintSDFile(void);

extern int main_display_dual(void);
extern int print_display_dual(int status, bool have_chg_filament);

//��ת����ҳ��
void goto_page_homing(void)
{
  if (gui::is_refresh())
  {
    if (PICTURE_IS_JAPANESE != t_sys_data_current.pic_id) //����û����ҳ�棬�����������
    {
      if (!gcode::g28_complete_flag)
      {
        gui::set_current_display(page_homing); //���ý���ˢ��
      }
    }
  }
}

//����ҳ��
void page_homing(void)
{
  if (gui::is_refresh())
  {
    display_picture(83);
  }

  if (gui::is_refresh_rtc())
  {
    if (gcode::g28_complete_flag)
    {
      gui::set_current_display(prepareF);
    }
  }
}


int CheckIsPrintFinish(void)
{
  //���¸���ʵʱ�ź���ת����
  if (IsFinishedPrint()) //��ӡ��ɱ�־
  {
    print_flage = 0;
    gui::set_current_display(printfinishF);                  //���ý���ˢ��
    return 1;
  }

  return 0;
}

int CheckIsHaveUdisk(void)
{
  //��U�̷���������
  if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
  {
    //U�̰γ���־���ļ��ϴ���ɺ��ӡ�ı�־
    waiting_for_stopping();
    return 1;
  }

  return 0;
}

void Custom_PrintInterfaceDisplayInit(void)
{
  char print_percent[5];
  SetTextDisplayRange(132, 36, 12 * 3, 24, &NozzleTempTextRange); //������ʾ����
  SetTextDisplayRange(324, 36, 12 * 3, 24, &HotBedTempTextRange);
  SetTextDisplayRange(230, 147, 12 * 3, 24, &PrintScheduleTextRange);
  SetTextDisplayRange(324, 84, 12 * 9, 24, &PrintTimeTextRange);
  SetTextDisplayRange(132, 84, 12 * 3, 24, &CavityTempTextRange); //������ʾ����
  SetTextDisplayRange(132 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &NozzleTargetTempTextRange);
  SetTextDisplayRange(324 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &HotBedTargetTempTextRange);
  SetTextDisplayRange(132 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 84, 12 * 3, 24, &CavityTargetTempTextRange);
  //���´�ӡ����
  snprintf(print_percent, sizeof(print_percent), "%2d%%", (int)t_gui.print_percent);

  if (Draw_progressBar_new(t_gui.printfile_size, t_gui.file_size - (uint32_t)((t_gui.printfile_size + 99) * 0.01), 92, 148, 344, 20))
  {
    ReadTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule);
    CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)print_percent, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  }

  ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp); //��lcd��ȡ���أ����浽����
  ReadTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp);
  ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp);
  //  ReadTextDisplayRangeInfo(PrintScheduleTextRange,TextRangeBuf_PrintSchedule);
  ReadTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time);
  ReadTextDisplayRangeInfo(CavityTempTextRange, TextRangeBuf_CavityTemp); //��lcd��ȡ���أ����浽����
  ReadTextDisplayRangeInfo(CavityTargetTempTextRange, TextRangeBuf_CavityTargetTemp);
}

void PrintInterfaceDisplayInit(void)
{
  if (1 == t_sys.enable_cavity_temp)
  {
    Custom_PrintInterfaceDisplayInit();
  }
  else
  {
    char printnameb[_MAX_LFN];

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      SetTextDisplayRange(100, 22, 12 * 3, 24, &NozzleTempTextRange); //������ʾ����
      SetTextDisplayRange(100, 62, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange(100 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 22, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange(100 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 62, 12 * 3, 24, &HotBedTargetTempTextRange);
      SetTextDisplayRange(250, 286, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRange(100, 102, 12 * 9, 24, &PrintTimeTextRange);
      SetTextDisplayRange(100, 142, 12 * 9, 24, &SpeedRange);
      ReadTextDisplayRangeInfo(SpeedRange, TextRangeBuf_Speed);
      strcpy(printnameb, SettingInfoToSYS.PrintFileName);

      if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4) //������ʾ��ʽ��һ���ֽ�ռ12��
      {
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4] = 0;
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 1] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 2] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 3] = '.';
      }

      DisplayText((uint8_t *)printnameb, (int)(375 - (strlen(printnameb) / 2) * 12), 260, 24, (u16)testcolor); //240-(length/2)*12��Ϊ����������ʾ���м�
      //���´�ӡ����
      snprintf(printnameb, sizeof(printnameb), "%2d%%", (int)t_gui.print_percent);

      if (Draw_progressBar(t_gui.printfile_size, t_gui.file_size - (uint32_t)((t_gui.printfile_size + 99) * 0.01)))
      {
        ReadTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule);
        CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
        DisplayTextInRange((unsigned char *)printnameb, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
      }
    }
    else
    {
      SetTextDisplayRange(132, 36, 12 * 3, 24, &NozzleTempTextRange); //������ʾ����
      SetTextDisplayRange(335, 36, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange(132 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange(335 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &HotBedTargetTempTextRange);
      SetTextDisplayRange(132, 114, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRange(335, 114, 12 * 9, 24, &PrintTimeTextRange);
      ReadTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule);

      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        SetTextDisplayRange(132, 66, 12 * 3, 24, &NozzleTempTextRange1); //������ʾ����
        SetTextDisplayRange(132 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 66, 12 * 3, 24, &NozzleTargetTempTextRange1);
        ReadTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1); //��lcd��ȡ���أ����浽����
        ReadTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1);
      }
    }

    ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp); //��lcd��ȡ���أ����浽����
    ReadTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp);
    ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp);
    ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp);
    //  ReadTextDisplayRangeInfo(PrintScheduleTextRange,TextRangeBuf_PrintSchedule);
    ReadTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time);
  }
}

void Custom_PrintInterfaceDisplayText(void)
{
  char TextBuffer[20];
  int second, minute, hour;
  //��ʾ�����¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾб��
  DisplayText((unsigned char *)"/", 132 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36, 24, (u16)testcolor); //ֱ����ʾ��lcd
  //��ʾ����Ŀ���¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

  if (!t_custom_services.disable_hot_bed)
  {
    //��ʾ�ȴ��¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, HotBedTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    //��ʾб��
    DisplayText((unsigned char *)"/", 324 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36, 24, (u16)testcolor);
    //��ʾ�ȴ�Ŀ���¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  }

  //��ʾ��ӡ����
  snprintf(TextBuffer, sizeof(TextBuffer), "%2d%%", (int)t_gui.print_percent);

  if (Draw_progressBar_new(t_gui.printfile_size, t_gui.file_size, 92, 148, 344, 20))
  {
    CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  }

  //��ʾ�����¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.cavity_temp);
  CopyTextDisplayRangeInfo(CavityTempTextRange, TextRangeBuf_CavityTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, CavityTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾб��
  DisplayText((unsigned char *)"/", 132 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 84, 24, (u16)testcolor); //ֱ����ʾ��lcd
  //��ʾ����Ŀ���¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_cavity_temp);
  CopyTextDisplayRangeInfo(CavityTargetTempTextRange, TextRangeBuf_CavityTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, CavityTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  //��ʾ��ӡʱ��
  second = t_gui.printed_time_sec;
  hour = second / 3600;
  minute = (second - hour * 3600) / 60;
  second = (second - hour * 3600) % 60;
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d:%2d:%2d", hour, minute, second);
  CopyTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, PrintTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
}


void PrintInterfaceDisplayText(void)
{
  if (1 == t_sys.enable_cavity_temp)
  {
    Custom_PrintInterfaceDisplayText();
  }
  else
  {
    char TextBuffer[20];
    int second, minute, hour;
    //��ʾ�����¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

    //��ʾб��
    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
      DisplayText((unsigned char *)"/", 100 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 22, 24, (u16)testcolor); //ֱ����ʾ��lcd
    else
      DisplayText((unsigned char *)"/", 132 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36, 24, (u16)testcolor); //ֱ����ʾ��lcd

    //��ʾ����Ŀ���¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      //��ʾ�����¶�
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[1]);
      CopyTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);

      //��ʾб��
      if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
        DisplayText((unsigned char *)"/", 100 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 52, 24, (u16)testcolor); //ֱ����ʾ��lcd
      else
        DisplayText((unsigned char *)"/", 132 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 66, 24, (u16)testcolor); //ֱ����ʾ��lcd

      //��ʾ����Ŀ���¶�
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[1]);
      CopyTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
    }

    if (!t_custom_services.disable_hot_bed)
    {
      //��ʾ�ȴ��¶�
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.hot_bed_temp);
      CopyTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, HotBedTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

      //��ʾб��
      if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
        DisplayText((unsigned char *)"/", 100 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 62, 24, (u16)testcolor);
      else
        DisplayText((unsigned char *)"/", 335 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36, 24, (u16)testcolor);

      //��ʾ�ȴ�Ŀ���¶�
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_hot_bed_temp);
      CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    }

    //��ʾ��ӡ����
    snprintf(TextBuffer, sizeof(TextBuffer), "%2d%%", (int)t_gui.print_percent);

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      if (Draw_progressBar(t_gui.printfile_size, t_gui.file_size))
      {
        //ReadTextDisplayRangeInfo(PrintScheduleTextRange,TextRangeBuf_PrintSchedule);
        CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
        DisplayTextInRange((unsigned char *)TextBuffer, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
      }
    }
    else
    {
      CopyTextDisplayRangeInfo(PrintScheduleTextRange, TextRangeBuf_PrintSchedule, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, PrintScheduleTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    }

    //��ʾ��ӡʱ��
    second = t_gui.printed_time_sec;
    hour = second / 3600;
    minute = (second - hour * 3600) / 60;
    second = (second - hour * 3600) % 60;
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d:%2d:%2d", hour, minute, second);
    CopyTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, PrintTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      snprintf(TextBuffer, sizeof(TextBuffer), "%5.1fmm/s", t_gui.print_speed_value * t_gui.cura_speed / 100.0f);
      CopyTextDisplayRangeInfo(SpeedRange, TextRangeBuf_Speed, TextRangeBuf_Str);
      DisplayTextInRange((unsigned char *)TextBuffer, SpeedRange, TextRangeBuf_Str, 24, (u16)testcolor);
    }
  }
}

#define PauseInterface 1
#define ResumeInterface 2

typedef enum
{
  NO_KEY = 0,
  PrintSetLeft_KEY,
  PrintSetRight_KEY,
  PrintSetCavity_KEY,
  PauseOrResume_KEY,
  ChangeFilament_KEY,
  Stop_KEY,
} KEYValueTypeDef;

KEYValueTypeDef Custom_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(132, 36, 232, 66)) //�����¶� || ��ӡ�ٶ�
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //�ȴ��¶�
  {
    if (touchxy(324, 36, 424, 66)) //�ȴ��¶� || �����ٶ�
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (touchxy(132, 86, 232, 116))
  {
    KEYValue = PrintSetCavity_KEY;
  }

  if (touchxy(48, 200, 144, 280)) //��ͣ��ӡ �� ������ӡ
  {
    KEYValue = PauseOrResume_KEY;
  }

  if (have_chg_filament && touchxy(192, 200, 290, 282)) //��;����
  {
    KEYValue = ChangeFilament_KEY;
  }

  if (touchxy(338, 200, 434, 280)) //ֹͣ��ӡ��ť
  {
    KEYValue = Stop_KEY;
  }

  return KEYValue;
}

KEYValueTypeDef sgcode_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(0, 0, 255, 45) || touchxy(0, 130, 255, 170)) //�����¶� || ��ӡ�ٶ�
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //�ȴ��¶�
  {
    if (touchxy(0, 46, 255, 90)) //�ȴ��¶� || �����ٶ�
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (touchxy(0, 180, 89, 260)) //��ͣ��ӡ �� ������ӡ
  {
    KEYValue = PauseOrResume_KEY;
  }

  if (have_chg_filament && touchxy(90, 180, 179, 260)) //��;����
  {
    KEYValue = ChangeFilament_KEY;
  }

  if (touchxy(180, 180, 265, 260)) //ֹͣ��ӡ��ť
  {
    KEYValue = Stop_KEY;
  }

  return KEYValue;
}

KEYValueTypeDef normal_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(0, 0, 225, 67))
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //NotM14_Right
  {
    if (touchxy(225, 0, 480, 67))
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (have_chg_filament)
  {
    if (touchxy(50, 212, 144, 280)) //��ͣ��ӡ �� ������ӡ
    {
      KEYValue = PauseOrResume_KEY;
    }

    if (touchxy(194, 210, 290, 282)) //��;����
    {
      KEYValue = ChangeFilament_KEY;
    }

    if (touchxy(338, 212, 434, 282)) //ֹͣ��ӡ��ť
    {
      KEYValue = Stop_KEY;
    }
  }
  else
  {
    if (touchxy(94, 212, 190, 278)) //��ͣ��ӡ �� ������ӡ
    {
      KEYValue = PauseOrResume_KEY;
    }

    if (touchxy(288, 212, 384, 280)) //ֹͣ��ӡ��ť
    {
      KEYValue = Stop_KEY;
    }
  }

  return KEYValue;
}

int InterfaceTouchCheck(uint8_t SelectPauseOrResumeInterface, bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  /******************************����ɨ��**************************************/
  if (1 == t_sys.enable_cavity_temp)
  {
    KEYValue = Custom_get_key_value(have_chg_filament);
  }
  else if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
  {
    KEYValue = sgcode_get_key_value(have_chg_filament);
  }
  else
  {
    KEYValue = normal_get_key_value(have_chg_filament);
  }

  /****************************����ֵ����ִ��*******************************************/
  switch (KEYValue)
  {
  case PrintSetLeft_KEY:
    if (t_custom_services.disable_hot_bed) //M14
    {
      gui::set_current_display(PrintSet_M14);
    }
    else  //NotM14_Left
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
      {
        gui::set_current_display(PrintSet_NotM14_Left_dual);
      }
      else
      {
        gui::set_current_display(PrintSet_NotM14_Left);
      }
    }

    return 1;

  case PrintSetRight_KEY:
    gui::set_current_display(PrintSet_NotM14_Right);
    return 1;

  case PrintSetCavity_KEY:
    gui::set_current_display(PrintSet_Cavity);
    return 1;

  case PauseOrResume_KEY:
    if (PauseInterface == SelectPauseOrResumeInterface)
    {
      gui::set_current_display(PausePrintF);               //���ý���ˢ��
    }
    else
    {
      gui::set_current_display(ResumePrintF);               //���ý���ˢ��
    }

    return 1;

  case ChangeFilament_KEY:
    if (have_chg_filament)
    {
      gui::set_current_display(ChangeFilamentConfirm);
      return 1;
    }

  case Stop_KEY:
    gui::set_current_display(stopprintF);
    return 1;

  default:
    break;
  }

  /****************************��������ִ��*******************************************/
  if ((IsComputerControlToPausePrint) || (IsComputerControlToResumePrint)) //���Կ�������ͣ������
  {
    IsComputerControlToPausePrint = 0;
    IsComputerControlToResumePrint = 0;

    if (PauseInterface == SelectPauseOrResumeInterface)
    {
      waiting_for_pausing();
    }
    else
    {
      waiting_for_resuming();
    }

    return 1;
  }

  if (IsComputerControlToStopPrint) //���Զ˿���ֹͣ��ӡ
  {
    IsComputerControlToStopPrint = 0;
    waiting_for_stopping();
    return 1;
  }

  //��ӡ�м�⵽û�в���
  if (IsNotHaveMatInPrint)
  {
    gui::set_current_display(NoHaveMatWaringInterface);
    return 1;
  }

  return 0;
}

int pause_or_resume_interface(int status, bool have_chg_filament)
{
  if (gui::is_refresh())            //����Ƿ���Ҫ���£���ʼ��
  {
    if (1 == t_sys.enable_cavity_temp)
    {
      if (PauseInterface == status)
        display_picture(107);
      else if (ResumeInterface == status)
        display_picture(108);

      if (!have_chg_filament)
        LCD_Fill(192, 210, 290, 282, BACKBLUE); //��ס��;����
    }
    else if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      if (PauseInterface == status)
        display_picture(96);
      else if (ResumeInterface == status)
        display_picture(97);

      if (!have_chg_filament)
        LCD_Fill(95, 185, (t_sys.lcd_ssd1963_43_480_272 ? 165 : 175), 255, BACKBLUE); //��ס��;����

      diplayBMP(t_sys.lcd_ssd1963_43_480_272 ? 4 : 24); //��ʾδѹ����bmpͼƬ
    }
    else if (t_custom_services.disable_hot_bed)
    {
      if (PauseInterface == status)
        display_picture(have_chg_filament ? 33 : 36);
      else if (ResumeInterface == status)
        display_picture(have_chg_filament ? 34 : 37);
    }
    else
    {
      if (PauseInterface == status)
        display_picture(have_chg_filament ? 2 : 28);
      else if (ResumeInterface == status)
        display_picture(have_chg_filament ? 3 : 29);
    }

    PrintInterfaceDisplayInit();
    PrintInterfaceDisplayText();
  }

  if (InterfaceTouchCheck(status, have_chg_filament))
    return 1;

  if (gui::is_refresh_rtc())   //����rtc�źŸ�����ֵ��ʾ
  {
    PrintInterfaceDisplayText();

    if (CheckIsPrintFinish())
      return 1;

    if (PauseInterface == status)
    {
      if (!have_chg_filament && CheckIsHaveUdisk())
        return 1;
    }
    else if (ResumeInterface == status)
    {
      if (CheckIsHaveUdisk())
        return 1;
    }
  }

  return 0;
}

//#define DEBUG_Tempertuer_PID//debug pid temperatuer

void MainInterfaceDisplayInit(void)
{
  if (1 == t_sys.enable_cavity_temp)
  {
    SetTextDisplayRange(138, 32, 12 * 3, 24, &NozzleTempTextRange);
    SetTextDisplayRange(344, 32, 12 * 3, 24, &HotBedTempTextRange);
    SetTextDisplayRange(138, 86, 12 * 3, 24, &CavityTempTextRange);
    SetTextDisplayRange(138 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 32, 12 * 3, 24, &NozzleTargetTempTextRange);
    SetTextDisplayRange(344 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 32, 12 * 3, 24, &HotBedTargetTempTextRange);
    SetTextDisplayRange(138 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 86, 12 * 3, 24, &CavityTargetTempTextRange);
    ReadTextDisplayRangeInfo(CavityTempTextRange, TextRangeBuf_CavityTemp);
    ReadTextDisplayRangeInfo(CavityTargetTempTextRange, TextRangeBuf_CavityTargetTemp);
  }
  else
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      SetTextDisplayRange(137, 35, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRange(137, 60, 12 * 3, 24, &NozzleTempTextRange1);
      SetTextDisplayRange(345, 35, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange(137 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange(137 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 60, 12 * 3, 24, &NozzleTargetTempTextRange1);
      SetTextDisplayRange(345 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &HotBedTargetTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1);
      ReadTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1);
    }
    else
    {
      SetTextDisplayRange(137, 35, 12 * 3, 24, &NozzleTempTextRange);
      SetTextDisplayRange(345, 35, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange(137 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange(345 + 12 * 4 + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &HotBedTargetTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp);
      ReadTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp);
      ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp);
      ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp);
    }
  }

  ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp);
  ReadTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp);
  ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp);
  #ifdef DEBUG_Tempertuer_PID
  SetTextDisplayRange(335, 114, 12 * 9, 24, &PrintTimeTextRange);
  ReadTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time);
  #endif
}

extern void protect_nozzle(float hour);

void MainInterfaceDisplayText(void)
{
  char TextBuffer[20];
  protect_nozzle(1.5);
  //��ʾ�����¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

  //��ʾб��
  if (1 == t_sys.enable_cavity_temp)
  {
    DisplayText((unsigned char *)"/", 138 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 32, 24, (u16)testcolor);
  }
  else
  {
    DisplayText((unsigned char *)"/", 137 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 35, 24, (u16)testcolor);
  }

  //��ʾ����Ŀ���¶�
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[0]);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, TextRangeBuf_NozzleTargetTemp, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.nozzle_temp[1]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange1, TextRangeBuf_NozzleTemp1, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, NozzleTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_nozzle_temp[1]);
    CopyTextDisplayRangeInfo(NozzleTargetTempTextRange1, TextRangeBuf_NozzleTargetTemp1, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, NozzleTargetTempTextRange1, TextRangeBuf_Str, 24, (u16)testcolor);
    DisplayText((unsigned char *)"/", 137 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 60, 24, (u16)testcolor);
  }

  if (!t_custom_services.disable_hot_bed)
  {
    //��ʾ�ȴ��¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTempTextRange, TextRangeBuf_HotBedTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, HotBedTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

    //��ʾб��
    if (1 == t_sys.enable_cavity_temp)
    {
      DisplayText((unsigned char *)"/", 344 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 32, 24, (u16)testcolor);
    }
    else
    {
      DisplayText((unsigned char *)"/", 345 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 35, 24, (u16)testcolor);
    }

    //��ʾ�ȴ�Ŀ���¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, TextRangeBuf_HotBedTargetTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  }

  if (1 == t_sys.enable_cavity_temp)
  {
    //��ʾ�����¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.cavity_temp);
    CopyTextDisplayRangeInfo(CavityTempTextRange, TextRangeBuf_CavityTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, CavityTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    //��ʾб��
    DisplayText((unsigned char *)"/", 138 + 12 * 3 + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 86, 24, (u16)testcolor); //ֱ����ʾ��lcd
    //��ʾ����Ŀ���¶�
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)t_gui.target_cavity_temp);
    CopyTextDisplayRangeInfo(CavityTargetTempTextRange, TextRangeBuf_CavityTargetTemp, TextRangeBuf_Str);
    DisplayTextInRange((unsigned char *)TextBuffer, CavityTargetTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  }

  #ifdef DEBUG_Tempertuer_PID
  snprintf(TextBuffer, sizeof(TextBuffer), "f%0.4f ", t_sys_data_current.pid_output_factor);
  CopyTextDisplayRangeInfo(PrintTimeTextRange, TextRangeBuf_Time, TextRangeBuf_Str);
  DisplayTextInRange((unsigned char *)TextBuffer, PrintTimeTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
  #endif
}

void lcd_debug_interface(void)
{
  #ifdef LCD_DEBUG
  extern menufunc_t lcdtest_lastdisplay;
  extern void board_test_cal_touch_count(void);

  //TFTLCD���󴥷��������ԣ�¬��2017.4.20
  if (touchxy(192, 109, 300, 200)) //���ذ�������׼������U�̼�֮��
  {
    //    t_sys_data_current.have_set_machine = 0;
    //    motion_3d.enable_board_test = 1;
    lcdtest_lastdisplay = currentdisplay;
    gui::set_current_display(board_test_cal_touch_count);
  }

  #endif
}

void debug_presure_sensor(void)
{
  //#define Debug_PresureSensor
  #ifdef Debug_PresureSensor
#include "functioncustom.h"
  extern menufunc_t lcdtest_lastdisplay;

  if (touchxy(0, 109, 105, 200)) //���ذ�������׼������U�̼�֮��
  {
    lcdtest_lastdisplay = currentdisplay;
    gui::set_current_display(board_test_pressure);
    return 1;
  }

  #endif
}

int MainInterfaceTouchCheck(void)
{
  if (1 == t_sys.enable_cavity_temp)
  {
    if (touchxy(138, 32, 238, 62))
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

    if (touchxy(344, 32, 444, 62))
    {
      gui::set_current_display(PrintSet_NotM14_Right);
      return 1;
    }

    if (touchxy(138, 86, 238, 116))
    {
      gui::set_current_display(PrintSet_Cavity);
      return 1;
    }
  }
  else
  {
    if (touchxy(0, 0, 240, 65))
    {
      if (t_custom_services.disable_hot_bed) //M14
      {
        gui::set_current_display(PrintSet_M14);
      }
      else  //NotM14_Left
      {
        if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
        {
          gui::set_current_display(PrintSet_NotM14_Left_dual);
        }
        else
        {
          gui::set_current_display(PrintSet_NotM14_Left);
        }
      }

      return 1;
    }

    if (!t_custom_services.disable_hot_bed) //NotM14_Right
    {
      if (touchxy(240, 0, 480, 65))
      {
        gui::set_current_display(PrintSet_NotM14_Right);
        return 1;
      }
    }
  }

  if (1 == t_sys.enable_cavity_temp)
  {
    if (touchxy(121, 138, 198, 218))
    {
      gui::set_current_display(prepareF);
      return 1;
    }

    debug_presure_sensor();
    lcd_debug_interface();

    if (touchxy(74, 228, 150, 306))
    {
      gui::set_current_display(settingF);
      return 1;
    }

    if (touchxy(170, 228, 244, 306))
    {
      gui::set_current_display(statusF);
      return 1;
    }

    if (touchxy(302, 142, 402, 306))
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
  }
  else
  {
    if (touchxy(107, 109, 191, 193))
    {
      gui::set_current_display(prepareF);
      return 1;
    }

    debug_presure_sensor();
    lcd_debug_interface();

    if (touchxy(55, 205, 140, 292))
    {
      gui::set_current_display(settingF);
      return 1;
    }

    if (touchxy(163, 205, 243, 292))
    {
      gui::set_current_display(statusF);
      return 1;
    }

    if (touchxy(308, 111, 418, 292))
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
  }

  if (t_power_off.flag == 1) //�ϵ�����, ע�������־λ�Ǵ�flash�ж����ٸ�ֵ�����ģ�flash������Ĭ����0xFF������д�ж����Ҫд��== 1��
  {
    t_power_off.flag = 0;
    strcpy(printname, t_power_off.file_name);
    gui::set_current_display(PowerOffRecover);
    return 1;
  }

  return 0;
}

int MainInterface(void)
{
  if (gui::is_refresh())
  {
    if (t_custom_services.disable_hot_bed)
    {
      display_picture(32);
    }
    else
    {
      if (1 == t_sys.enable_cavity_temp)
      {
        display_picture(106);
      }
      else
      {
        display_picture(1);
      }
    }

    MainInterfaceDisplayInit();
    MainInterfaceDisplayText();
  }

  if (MainInterfaceTouchCheck())
    return 1;

  if (gui::is_refresh_rtc())
  {
    MainInterfaceDisplayText();
  }

  return 1;
}

#define HeatFinish 1
#define CurrentInterfaceIsNotPrintInterface 0
#define CurrentInterfaceIsPauseAndNoChangeFilament  1
#define CurrentInterfaceIsPauseAndHaveChangeFilament  2
#define CurrentInterfaceIsResumeAndNoChangeFilament  3
#define CurrentInterfaceIsResumeAndHaveChangeFilament  4

void IsRefreshPrintInterface(uint8_t CurrentInterface)
{
  static uint8_t LastPrintInterfaceValue = CurrentInterfaceIsNotPrintInterface;

  if (CurrentInterface != CurrentInterfaceIsNotPrintInterface)
  {
    if (LastPrintInterfaceValue != CurrentInterface) //��ӡʱ����ǰҪ��ʾ�Ľ�����ϴ���ʾ�Ľ��治һ����ˢ�½���ͼƬ
      gui::need_refresh();
  }

  LastPrintInterfaceValue = CurrentInterface;
}

void PrintInterface(void)
{
  if (HeatFinish == ChangeFilamentHeatStatus) //����˼��Ⱥ���ʾ������;���ϵĽ���
  {
    IsRefreshPrintInterface(!pauseprint ? CurrentInterfaceIsPauseAndHaveChangeFilament : CurrentInterfaceIsResumeAndHaveChangeFilament);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (print_display_dual(!pauseprint ? PauseInterface : ResumeInterface, true))
        return;
    }
    else
    {
      if (pause_or_resume_interface(!pauseprint ? PauseInterface : ResumeInterface, true))
        return;
    }
  }
  else  //û����ɼ��Ⱥ͹������ʾû�к�����;���ϵĽ���
  {
    IsRefreshPrintInterface(!pauseprint ? CurrentInterfaceIsPauseAndNoChangeFilament : CurrentInterfaceIsResumeAndNoChangeFilament);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (print_display_dual(!pauseprint ? PauseInterface : ResumeInterface, false))
        return;
    }
    else
    {
      if (pause_or_resume_interface(!pauseprint ? PauseInterface : ResumeInterface, false))
        return;
    }
  }

  if (!pauseprint && IsDoorOpen)
  {
    gui::set_current_display(DoorOpenWarningInfo_Printing);
  }

  lcd_debug_interface();
}

void maindisplayF(void)
{
  if (print_flage == 1) //���ڴ�ӡ����ʾ��ӡ����
  {
    PrintInterface();
  }
  else if (print_flage == 0) //û�д�ӡ����ʾ������
  {
    IsRefreshPrintInterface(CurrentInterfaceIsNotPrintInterface);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (main_display_dual())
        return ;
    }
    else
    {
      if (MainInterface())
        return ;
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

