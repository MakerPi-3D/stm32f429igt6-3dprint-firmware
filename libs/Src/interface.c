#include "interface.h"
#include "cmsis_os.h"
//ȫ���ź���
extern osSemaphoreId GUISendSemHandle;
extern osSemaphoreId GUIWaitSemHandle;

SettingInfo SettingInfoToSYS;

void respond_gui_send_sem(const int sempValue)
{
  SettingInfoToSYS.GUISempValue = sempValue;
  (void)osSemaphoreRelease(GUISendSemHandle);
  (void)osSemaphoreWait(GUIWaitSemHandle, osWaitForever);
  osDelay(50);
}

