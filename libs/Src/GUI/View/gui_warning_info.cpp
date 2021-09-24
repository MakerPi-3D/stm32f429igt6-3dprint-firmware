#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "temperature.h"
#include "sysconfig_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int WarningInfoFlag;

#define BeepWarningTime 5000
unsigned long BeepWarningTimeout;


#define MinTempInfo 0
#define MaxTempInfo 1
#define MaxTempBedInfo 2
#define HeatFailInfo 3
#define XMinLimitInfo 4
#define YMinLimitInfo 5
#define ZMinLimitInfo 6
#define XMaxLimitInfo 7
#define YMaxLimitInfo 8
#define ZMaxLimitInfo 9
#define IWDGResetInfo 10
#define FatfsInfo 11
#define ThermistorFallsInfo 12

void WarningInfoF(void)
{
  static bool SendCloseBeepOnece;

  if (gui::is_refresh())
  {
    switch (WarningInfoSelect)
    {
    case MinTempInfo:
      display_picture(67);
      break;

    case MaxTempInfo:
      display_picture(66);
      break;

    case MaxTempBedInfo:
      display_picture(68);
      break;

    case HeatFailInfo:
      display_picture(24);
      break;

    case XMinLimitInfo:
      display_picture(69);
      break;

    case YMinLimitInfo:
      display_picture(70);
      break;

    case ZMinLimitInfo:
      display_picture(71);
      break;

    case XMaxLimitInfo:
      display_picture(72);
      break;

    case YMaxLimitInfo:
      display_picture(73);
      break;

    case ZMaxLimitInfo:
      display_picture(74);
      break;

    case IWDGResetInfo:
      display_picture(23);
      break;

    case FatfsInfo:
      display_picture(75);
      break;

    case ThermistorFallsInfo:
      display_picture(112);
      break;

    default:
      break;
    }

    respond_gui_send_sem(OpenBeep);
    BeepWarningTimeout = xTaskGetTickCount() + BeepWarningTime;
    SendCloseBeepOnece = TRUE;

    if (print_flage == 1) //若显示警告信息的时候正在打印则停止它
    {
      respond_gui_send_sem(StopPrintValue);
      // 出错关闭激光模块
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
        temperature_disable_heater();
    }
  }

  if (touchxy(155, 185, 340, 290))
  {
    WarningInfoFlag = 1;
    pauseprint = 0;
    print_flage = 0;
    respond_gui_send_sem(CloseBeep);
    respond_gui_send_sem(SysErrValue);
    gui::set_current_display(maindisplayF);
    return ;
  }

  if (SendCloseBeepOnece)
  {
    SendCloseBeepOnece = FALSE;

    if (BeepWarningTimeout < xTaskGetTickCount())
    {
      respond_gui_send_sem(CloseBeep);
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif


