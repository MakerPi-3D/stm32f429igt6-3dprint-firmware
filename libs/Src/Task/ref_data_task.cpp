#include "ref_data_task.h"
#include "config_model_tables.h"
#include "temperature.h"
#include "machinecustom.h"
#include "PrintControl.h"
#include "midwaychangematerial.h"
#include "process_command.h"
#include "RespondGUI.h"
#include "interface.h"
#include "process_m_code.h"
#include "controlfunction.h"
#include "globalvariables.h"
#include "user_common.h"
#include "USBFileTransfer.h"
#include "jshdisplay.h"
#include "flashconfig.h"
#include "user_fan.h"
#include "sysconfig_data.h"
#ifdef __cplusplus
extern "C" {
#endif

void UpdateGUIInfo(void)
{
  // RefreshGUI
  t_gui.printed_time_sec = printControl.getTime();
  t_gui.print_percent = printFileControl.getPercent();
}

//电路板问题，20170502
//有些电路板有问题，导致系统上电就会自动加热，并且加热功率为最大，如果不断电则后果严重

#if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
// 检测打印状态下，热敏电阻状态
void check_print_extruder_thermistor_status(void)
{
  // 打印状态且已经加热完成，温度突然下降10度，判断热敏电阻脱落，需要测试
  static int extruder_tmp_save_status = 0;
  static int extruder_tmp_save = 0;
  static int extruder_tmp_target_save = 0;
  static unsigned long extruder_tmp_save_peroid = 0;
  static int extruder_tmp_save_time_count = 0;

  // 只有在打印状态，且执行M109指令完成才进行以下操作
  if (IsPrint() && isM109HeatingComplete())
  {
    // 目标温度与当前温度差值小于3，开启检测热敏电阻状态
    if (0 == extruder_tmp_save_status && abs((int)(temperature_get_extruder_target(0) - temperature_get_extruder_current(0))) < 3)
    {
      extruder_tmp_save_status = 1;
      extruder_tmp_save = temperature_get_extruder_current(0);
      extruder_tmp_target_save = temperature_get_extruder_target(0);
    }

    // 打印过程中更新目标温度，检测状态重新设置
    if (extruder_tmp_target_save != temperature_get_extruder_target(0))
    {
      extruder_tmp_save_status = 0;
      extruder_tmp_target_save = temperature_get_extruder_target(0);
      return;
    }
  }
  else
  {
    extruder_tmp_save_status = 0;
  }

  if (extruder_tmp_save_status)
  {
    // 秒数累加
    if (extruder_tmp_save_peroid < MILLIS())
    {
      extruder_tmp_save_peroid = MILLIS() + 1000;
      extruder_tmp_save_time_count++;
    }

    // 5秒内记录的温度比当前温度大15度，判断热敏电阻脱落导致此温度差
    if (extruder_tmp_save - temperature_get_extruder_current(0) >= 18 && extruder_tmp_save_time_count == 5)
    {
      PopWarningInfo(ThermistorFallsWarning);
      extruder_tmp_save_time_count = 0;
    }
    else
    {
      if (extruder_tmp_save_time_count > 5)
      {
        extruder_tmp_save_time_count = 0;
      }
    }
  }
}
#endif // #if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF

/*
 * 当温度超过最大温度时就发生警报，因温度最大只能是HEATER_0_MAXTEMP-25
 * 已经注释的代码不稳定，经常乱报警，所以方案不可行
 * 2017/8/22
*/
void temperature_error_pop(void)
{
  if (DETECT_PCB_FAULSE == temperature_get_error_status())
  {
    display_picture(95);//警告界面
    //      gui_send_sem_open_beep();
    respond_gui_send_sem(OpenBeep);
  }
  else if (MaxTempBedError == temperature_get_error_status())
  {
    PopWarningInfo(MaxTempBedWarning);
  }
  else if (MinTempError == temperature_get_error_status())
  {
    if (P3_Pro == t_sys_data_current.model_id)
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        extern uint8_t MinTempWarningPopSet;
        extern uint8_t HeatFailWarningPopSet;
        MinTempWarningPopSet = 1;
        HeatFailWarningPopSet = 1;
      }
    }

    PopWarningInfo(MinTempWarning);
  }
  else if (MaxTempError == temperature_get_error_status())
  {
    PopWarningInfo(MaxTempWarning);
  }
  else if (HeatFailError == temperature_get_error_status())
  {
    PopWarningInfo(HeatFailWarning);
  }
  else if (ThermistorFallsOffError == temperature_get_error_status())
  {
    PopWarningInfo(ThermistorFallsWarning);
  }

  #if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF

  if (1 == t_sys.is_detect_extruder_thermistor)
  {
    check_print_extruder_thermistor_status();
  }

  #endif // #if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
}

void manage_synchronize()
{
  temperature_manage_heater(feature_get_extruder_fan_speed());   // 加热控制
  manage_inactivity();           // 打印相关控制
  temperature_error_pop();       // 检测是否电路板有问题，导致喷嘴一直加热
  //    if(temperature_update_pid_output_factor())
  //    {
  //      SavePidOutputFactorValue();
  //    }
}

void UpdatePrintStatus(void)
{
  RefChangeFilamentStatus();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  manage_synchronize();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  // 停止打印乱跑，过滤乱跑指令
  printControl.processStop();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
}

void UpdateNoPrintStatus(void)
{
  (void)OS_DELAY(50);  //延时以让低优先级的任务执行
  flash_param_process();
  (void)OS_DELAY(50);  //延时以让低优先级的任务执行
  user_usb_host_update_status();
  printControl.processPause();
  (void)OS_DELAY(50);  //延时以让低优先级的任务执行
  printControl.processResume();
  (void)OS_DELAY(50);  //延时以让低优先级的任务执行
  user_usb_device_trans_file_err(); //传输文件是否错误，比如传输中突然拔线，采用的是超时检测
}

//void my_printf_test(void)
//{
//  static unsigned long RefreshGuiTimeOut = 0;
//  static unsigned count = 0;

//  if (mcu_id == MCU_GD32F450IIH6)
//  {
//    if (RefreshGuiTimeOut < xTaskGetTickCount())
//    {
//      USER_DbgLog("count=%d", count++);
//      RefreshGuiTimeOut = xTaskGetTickCount() + 100;
//    }
//  }
//}

void ref_data_task_loop(void)
{
  user_iwdg_refresh();
  UpdatePrintStatus();                      // 更新打印状态

  if (!IsPrint()) // 串口打印和U盘打印情况下不执行下面函数
    UpdateNoPrintStatus();                  // 更新GUI相关数据

  //    if(!serialPrintStatus())                  // 串口打印没打开时，更新界面gui数据
  UpdateGUIInfo();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  control_function_process();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  //  my_printf_test();
//  user_os_print_task_remaining_space();
}

#ifdef __cplusplus
} // extern "C" {
#endif







