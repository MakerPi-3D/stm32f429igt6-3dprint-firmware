#include "read_udisk_task.h"
#include "config_model_tables.h"
#include "interface.h"
#include "PrintControl.h"

#ifdef __cplusplus
extern "C" {
#endif

extern osSemaphoreId ReadUdiskSemHandle;

void read_udisk_task_loop(void)
{
  (void)osSemaphoreWait(ReadUdiskSemHandle, osWaitForever); //信号量等待

  if (IsPrint() && !IsPausePrint()) //若打印才去SD卡或U盘中读取数据
  {
    if (SettingInfoToSYS.GUISempValue != StopPrintValue) //防止点击停止打印后，还去读取数据的情况，此种情况是由于RefDataTask_OS高优先级任务中的osDelay(10);引起的，此时是否打印标志位还没更新
    {
      readGcodeBuf(); //从SD卡或U盘中读取数据，当上传文件打印时是从SD卡读取，当触摸显示屏选择文件打印时是从U盘读取
      (void)OS_DELAY(10); //延时以让低优先级的任务执行
    }
  }
}



#ifdef __cplusplus
} // extern "C" {
#endif












