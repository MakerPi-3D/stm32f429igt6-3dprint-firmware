#include "gui_task.h"
#include "config_model_tables.h"
#include "process_m_code.h"
#include "user_common.h"
#include "touch.h"
#include "jshdisplay.h"
#include "globalvariables.h"
#include "PrintControl.h"
#include "interface.h"
#include "commonf.h"
#include "user_board_pin.h"
#include "sysconfig_data.h"
#ifdef __cplusplus
extern "C" {
#endif
//extern void LCD_Init(void);
//extern void board_test_display_function(void);//品质测试时上电直接进入测试界面
void PowerOntoRunOnce(void) //上电执行一次
{
  static uint8_t OnlyRunOnce = 1;

  if (OnlyRunOnce)
  {
    OnlyRunOnce = 0;
    (void)OS_DELAY(100);//需要延时一点时间，否则SD卡不知为什么会读写异常
    (void)tp_dev.init();//触摸初始化

    /**开机LOGO界面************************************************************/
    if (t_sys_data_current.logo_id)
    {
      DisplayLogoPicture(t_sys_data_current.logo_id);
      user_pin_lcd_backlight_ctrl(true);
      (void)OS_DELAY(3000);//延时，显示logo图标一段时间
    }

    /**LOGO界面结束************************************************************/
    gui::set_current_display(maindisplayF);
    user_pin_lcd_backlight_ctrl(true);
    /*
    //品质测试用的固件20170825
    motion_3d.enable_board_test = 1;
    gui::set_current_display(board_test_display_function);
    */
  }
}

int WarningInfoFlag = 1;
void ErrInterfaceControl(void)
{
  if (IsWarning) //是否显示错误界面
  {
    if (WarningInfoFlag)
    {
      WarningInfoFlag = 0;
      gui::set_current_display(WarningInfoF);
    }
  }
}

void transfileF(void)
{
  if (gui::is_refresh())
  {
    display_picture(25);
  }
}

void TransFileInterfaceControl(void)
{
  static int transflag = 1;
  static int transprintflag = 1;
  static int transpause = 0;

  if (IsTransFile) //是否显示传输界面
  {
    if (transflag)
    {
      transflag = 0;
      transpause = 1;
      transprintflag = 1;
      gui::set_current_display(transfileF);
    }
  }
  else if (IsPrintSDFile()) //是否打印上传的文件
  {
    if (transprintflag)
    {
      transpause = 0;
      transflag = 1;
      transprintflag = 0;
      strcpy(printname, SDFileName);
      respond_gui_send_sem(FilePrintValue);
      pauseprint = 0;
      print_flage = 1;
      gui::set_current_display(maindisplayF);
    }
  }
  else  //显示主界面
  {
    if (transpause)
    {
      transpause = 0;
      transprintflag = 0;
      transflag = 1;
      pauseprint = 0;
      print_flage = 0;
      gui::set_current_display(maindisplayF);
    }
  }
}

void TouchAndRtcControl(void)
{
  static struct _touch *tch = &touch;

  if (t_gui.is_refresh_rtc)    //根据公共api变量实时刷新rtc信号
  {
    t_gui.is_refresh_rtc = 0;
    gui::rtc_flag = 1;
  }

  if (tch->touch_flag ||  gui::rtc_flag)      //根据触摸信号和rtc信号进入界面显示
  {
    gui::current_display(); //进入模块函数处理信号

    if (tch->touch_flag && tch->up_flag) //有触摸信号，进入函数处理
    {
      tch->touch_flag = 0;            //关闭相关的tch信号
      tch->up_flag = 0;               //关闭相关的tch信号
    }
    else if (gui::rtc_flag)       //有rtc信号，进入函数处理
    {
      gui::rtc_flag = 0;                     //给gui::rtc_flag信号赋值
    }
  }
}

void gui_task_loop(void)
{
//  static unsigned long RefreshGuiTimeOut = 0;
  osDelay(50);

//  if (RefreshGuiTimeOut < xTaskGetTickCount())
  {
    PowerOntoRunOnce(); //上电后界面显示首先要执行的操作
    TouchAndRtcControl(); //触摸反应和界面数据刷新
    guitouchscan(); //触摸扫描函数，处理扫描数据
    ErrInterfaceControl(); //错误界面显示控制
    TransFileInterfaceControl(); //传输界面控制
    DoorOpenWarningInfo_NotPrinting(); //M14R03,M14S 门打开高温提示
//    RefreshGuiTimeOut = xTaskGetTickCount() + 50;
  }
}



#ifdef __cplusplus
} //extern "C" {
#endif



