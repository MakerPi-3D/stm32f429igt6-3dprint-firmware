#include "common.h"
#include "machinecustom.h"
#include "commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#define PowerOffRecoverWarningTime 5000

#ifdef __cplusplus
extern "C" {
#endif

static unsigned long PowerOffRecoverWarningTimeout;

extern void MainInterfaceDisplayInit(void);
extern void MainInterfaceDisplayText(void);
char poweroffrunonce;

void PowerOffRecoverReady(void)
{
  if (gui::is_refresh())
  {
    if (t_custom_services.disable_hot_bed)
    {
      display_picture(53);
    }
    else
    {
      display_picture(49);
    }

    MainInterfaceDisplayInit();
    MainInterfaceDisplayText();

    if (poweroffrunonce)
    {
      respond_gui_send_sem(CloseBeep);
      respond_gui_send_sem(ConfirmPowerOffRecover);
      poweroffrunonce = 0;
    }

    pauseprint = 0;
    print_flage = 1;//2017511在按下确定断电续打时标志，防止加热失败仍继续打印
  }

  if (touchxy(0, 0, 240, 65))
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(SetPowerOffRecoverNozzleTemp_dual);
    }
    else
    {
      gui::set_current_display(SetPowerOffRecoverNozzleTemp);
    }

    return;
  }

  if (!t_custom_services.disable_hot_bed) //NotM14_Right
  {
    if (touchxy(240, 0, 480, 65))
    {
      gui::set_current_display(SetPowerOffRecoverHotBedTemp);
      return;
    }
  }

  if (IsFinishedPowerOffRecoverReady)
  {
    IsFinishedPowerOffRecoverReady = 0;
    //    pauseprint=0;
    //    print_flage = 1;//2017511在按下确定断电续打时标志，防止加热失败仍继续打印
    gui::set_current_display(maindisplayF);
  }

  if (gui::is_refresh_rtc())
  {
    MainInterfaceDisplayText();
  }
}

// 设置断电续打超时时间为60s，超过60s直接恢复打印
static unsigned long power_off_recover_tick_count = 0;
static const unsigned long POWER_OFF_RECOVER_TIME_OUT = 1000 * 60;

void PowerOffRecover(void)
{
  char printnameb[_MAX_LFN];
  unsigned int length;

  if (0 == power_off_recover_tick_count)
    power_off_recover_tick_count = xTaskGetTickCount();

  if (gui::is_refresh())
  {
    display_picture(45);
    strcpy(printnameb, printname);

    if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
    }

    length = strlen(printnameb);
    DisplayText((uint8_t *)printnameb, (int)(240 - (length / 2) * 12), 105, 24, (u16)testcolor); //240-(length/2)*12   是为了让文字显示在中间
    respond_gui_send_sem(OpenBeep);
    PowerOffRecoverWarningTimeout = xTaskGetTickCount() + PowerOffRecoverWarningTime;
  }

  if ((power_off_recover_tick_count + POWER_OFF_RECOVER_TIME_OUT <= xTaskGetTickCount()) || touchxy(60, 200, 240, 285)) //确认键
  {
    poweroffrunonce = 1;
    gui::set_current_display(PowerOffRecoverReady);
    return ;
  }

  if (touchxy(240, 200, 425, 285)) //取消键
  {
    flash_erase_poweroff_data();
    respond_gui_send_sem(CloseBeep);
    OS_DELAY(50);
    respond_gui_send_sem(CancelPowerOffRecover);
    gui::set_current_display(maindisplayF);
    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (PowerOffRecoverWarningTimeout < xTaskGetTickCount())
    {
      respond_gui_send_sem(CloseBeep);
    }
  }
}


#ifdef __cplusplus
} //extern "C" {
#endif

