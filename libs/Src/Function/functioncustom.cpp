#include "functioncustom.h"
#include "poweroffrecovery.h"
#include "globalvariables.h"
#include "boardtest.h"
#include "user_common.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
#include "stepper_pin.h"
//#include "ConfigurationStore.h"
#include "process_m_code.h"
#include "sysconfig_data.h"
//static PowerOffOperation powerOffOperation;
//static PowerOffUpDownMinMin powerOffUpDownMinMin;
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "temperature.h"
#include "PrintControl.h"

//extern uint8_t serialPrintStatus(void);

//__attribute__((section("ccmram")))
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//  if (GPIO_Pin == GPIO_PIN_3 && t_sys_data_current.enable_powerOff_recovery) // PD3
	if (GPIO_Pin == GPIO_PIN_3)
  {
    // 重读pd3，如果高电平，干扰导致断电续打，退出
//    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET)
//      return;

//    __NOP();

//    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET)
//      return;

//    __NOP();

//    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_SET)
//      return;

    if (IsPrint())
    {
 //     temperature_disable_heater();

      for (int i = 0; i < MAX_NUM_AXIS; i++)
      {
        stepper_axis_enable(i, false);
      }

      t_power_off.is_power_off = 1; // 断电状态，更新标志
      powerOffOperation.syncData();
    }

    HAL_NVIC_SystemReset(); //复位
  }
}

void poweroff_start_cal_z_max_pos(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.startCalculateZMaxPos();
}

void poweroff_stop_cal_z_max_pos(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.stopGetZMaxPos();
}

void poweroff_reset_flag(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.resetFlag();
}

void poweroff_set_file_path_name(const char *filePathName)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.setFilePathName(filePathName);
}

void poweroff_set_data(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.setData();
}

void poweroff_ready_to_recover_print(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.readyToRecoveryPrint();
}

void poweroff_delete_file_from_sd(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  powerOffOperation.deleteFileFromSD();
  powerOffOperation.resetFlag();
}

/////////////////////////////////////PowerOff end/////////////////////
void board_test_display_function(void)
{
  if (!ccm_param::motion_3d.enable_board_test)
    return;

  if (boardTest.guiInterface())
    return ;
}

void board_test_model_select(void)
{
  if (!ccm_param::motion_3d.enable_board_test)
    return;

  boardTest.modelSelect();
}

void board_test_cal_heat_time_gui(void)
{
  if (!ccm_param::motion_3d.enable_board_test)
    return;

  if (boardTest.calHeatTimeGui())
    return ;
}

void board_test_cal_touch_count(void)
{
  //  if(!motion_3d.enable_board_test)
  //    return;
  if (boardTest.ERRTouchCount())
    return ;
}
void board_test_pressure(void)
{
  //  if(!motion_3d.enable_board_test)
  //    return;
  boardTest.PressureTest();
  return ;
}


#ifdef __cplusplus
} //extern "C" {
#endif

