#include "midwaychangematerial.h"
#include "globalvariables.h"
#include "user_common.h"
#include "gcodebufferhandle.h"
#include "controlfunction.h"
#include "process_command.h"
#include "process_m_code.h"
#include "config_motion_3d.h"
#include "user_ccm.h"
#include "gcode.h"
#include "flashconfig.h"
#include "sysconfig_data.h"
#include "PrintControl.h"
#include "planner.h"
#include "stepper.h"
#ifdef __cplusplus
extern "C" {
#endif



#define NotStartChangeFilament 0
#define StartChangeFilament 1
#define HeatNotComplete 0
#define HeatComplete 1

static int IsStartChangeFilament = NotStartChangeFilament;
void setConfirmLoadFilament(int _IsConfirmLoadFilament);
//  extern uint8_t GetM109HeatingStatus(void);

void RefM600FilamentChangeStatus(void)
{
  M600FilamentChangeStatus = gcode::m600_status;
}

void ConfirmLoadFilament(void)
{
  USER_EchoLogStr("ConfirmLoadFilamentValue ok!\r\n");//串口上传信息到上位机2017.7.6
  setConfirmLoadFilament(1);
}


void IsCompleteHeat(void)
{
  if (isM109HeatingComplete())
  {
    ChangeFilamentHeatStatus = HeatComplete;
  }
  else
  {
    ChangeFilamentHeatStatus = HeatNotComplete;
  }
}

void ChangeFilament(void)
{
  uint8_t print_status = 0;

  if (IsPrint())
  {
    print_status = 1;
    SetPrintStatus(false);  //解决多次中途换料后，打印乱跑现象
  }
  else if (IsPausePrint())
  {
    print_status = 2;
    SetPausePrintingStatus(false); // 解决暂停打印后中途换料，中途换料时，暂停后续操作同时进行（目标温度下降为目标温度一半），导致进丝异常
  }

  M600FilamentChangeStatus = gcode::m600_status;
  IsStartChangeFilament = StartChangeFilament;
  // 打开中途换料标志位
  setMidWayChangeMat(true);

  if (IsNotHaveMatInPrint) //断料续打中的换料发送M601
  {
    user_send_internal_cmd((char *)"M601");   //换料
  }
  else
  {
    if (0 == print_status)
      user_send_internal_cmd((char *)"M600 S0");   //不是打印状态下换料，不会执行该逻辑

    if (1 == print_status)
      user_send_internal_cmd((char *)"M600 S1");   //正常打印下换料

    if (2 == print_status)
      user_send_internal_cmd((char *)"M600 S2");   //暂停打印下换料
  }
}

void RefChangeFilamentStatus(void)
{
  IsCompleteHeat();

  if (StartChangeFilament == IsStartChangeFilament)
  {
    M600FilamentChangeStatus = gcode::m600_status;

    if (M600_STATUS_FINISH == M600FilamentChangeStatus)
    {
      IsStartChangeFilament = NotStartChangeFilament;
      gcode::m600_status = 0;
      setConfirmLoadFilament(0);
    }
  }
}


#ifdef __cplusplus
} // extern "C" {
#endif


