#include "respond_gui_task.h"
#include "config_model_tables.h"
#include "interface.h"
#include "user_common.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
#include "UdiskControl.h"
#include "PrintControl.h"
#include "controlfunction.h"
#include "midwaychangematerial.h"
#include "globalvariables.h"
#include "machinecustom.h"
#include "boardtest.h"
#include "RespondGUI.h"
#include "process_m_code.h"
#include "functioncustom.h"
#include "user_ccm.h"
#include "gcode.h"
#include "common.h"
#include "sysconfig_data.h"
#ifdef __cplusplus
extern "C" {
#endif

extern osSemaphoreId GUISendSemHandle;
extern osSemaphoreId GUIWaitSemHandle;

void ScanGUISemStatus(void)
{
  switch (SettingInfoToSYS.GUISempValue)
  {
  case BackZeroValue:  //回零
    USER_DbgLog("BackZero ok!");

    if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
        flash_param_t.extruder_type == EXTRUDER_TYPE_LASER ||
        flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      user_send_internal_cmd((char *)"T0 S-1");
      user_send_internal_cmd((char *)"G28");      // XYZ归零
    }
    else
    {
      user_send_internal_cmd((char *)"G28");
    }

    for (int i = 0; i < 3; i ++)
      t_gui.move_xyz_pos[i] = ccm_param::motion_3d_model.xyz_home_pos[i];

    break;

  case DisableStepValue:  //解锁电机
    guiControl.disableSteppers();
    break;

  case PreHeatPLAValue:  //预热PLA
    guiControl.preHeatPLA();
    break;

  case CoolDownValue:  //冷切
    USER_DbgLog("CoolDown ok!");
    guiControl.coolDown();
    break;

  case FeedFilamentValue:  //进料
    filamentControl.startLoad();
    break;

  case StopFeedFilamentValue:  //退出进料
    //      USER_EchoLogStr("DEBUG: StopFeedFilament ok!\r\n");
    filamentControl.cancelProcess();
    break;

  case BackFilamentValue:  //退料
    filamentControl.startUnload();
    break;

  case StopBackFilamentValue:  //退出退料
    //      USER_EchoLogStr("DEBUG: StopBackFilament ok!\r\n");
    filamentControl.cancelProcess();
    break;

  case OpenSDCardValue:  //打开SD卡
    USER_DbgLog("OpenSDCard ok!");
    GUIOpenSDCard();
    break;

  case OpenDirValue:  //打开目录
    USER_DbgLog("OpenDir ok!");
    GUIOpenSDDir();
    break;

  case BackLastDirValue:  //返回上层目录
    USER_DbgLog("BackLastDir ok!");
    GUIBackSDLastDir();
    break;

  case NextPageValue:  //下一页
    USER_DbgLog("NextPage ok!");
    GUINextPage();
    break;

  case LastPageValue:  //上一页
    USER_DbgLog("LastPage ok!");
    GUILastPage();
    break;

  case FilePrintValue:  //选中文件确定打印
    USER_DbgLog("FilePrint ok!");
    printControl.start();
    resetM109HeatingComplete();  //打印时重置为还没有加热完成
    IsCompleteHeat(); //解决一开始就跳转到有中途换料的界面的问题
    break;

  case PausePrintValue:  //暂停打印
    USER_DbgLog("PausePrint ok!");
    pauseprint = 1;
    printControl.pause();
    break;

  case ResumePrintValue:  //继续打印
    USER_DbgLog("ResumePrint ok!");
    pauseprint = 0;
    printControl.resume();
    break;

  case StopPrintValue:  //停止打印
    USER_DbgLog("StopPrint ok!");
    print_flage = 0;
    pauseprint = 0;
    guiControl.coolDown();
    printControl.stop();
    break;

  case OpenBeep:  //打开蜂鸣器
    USER_DbgLog("OpenBeep ok!");

    if (t_sys.alarm_sound)
      user_buzzer_set_alarm_status(true);

    break;

  case CloseBeep:  //关闭蜂鸣器
    USER_DbgLog("CloseBeep ok!");
    user_buzzer_set_alarm_status(false);
    break;

  case SysErrValue:  //系统错误
    ManagWarningInfo();
    break;

  case PrintSetValue_M14:  //M14机型 打印设置
    guiControl.printSetForM14();
    break;

  case PreHeatABSValue:  //预热ABS
    guiControl.preHeatABS();
    //    GUI_PreHeatABS();
    break;

  case MoveXYZValue:  //移动光轴确定键
    guiControl.moveXYZ(t_gui.move_xyz_pos);
    //    GUI_MoveXYZ(t_gui.move_x_pos,t_gui.move_y_pos,t_gui.move_z_pos);
    user_send_internal_cmd((char *)"M2004 S1");
    break;

  case ConfirmChangeFilamentValue:  //确认中途换料
    ChangeFilament();
    break;

  case ConfirmLoadFilamentValue:  //中途换料中确认进料
    USER_EchoLogStr("ConfirmLoadFilamentValue ok!\r\n");//串口上传信息到上位机2017.7.6
    ConfirmLoadFilament();
    break;

  case ConfirmChangeModelValue:  //更改机型
    USER_EchoLogStr("ConfirmChangeModelValue ok!\r\n");//串口上传信息到上位机2017.7.6
    SaveSelectedModel();
    break;

  case ConfirmChangePictureValue:  //
    SaveSelectedPicture();
    break;

  case ConfirmChangeFunctionValue:  //更改功能
    //      USER_EchoLogStr("ConfirmChangeFunctionValue ok!\r\n");//串口上传信息到上位机2017.7.6
    SaveSelectedFunction();
    break;

  case PrintSetValue_NotM14_Left:  //非M14机型 点击左上方 打印设置
    guiControl.printSetForNotM14Left();
    break;

  case PrintSetValue_NotM14_Right:  //非M14机型 点击右上方 打印设置
    guiControl.printSetForNotM14Right();
    break;

  case ConfirmPowerOffRecover:  //断电续打确认键
    poweroff_ready_to_recover_print();
    break;

  case CancelPowerOffRecover:  //断电续打取消
    poweroff_delete_file_from_sd();
    //    poweroff_reset_flag(); //重置标志位
    break;

  case CalculateZMaxPos:  //测量行程
    poweroff_start_cal_z_max_pos();
    break;

  case MatCheckCalibrateValue:  //断料检测模块校准
    break;

  case PauseToResumeNozzleTemp: //断料状态下去换料，先把暂停打印降低的温度恢复
    USER_EchoLogStr("PauseToResumeTemp ok!\r\n");//串口上传信息到上位机2017.7.6
    //PauseToResumeNozTemp();
    PauseToResumeTemp();
    break;

  case StepTestValue:  //电机测试
    boardTest.toggleStepStatus();
    break;

  case FanTestValue:  //风扇测试
    boardTest.toggleFanStatus();
    break;

  case HeatTestValue:  //加热测试
    boardTest.toggleHeatStatus();
    break;

  case RunMaxPos:  //加热测试
    boardTest.runMaxPos();
    break;

  case CalHeatTime:  //加热計時
    boardTest.calHeatTime();
    break;

  case ConfirmChangelogoValue:  //确定logo设置
    SaveSelectedlogo();
    break;

  case PreHeatBedValue:
    guiControl.preHeatBed();
    break;

  case PrintSetValue_Cavity:
    guiControl.printSetForCavity();
    break;
    #ifdef CAL_Z_ZERO_OFFSET

  case StartCalZZero:
    isCalZero = true;
    break;

  case CancelCalZZero:
    SaveZOffsetZero() ;
    isCalZero = false;
    break;

  // 校准平台接口
  case StartCalBedLevel:
    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    {
      if (!gcode::g28_complete_flag)
        user_send_internal_cmd((char *)"G28");

      user_send_internal_cmd((char *)"G29");
    }

    break;
    #endif

  case ResetEBValue:
    user_send_internal_cmd((char *)"G92 E0 B0");
    break;

  case SwitchLaser:
    user_send_internal_cmd((char *)"T1 S-1");
    user_send_internal_cmd((char *)"G92 E0 B0");
    break;

  case LaserMoveZUp:
    user_send_internal_cmd((char *)"G1 F300 Z10");
    break;

  case IdexMoveX:
    char buf[30];

    if (gui_respond_data.active_extruder == 0)
    {
      user_send_internal_cmd((char *)"T0 S-1");
      (void)snprintf(buf, sizeof(buf), "G1 F1200 X%d", gui_respond_data.x_move_value);
      user_send_internal_cmd((char *)buf);
    }
    else if (gui_respond_data.active_extruder == 1)
    {
      user_send_internal_cmd((char *)"T1 S-1");
      (void)snprintf(buf, sizeof(buf), "G1 F1200 X%d", gui_respond_data.x_move_value);
      user_send_internal_cmd((char *)buf);
      user_send_internal_cmd((char *)"T0 S-1");
    }

    break;

  default:
    USER_DbgLog("GUISempValue is not effective Value");
  }

  (void)osSemaphoreRelease(GUIWaitSemHandle);
}


void respond_gui_task_loop(void)
{
  if (osSemaphoreWait(GUISendSemHandle, osWaitForever) == osOK) //GUI信号量处理
  {
    ScanGUISemStatus();
    (void)OS_DELAY(task_schedule_delay_time);
  }
}


#ifdef __cplusplus
} // extern "C" {
#endif



