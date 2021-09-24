#include "start_default_task.h"
#include "user_common.h"
#include "machinecustom.h"
#include "controlfunction.h"

#include "machine_custom.h"
#include "UdiskControl.h"
#include "config_model_tables.h"
#include "USBFileTransfer.h"
#include "globalvariables.h"
#include "machine_model.h"
#include "flashconfig.h"
#include "user_ccm.h"
#include "gcode.h"
#include "controlxyz.h"
#include "touch.h"
#include "flashconfig.h"
#include "sysconfig_data.h"
#ifdef __cplusplus
extern "C" {
#endif
extern osSemaphoreId ReceiveUartCmdHandle;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

void start_default_task_init(void)
{
  flash_read_data();
  // 加载系统配置文件
  sysconfig_init();
  // 机器型号初始化
  machine_model_init();
  gcode::init();

  // 控制功能初始化
  control_function_init();

  machine_custom_init();
  //删除SD卡里的gcode文件
  DelectSDFiles();

  for (int i = 0; i < 3; i ++)
  {
    t_gui.move_xyz_pos[i] = ccm_param::motion_3d_model.xyz_home_pos[i];
  }

  control_xyz_init();
}



void start_default_task_loop(void)
{
  (void)osSemaphoreWait(ReceiveUartCmdHandle, osWaitForever); // 等待信号量
  (void)OS_DELAY(task_schedule_delay_time); // 延时以让低优先级的任务执行
  SaveUSBFile(); // 存储从电脑端发过来的文件
}


#ifdef __cplusplus
} // extern "C" {
#endif

