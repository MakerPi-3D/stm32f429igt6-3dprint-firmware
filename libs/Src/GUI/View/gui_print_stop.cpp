#include "common.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif

bool IsPrintSDFile(void);
void stopprintF(void)
{
  char printnameb[_MAX_LFN];
  unsigned int length;

  if (gui::is_refresh())
  {
    display_picture(7);
    {
      strcpy(printnameb, printname);

      if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0))
      {
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
      }
    }
    length = strlen(printnameb);
    DisplayText((uint8_t *)printnameb, (int)((t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - (length / 2) * 12), 105, 24, (u16)testcolor); //240-(length/2)*12    是为了让文字显示在中间
  }

  if (touchxy(60, 200, 240, 285))
  {
    waiting_for_stopping();
		flash_erase_poweroff_data();
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

  if ((pauseprint && print_flage) || (ChangeFilamentHeatStatus == 0)) //打印且暂停了，或加热还没完成，拔出U盘则停止打印且返回主界面
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

#ifdef __cplusplus
} //extern "C" {
#endif


