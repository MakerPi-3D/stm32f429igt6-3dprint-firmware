#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

static unsigned long DoorOpenTimeCount = 0;

void DoorOpenWarning_StartPrint(void) //选择文件开始打印，但门打开，显示提示信息
{
  if (gui::is_refresh())
  {
    display_picture(50);
  }

  if (touchxy(155, 185, 335, 285))
  {
    gui::set_current_display(filescanF);
    return ;
  }
}

void StopPrintInfo(void) //门打开导致的停止打印，显示提示信息
{
  if (gui::is_refresh())
  {
    display_picture(52);
  }

  if (touchxy(155, 185, 340, 290))
  {
    gui::set_current_display(maindisplayF);
  }
}

static menufunc_t LastDisplay;
static uint8_t DisplayingDoorOpenInfo = 0;
void DoorOpenWarningInfo(void)
{
  if (gui::is_refresh())
  {
    display_picture(51);
  }

  if (touchxy(155, 185, 335, 285) || t_gui.nozzle_temp[0] < 60)
  {
    DisplayingDoorOpenInfo = 0;
    gui::set_current_display(LastDisplay);
    return ;
  }
}

void DoorOpenWarningInfo_NotPrinting(void) //待机、预热、进丝、退丝 时 温度大于60度且门打开显示提示信息
{
  if ((IsDisplayDoorOpenInfo) && (DisplayingDoorOpenInfo == 0))
  {
    USER_EchoLogStr("DoorOpenWarningInfo\r\n");//串口上传信息到上位机2017.7.6
    DisplayingDoorOpenInfo = 1;
    LastDisplay = gui::current_display;
    gui::set_current_display(DoorOpenWarningInfo);
  }

  #if USER_DEBUG_LEVEL>0  //开启usart通信，才使用下面的代码

  if ((IsDisplayDoorOpenInfo == 0) && (DisplayingDoorOpenInfo == 1))
  {
    USER_EchoLogStr("DoorClosed!\r\n");//串口上传信息到上位机2017.7.6
    DisplayingDoorOpenInfo = 0;
  }

  #endif
}

void DoorOpenWarningInfo_Printing(void) //打印时 温度大于60度且门打开显示提示信息
{
  static uint8_t LastStatus;

  if (gui::is_refresh())
  {
    display_picture(54);
    LastStatus = 0;
    DoorOpenTimeCount = xTaskGetTickCount() + 15 * 1000UL; //限制时间
  }

  if ((IsDoorOpen) && LastStatus == 0)
  {
    respond_gui_send_sem(OpenBeep);
    LastStatus = 1;
  }
  else if ((!IsDoorOpen) && LastStatus == 1)
  {
    respond_gui_send_sem(CloseBeep);
    LastStatus = 0;
  }

  if (DoorOpenTimeCount < xTaskGetTickCount()) //限制时间到
  {
    if (IsDoorOpen)
    {
      USER_EchoLogStr("DoorOpenWarningInfo_StopPrint\r\n");//串口上传信息到上位机2017.7.6
      respond_gui_send_sem(CloseBeep);
      respond_gui_send_sem(StopPrintValue);
      gui::set_current_display(StopPrintInfo);
    }
    else
    {
      gui::set_current_display(maindisplayF);
    }
  }

  if (touchxy(155, 185, 340, 290)) //确定键
  {
    respond_gui_send_sem(CloseBeep);
    gui::set_current_display(maindisplayF);
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

