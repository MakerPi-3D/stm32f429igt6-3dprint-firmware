#include "machinecustom.h"
#include "user_common.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "midwaychangematerial.h"
#include "common.h"

#include "controlfunction.h"
#include "boardtest.h"
#include "PrintControl.h"
#include "midwaychangematerial.h"
#include "temperature.h"

#include "process_m_code.h"
#include "config_model_tables.h"

#ifdef CAL_Z_ZERO_OFFSET
  #include "common.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include "interface.h"
#include "UdiskControl.h"
#include "PrintControl.h"
#include "RespondGUI.h"
#include "process_command.h"

#include "USBFileTransfer.h"

#define Pop 0
#define NotPop 1

uint8_t MinTempWarningPopSet = Pop;
uint8_t MaxTempWarningPopSet = Pop;
uint8_t MaxTempBedWarningPopSet = Pop;
uint8_t HeatFailWarningPopSet = Pop;
uint8_t MachineSizeLimitWarningPopSet = Pop;
uint8_t IWDGResetWarningPopSet = Pop;
uint8_t FatfsFailWarningPopSet = Pop;
uint8_t ThermistorFallsWarnPopSet = Pop;

void PopWarningInfo(uint8_t WarningSelectValue)//弹出警告信息
{
  uint8_t IsPop = NotPop;
  WarningInfoSelect = WarningSelectValue;
  // 串口上传一次错误信息到jb-app，避免重复上传导致app崩溃
  static uint8_t WarningSelectValueTemp = 0;

  if (WarningSelectValueTemp != WarningSelectValue)
  {
    USER_EchoLogStr("WarningSelectValue:%d\r\n", WarningSelectValue); //串口上传信息到上位机2017.7.6
    WarningSelectValueTemp = WarningSelectValue;
  }

  switch (WarningSelectValue)
  {
  case MinTempWarning:
    if (Pop == MinTempWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case MaxTempWarning:
    if (Pop == MaxTempWarningPopSet)
    {
      IsPop = Pop;
      temperature_disable_heater();
    }

    break;

  case MaxTempBedWarning:
    if (Pop == MaxTempBedWarningPopSet)
    {
      IsPop = Pop;
      temperature_disable_heater();
    }

    break;

  case HeatFailWarning:
    if (Pop == HeatFailWarningPopSet)
    {
      IsPop = Pop;

      if (IsPrint()) // 加热失败，处于打印状态时，停止打印
      {
        respond_gui_send_sem(StopPrintValue);
      }
      else
      {
        temperature_disable_heater();
      }
    }

    break;

  case XMinLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case YMinLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case ZMinLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case XMaxLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case YMaxLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case ZMaxLimitWarning:
    if (Pop == MachineSizeLimitWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case IWDGResetWarning:
    if (Pop == IWDGResetWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case FatfsWarning:
    if (Pop == FatfsFailWarningPopSet)
    {
      IsPop = Pop;
    }

    break;

  case ThermistorFallsWarning:
    if (Pop == ThermistorFallsWarnPopSet)
    {
      IsPop = Pop;

      if (IsPrint()) // 加热失败，处于打印状态时，停止打印
      {
        respond_gui_send_sem(StopPrintValue);
      }
      else
      {
        temperature_disable_heater();
      }
    }

    break;

  default:
    break;
  }

  if (Pop == IsPop)
  {
    IsWarning = 1;
  }
}

void ManagWarningInfo(void)
{
  switch (WarningInfoSelect)
  {
  case MinTempWarning:
    MinTempWarningPopSet = NotPop;
    break;

  case MaxTempWarning:
    MaxTempWarningPopSet = Pop;
    break;

  case MaxTempBedWarning:
    MaxTempBedWarningPopSet = Pop;
    break;

  case HeatFailWarning:
    HeatFailWarningPopSet = Pop;
    break;

  case XMinLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case YMinLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case ZMinLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case XMaxLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case YMaxLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case ZMaxLimitWarning:
    MachineSizeLimitWarningPopSet = Pop;
    break;

  case IWDGResetWarning:
    IWDGResetWarningPopSet = NotPop;
    break;

  case FatfsWarning:
    FatfsFailWarningPopSet = Pop;
    break;

  case ThermistorFallsWarning:
    ThermistorFallsWarnPopSet = Pop;
    break;

  default:
    break;
  }

  IsWarning = 0;
}


#ifdef __cplusplus
} //extern "C" {
#endif

