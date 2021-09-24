#include "controlfunction.h"
#include "filamentcontrol.h"
#include "boardtest.h"
#include "poweroffrecovery.h"
#include "guicontrol.h"
#include "gcodebufferhandle.h"
#include "temperature.h"
#include "stepper.h"
#include "planner.h"
#include "user_common.h"
#include "config_model_tables.h"
#include "globalvariables.h"
#include "stm32f4xx_hal.h"

#include "process_m_code.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#ifdef __cplusplus
extern "C" {
#endif

extern int IsPrint(void);

// 门检测
void control_check_door_open(void)
{
  if (ccm_param::motion_3d.enable_check_door_open) // 有门检测功能，开启门检测
  {
    if (user_pin_sig_door_read())  //M14R03，M14S 门检测占用了ZMAX PIN接口
    {
      //door open
      IsDoorOpen = 1;
      doorStatus = 1;
    }
    else
    {
      //door close
      IsDoorOpen = 0;
      doorStatus = 0;
    }
  }
}

static void poweroff_init(void)
{
  if (t_sys_data_current.enable_powerOff_recovery)
  {
    PowerOffRecovery *powerOffRecovery = new PowerOffRecovery();
    powerOffRecovery->init();
    powerOffRecovery->initGlobalVariables();
    #if (1 == DEBUG_POWEROFF_CUSTOM)
    USER_DbgLog("%40s = %d", "PowerOffRecovery Object Size", sizeof(*powerOffRecovery));
    #endif // DEBUG_POWEROFF_CUSTOM
    delete powerOffRecovery;
    powerOffRecovery = NULL;
  }
}

void control_function_init(void)
{
  poweroff_init();
  #if (1 == DEBUG_FUNCTION_CUSTOM)
  USER_DbgLog("%40s = %d", "PowerOffOperation Object Size", sizeof(powerOffOperation));
  USER_DbgLog("%40s = %d", "PowerOffUpDownMinMin Object Size", sizeof(powerOffUpDownMinMin));
  USER_DbgLog("%40s = %d", "BoardTest Object Size", sizeof(boardTest));
  USER_DbgLog("%40s = %d", "BlockDetect Object Size", sizeof(blockDetect));
  USER_DbgLog("%40s = %d", "MaterialCheck Object Size", sizeof(materialCheck));
  #endif
  //  // 设置gcode数组是否需要解密
  //  gcodeBufHandle.init((1==t_sys_data_current.enable_color_mixing && 1 == t_sys_data_current.have_set_machine));
}

void control_function_process(void)
{
  if (!IsPrint())    // 串口打印和U盘打印情况下不执行下面函数
  {
    filamentControl.process();                 // 进退丝操作入口

    if (ccm_param::motion_3d.enable_board_test)   // 测试固件入口
    {
      boardTest.process();
    }

    //    powerOffOperation.getZMaxPos();            // 校准z高度入口
  }

  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  guiControl.refreshGuiInfo();
  (void)OS_DELAY(task_schedule_delay_time * 3);                      // 延时以让低优先级的任务执行
  powerOffOperation.recoveryPrint();           // 断电恢复入口
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  feature_fan_control(); // 电机风扇控制
  control_check_door_open();
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  feature_led_control();                      // 灯控制入口
  (void)OS_DELAY(task_schedule_delay_time);
  feature_filament_check();                     // 断料检测入口
  (void)OS_DELAY(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  user_buzzer_beep_alarm();
}

void TIM3_IRQHandler_process(void)
{
  temperature_update();
}

void TIM4_IRQHandler_process(void)
{
  st_process();
}

#ifdef __cplusplus
} // extern "C" {
#endif






