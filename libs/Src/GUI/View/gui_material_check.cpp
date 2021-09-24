#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "user_ccm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TextRangeBuf_Str ccm_param::TextRangeBuf_24_12_9_1
#define TextRangeBuf_NozzleTemp ccm_param::TextRangeBuf_24_12_3_6

void MatCheckCalibrateFinished(void)
{
  char TextBuffer[20];

  if (gui::is_refresh())
  {
    display_picture(59);
    //��ʾ���ϼ��ģ��У׼ֵ
    snprintf(TextBuffer, sizeof(TextBuffer), "%.2f", t_sys_data_current.material_chk_vol_value);
    DisplayText((uint8_t *)TextBuffer, 334, 143, 24, (u16)testcolor);
  }

  if (touchxy(155, 185, 335, 285))
  {
    gui::set_current_display(settingF);
  }
}

void MatCheckCalibrate(void) //���ϼ��У׼
{
  if (gui::is_refresh())
  {
    display_picture(58);
  }

  if (IsFinishMatCheckCalibrate)
  {
    IsFinishMatCheckCalibrate = 0;
    gui::set_current_display(MatCheckCalibrateFinished);
  }
}

//void MatCheckCalibrateReady(void) //���ϼ��У׼׼��
//{
//  if(gui::is_refresh())
//  {
//    display_picture(57);
//  }
//
//  if(touchxy(60,200,240,285))
//  {
//    gui::set_current_display(MatCheckCalibrate);
//
//    SettingInfoToSYS.GUISempValue=MatCheckCalibrateValue;
//    GUISendSempToSYS();
//    return ;
//  }
//  if(touchxy(240,200,425,285))
//  {
//    gui::set_current_display(settingF);
//    return ;
//  }
//
//}
/*20170803*/
extern float blockdetect_OldEposition;

void NotHaveMatControlInterface2(void) //���ϲ�������2
{
  if (gui::is_refresh())
  {
    display_picture(61);
  }

  if (touchxy(106, 148, 196, 218)) //������ӡ
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_resuming();
    return ;
  }

  if (touchxy(270, 148, 361, 218)) //ֹͣ��ӡ
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_stopping();
    return ;
  }
}

extern struct _textrange NozzleTempTextRange;    //�����¶ȵ���ʾ����
void HeatNozzleToChangeFilament(void)
{
  char buffer[20];

  if (gui::is_refresh())
  {
    display_picture(64);
    SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
    ReadTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp);
    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);
    snprintf(buffer, sizeof(buffer), "/%3d", (int)t_gui.target_nozzle_temp[0]);
    DisplayText((uint8_t *)buffer, 244 + 12 * 3, 49, 24, (u16)testcolor);
  }

  if (gui::is_refresh_rtc())
  {
    snprintf(buffer, sizeof(buffer), "%3d", (int)t_gui.nozzle_temp[0]);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, TextRangeBuf_NozzleTemp, TextRangeBuf_Str);
    DisplayTextInRange((uint8_t *)buffer, NozzleTempTextRange, TextRangeBuf_Str, 24, (u16)testcolor);

    if (t_gui.nozzle_temp[0] >= (t_gui.target_nozzle_temp[0] - 5)) //���5����Ҫ�ȴ��ϳ�ʱ�䣬���ȴ�
    {
      respond_gui_send_sem(ConfirmChangeFilamentValue);
      gui::set_current_display(NotHaveMatToChangeFilament);
    }
  }
}
bool IsPauseToCoolDown(void);
void NotHaveMatControlInterface1(void) //���ϲ�������1
{
  if (gui::is_refresh())
  {
    display_picture(60);
  }

  if (touchxy(106, 148, 196, 218)) //����
  {
    if (IsPauseToCoolDown()) //��ͣ��ӡ��ʱ�򽵵����¶�
    {
      respond_gui_send_sem(PauseToResumeNozzleTemp);
      gui::set_current_display(HeatNozzleToChangeFilament);
    }
    else
    {
      respond_gui_send_sem(ConfirmChangeFilamentValue);
      gui::set_current_display(NotHaveMatToChangeFilament);
    }

    return ;
  }

  if (touchxy(270, 148, 361, 218)) //ֹͣ��ӡ
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_stopping();
    return ;
  }
}

void NoHaveMatWaringInterface(void) //������ʾ����
{
  static unsigned long BeepWaringTime = 0;

  if (gui::is_refresh())
  {
    display_picture(62);
    respond_gui_send_sem(OpenBeep);
    BeepWaringTime = xTaskGetTickCount() + 5000; //����5��
  }

  if (BeepWaringTime < xTaskGetTickCount()) //ʱ�䵽�ر�����
  {
    respond_gui_send_sem(CloseBeep);
  }

  if (touchxy(155, 185, 335, 285)) //���ȷ��
  {
    respond_gui_send_sem(CloseBeep);
    gui::set_current_display(NotHaveMatControlInterface1);
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

