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
    //显示断料检测模块校准值
    snprintf(TextBuffer, sizeof(TextBuffer), "%.2f", t_sys_data_current.material_chk_vol_value);
    DisplayText((uint8_t *)TextBuffer, 334, 143, 24, (u16)testcolor);
  }

  if (touchxy(155, 185, 335, 285))
  {
    gui::set_current_display(settingF);
  }
}

void MatCheckCalibrate(void) //断料检测校准
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

//void MatCheckCalibrateReady(void) //断料检测校准准备
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

void NotHaveMatControlInterface2(void) //断料操作界面2
{
  if (gui::is_refresh())
  {
    display_picture(61);
  }

  if (touchxy(106, 148, 196, 218)) //继续打印
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_resuming();
    return ;
  }

  if (touchxy(270, 148, 361, 218)) //停止打印
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_stopping();
    return ;
  }
}

extern struct _textrange NozzleTempTextRange;    //喷嘴温度的显示区域
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

    if (t_gui.nozzle_temp[0] >= (t_gui.target_nozzle_temp[0] - 5)) //最后5度需要等待较长时间，不等待
    {
      respond_gui_send_sem(ConfirmChangeFilamentValue);
      gui::set_current_display(NotHaveMatToChangeFilament);
    }
  }
}
bool IsPauseToCoolDown(void);
void NotHaveMatControlInterface1(void) //断料操作界面1
{
  if (gui::is_refresh())
  {
    display_picture(60);
  }

  if (touchxy(106, 148, 196, 218)) //换料
  {
    if (IsPauseToCoolDown()) //暂停打印的时候降低了温度
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

  if (touchxy(270, 148, 361, 218)) //停止打印
  {
    IsNotHaveMatInPrint = 0;
    waiting_for_stopping();
    return ;
  }
}

void NoHaveMatWaringInterface(void) //断料提示界面
{
  static unsigned long BeepWaringTime = 0;

  if (gui::is_refresh())
  {
    display_picture(62);
    respond_gui_send_sem(OpenBeep);
    BeepWaringTime = xTaskGetTickCount() + 5000; //鸣叫5秒
  }

  if (BeepWaringTime < xTaskGetTickCount()) //时间到关闭鸣叫
  {
    respond_gui_send_sem(CloseBeep);
  }

  if (touchxy(155, 185, 335, 285)) //点击确定
  {
    respond_gui_send_sem(CloseBeep);
    gui::set_current_display(NotHaveMatControlInterface1);
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

