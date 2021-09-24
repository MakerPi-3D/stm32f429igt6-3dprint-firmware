#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "user_common.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

static uint8_t IsNotHaveMatToChangeFilament = 0;

void ChangeFilamentStatus(void)
{
  static uint8_t RefreshOnce = 1;
  static uint8_t LastStatus;

  if (M600FilamentChangeStatus < M600_STATUS_BEEP_NOTICE)
  {
    if ((1 == RefreshOnce) || (gui::is_refresh()))
    {
      display_picture(21);
      RefreshOnce = 3;
      LastStatus = 0;
    }
  }
  else if (M600FilamentChangeStatus == M600_STATUS_BEEP_NOTICE)
  {
    if ((3 == RefreshOnce) || (gui::is_refresh()))
    {
      display_picture(22);
      RefreshOnce = 4;
    }

    if (touchxy(150, 200, 330, 300))
    {
      respond_gui_send_sem(ConfirmLoadFilamentValue);
    }
  }
  else if (M600FilamentChangeStatus == M600_STATUS_RESTORE_POS)
  {
    if ((4 == RefreshOnce) || (gui::is_refresh()))
    {
      display_picture(20);
      RefreshOnce = 5;
    }
  }
  else if (M600FilamentChangeStatus == M600_STATUS_FINISH)
  {
    if (IsNotHaveMatToChangeFilament)
      gui::set_current_display(NotHaveMatControlInterface2);
    else
      gui::set_current_display(maindisplayF);

    RefreshOnce = 1;
    respond_gui_send_sem(CloseBeep);
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
}

bool IsPrintSDFile(void);

void ChangeFilamentConfirm(void)
{
  char printnameb[_MAX_LFN];
  unsigned int length;

  if (gui::is_refresh())
  {
    display_picture(19);
    strcpy(printnameb, printname);

    if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
    }

    length = strlen(printnameb);
    DisplayText((uint8_t *)printnameb, (int)((t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - (length / 2) * 12), 105, 24, (u16)testcolor); //240-(length/2)*12    是为了让文字显示在中间
  }

  if (touchxy(60, 200, 240, 285))
  {
    //    gui_send_sem_confirm_chg_filament();
    respond_gui_send_sem(ConfirmChangeFilamentValue);
    gui::set_current_display(ChangeFilamentStatus);
    return ;
  }

  if (touchxy(240, 200, 425, 285))
  {
    gui::set_current_display(maindisplayF);
    return ;
  }

  if (IsNotHaveMatInPrint) //打印中检测到没有材料
  {
    gui::set_current_display(NoHaveMatWaringInterface);
    return ;
  }

  if (pauseprint && print_flage) //打印且暂停了，拔出U盘则停止打印且返回主界面
  {
    if (!user_usb_host_is_mount() && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      waiting_for_stopping();
      return ;
    }
  }

  if ((IsDoorOpen) && pauseprint == 0 && print_flage) //打印且没有暂停
  {
    gui::set_current_display(DoorOpenWarningInfo_Printing);
    return ;
  }
}

void NotHaveMatToChangeFilament(void)
{
  IsNotHaveMatToChangeFilament = 1;
  ChangeFilamentStatus();
  IsNotHaveMatToChangeFilament = 0;
}

#ifdef __cplusplus
} //extern "C" {
#endif




