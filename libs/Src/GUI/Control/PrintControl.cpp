#include "PrintControl.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "gcodebufferhandle.h"
#include "controlxyz.h"
#include "controlfunction.h"
#include "gcode.h"
#include "process_m_code.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_fan.h"
#include "user_ccm.h"
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "planner.h"
#include "stepper.h"
#include "process_command.h"
#include "interface.h"
#include "RespondGUI.h"
#include  "USBFileTransfer.h"
#include "config_model_tables.h"
#include "temperature.h"
#include "jshdisplay.h"
#include "flashconfig.h"

extern uint32_t old_sd_pos[6];/*20170807堵料修复*/


/////////////////////////////////////////////////////////////////////////////////////
///////////////////////// PrintControl private variables/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#define SD_CARD 1                                  /*!< SD卡存储介质 */
#define UDISK 2                                    /*!< U盘存储介质 */
char NAND_Path[4];
static uint8_t medium_id = UDISK;                /*!< 记录打印文件存储介质 */
static unsigned long starttime = 0;              /*!< 打印时间计数 */

// 打印状态
static volatile bool is_printing = false;                 /*!< 是否正在打印 */
static volatile bool is_pause_printing = false;           /*!< 是否正在暂停打印 */
static volatile bool is_stop_printing = false;            /*!< 是否正在停止打印 */
static volatile bool is_resume_printing = false;          /*!< 是否正在恢复止打印 */
static volatile bool is_finish_print = false;             /*!< 是否完成了打印 */
static volatile bool is_poweroff_recovery_print = false;  /*!< 是否为断电恢复操作 */

// 暂停打印后，等待电机停止，停止后移开喷嘴
static int pause_print_status = 0;               /*!< 暂停打印状态 */
static bool is_heating_status = false;           /*!< 是否处于打印加热状态 */
static TickType_t xTimeToCoolDown = 0;           /*!< 冷却超时时间 */
static int temp_hotend_to_resume = 0;            /*!< 恢复打印喷嘴温度 */
static int temp_bed_to_resume = 0;               /*!< 恢复打印热床温度 */
static float OldPosition[MAX_NUM_AXIS];                     /*!< 恢复打印XYZ位置 */
static int OldFeedRate;                          /*!< 恢复打印进料速度 */
static bool is_pause_to_cool_down = false;       /*!< 是否暂停温度降低 */
static bool is_pause_to_resume_temp = false;     /*!< 是否暂停恢复温度 */

// 停止打印
static bool is_process_stop_print = false;       /*!< 是否执行停止打印 */

/////////////////////////////////////////////////////////////////////////////////////
///////////////////// PrintFileControl private variables/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#define ReadSDSize  512

static unsigned int sd_buffersize = ReadSDSize;      /*!< 读取文件数组大小 */
//  static unsigned int sd_buffersize_bak = ReadSDSize;  /*!< 读取文件数组大小 */
static uint32_t print_file_size;                     /*!< 文件大小 */
static volatile uint32_t file_size = 0;              /*!< 文件大小计数 */
//  static volatile uint32_t file_size_bak = 0;          /*!< 文件大小计数 */
static char printfilepathname[_MAX_LFN];             /*!< 文件名称 */
static FIL printfile;                                /*!< 文件对象 */
static char sd_buffer[ReadSDSize];                   /*!< 读取数组对象 */
static bool is_print_sd_file = false;                /*!< 是否打印sd卡文件 */

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  private function/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static void reset_print_status(void)
{
  is_printing = false;
  is_pause_printing = false;
  is_stop_printing = false;
  is_resume_printing = false;
  is_finish_print = false;
}
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  public function/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//是否在加热
int IsHeating(void)
{
  if (!t_custom_services.disable_hot_bed)
  {
    if ((int)temperature_get_bed_target())
      return 1;
  }

  if ((int)temperature_get_extruder_target(0))
    return 1;

  return 0;
}

//是否正在打印
int IsPrint(void)
{
  return is_printing;
}

int IsPausePrint(void)
{
  return is_pause_printing;
}

int IsResumePrint(void)
{
  return is_resume_printing;
}

void SetPrintStatus(bool status)
{
  is_printing = status;
}

// 设置暂停打印状态
void SetPausePrintingStatus(bool status)
{
  is_pause_printing = status;
}

void PowerOffRecStartPrint(void)
{
  is_poweroff_recovery_print = true;
  printControl.start();
}

bool IsFinishedPrint(void)
{
  return is_finish_print;
}

bool IsPrintSDFile(void)
{
  return is_print_sd_file;
}

bool IsPauseToCoolDown(void)
{
  return is_pause_to_cool_down;
}

//从SD卡读取gcode命令
void readGcodeBuf(void)
{
  printFileControl.getGcodeBuf();
}

#ifdef __cplusplus
} //extern "C"
#endif

PrintControl::PrintControl()
{
}

// 执行暂停打印操作，平台下降60mm，xy归零
void PrintControl::pauseProcess(void)
{
  if (!is_heating_status) // 未开始打印，不用执行暂停操作
  {
    st_synchronize();

    for (int i = 0; i < XYZ_NUM_AXIS; i++)
    {
      //      if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
      //      {
      //        OldPosition[i] = st_get_position_length(i); //获取位置
      //      }
      //      else
      {
        OldPosition[i] = GetCurrentPosition(i); //获取位置
      }
    }

    {
      OldPosition[E_AXIS] = GetCurrentPosition(E_AXIS); //获取位置
      OldPosition[B_AXIS] = GetCurrentPosition(B_AXIS); //获取位置
    }

    OldFeedRate  = GetFeedRate(); //进料速度
    z_down_60mm_and_xy_to_zero();
  }
}

unsigned int PrintControl::getTime(void)
{
  return (unsigned int)((xTaskGetTickCount() / 1000 - starttime / 1000) + t_sys.print_time_save);
}

//停止打印
void PrintControl::stop(void)
{
  reset_print_status();
  is_stop_printing = true;
  printFileControl.close();
  poweroff_reset_flag();
  SetIsUSBPrintStop(true);
}

void PrintControl::prepareStop(void)
{
  if (is_stop_printing) // 串口停止打印或正常停止打印
  {
    stepper_quick_stop(); // 电机快速停止

    if (0 == planner_moves_planned())                    // 运动队列为空，执行以下操作
    {
      is_stop_printing = false;                   // 设置停止打印状态为false
      is_process_stop_print = true;             // 执行停止打印标志置为true

      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
      {
        t_sys.idex_print_type = IDEX_PRINT_TYPE_NORMAL;
      }

      return;
    }
  }
}

void PrintControl::processStop(void)
{
  prepareStop();                                                                   // 停止打印准备操作
  OS_DELAY(100); // 延时让其他任务执行

  if (is_process_stop_print)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
    {
      user_send_internal_cmd((char *)"T0 S-1");
    }

    z_down_60mm_and_xy_to_zero();
    //    if (gcode::g28_complete_flag)                                                     // 是否已经归零
    //    {
    //      z_down_60mm_and_xy_to_zero();                                                        // 平台下降60mm
    //    }
    //    else
    //    {
    //      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
    //      {
    //        user_send_internal_cmd((char *)"T0 S-1");
    //        user_send_internal_cmd((char *)"G28");      // XYZ归零
    //      }
    //      else
    //      {
    //        user_send_internal_cmd((char *)"G28");      // XYZ归零
    //      }
    //      user_send_internal_cmd((char *)"M2003 S0");    // 关闭坐标转换
    //      user_send_internal_cmd((char *)"G1 Z60");   // Z下降60mm
    //      user_send_internal_cmd((char *)"M2003 S1");    // 开启坐标转换
    //    }
    OS_DELAY(100); // 延时让其他任务执行
    m84_disable_steppers();
    resetCmdBuf();                                                                 // 重置指令数组
    t_gui.cura_speed = 0;                                                          // 读取gcode文件获取数值，停止打印则清0
    is_process_stop_print = false;                                                    // 执行停止打印标志置为false
    OS_DELAY(100); // 延时让其他任务执行
    user_send_internal_cmd((char *)"M2004 S1");
  }
}

//继续打印
void PrintControl::resume(void)
{
  reset_print_status();
  is_resume_printing = true;
  //  poweroff_reset_flag();
}

void PrintControl::pause(void)
{
  reset_print_status();
  is_pause_printing = true;
  pause_print_status = 0;
  is_pause_to_cool_down = false;
}

//开始打印
void PrintControl::start(void)
{
  reset_print_status();

  // 文件打开成功，发送指令M2000设置打印状态
  if (printFileControl.open())
  {
    flash_erase_poweroff_data();
    t_sys.idex_print_type = flash_param_t.idex_print_type;
    stepper_quick_stop(); // 电机快速停止
    user_send_file_cmd((char *)"M2000", 0, 0, 0);
  }

  if (t_sys_data_current.enable_powerOff_recovery)
  {
    //    poweroff_reset_flag(); //重置标志位
    poweroff_set_file_path_name(printfilepathname);
    is_poweroff_recovery_print = false; // 完成恢复操作，重置断电恢复操作标志
  }

  ccm_param::layer_count = 0;
  ccm_param::current_layer = 0;
  t_sys.enable_color_buf = 0;
  // 重置联机打印状态
  SetIsUSBPrintFinished(false);
  SetIsUSBPrintStop(false);
  SetIsUSBPrintPause(false);
}

void PrintControl::pauseToCoolDown(float coolDownFactor)
{
  temp_hotend_to_resume = (int)temperature_get_extruder_target(0);  // 保持当前喷嘴温度，用于恢复打印
  temp_bed_to_resume = (int)temperature_get_bed_target();         // 保持当前热床温度，用于恢复打印
  float hotendTemp = (temperature_get_extruder_target(0) * coolDownFactor);
  float bedTemp = (temperature_get_bed_target() * coolDownFactor);

  if (t_gui.target_nozzle_temp[0] != hotendTemp) //防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
  {
    temperature_set_extruder_target(hotendTemp, 0);
    t_gui.target_nozzle_temp[0] = hotendTemp;
  }

  if (!t_custom_services.disable_hot_bed && t_gui.target_hot_bed_temp != bedTemp)
  {
    temperature_set_bed_target(bedTemp);
    t_gui.target_hot_bed_temp = bedTemp;
  }

  is_pause_to_cool_down = true; // 断料状态下，换料要用到，即降温了则要先加温才能换料
}
volatile uint8_t active_extruder_bak = 0;
void PrintControl::processPause(void)
{
  if (!is_pause_printing) return;

  if (is_pause_printing && 0 == pause_print_status)
  {
    pause_print_status = 1;
  }

  // 判断当前是否已经加热完成，加热完成跳到下一步
  // 未加热，设置is_heating_status为true，等待加热完成
  if (1 == pause_print_status)
  {
    if (isM109HeatingComplete()) // 已加热，下一步
    {
      is_heating_status = false; // 处于加热状态
    }
    else
    {
      is_heating_status = true; // 处于加热状态
    }

    pause_print_status = 2;
  }
  else if (2 == pause_print_status)   // 判断当前运动队列是否执行完
  {
    if (is_heating_status)
    {
      is_printing = false; // 设置打印状态为false
      SetIsUSBPrintPause(true);
      pause_print_status = 3;
      return;
    }

    if (0 == planner_moves_planned()) // 运动队列为空，下一步
    {
      is_printing = false; // 设置打印状态为false
      SetIsUSBPrintPause(true);
      pause_print_status = 3;

      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
      {
        t_sys.idex_print_type_bak = t_sys.idex_print_type;
        t_sys.idex_print_type = IDEX_PRINT_TYPE_NORMAL;
      }
    }
  }
  else if (3 == pause_print_status) // 执行暂停打印操作
  {
    active_extruder_bak = gcode::active_extruder;
    pauseProcess();
    pause_print_status = 4;
  }
  else if (4 == pause_print_status && planner_moves_planned() == 0)
  {
    pause_print_status = 5;
    user_send_internal_cmd((char *)"M2004 S1");
    char home_xy_pos[96];
    user_send_internal_cmd((char *)"T0 S-1");
    sprintf(home_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)home_xy_pos);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1 && t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
    {
      user_send_internal_cmd((char *)"T1 S-1");
      sprintf(home_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
      user_send_internal_cmd((char *)home_xy_pos);
      sprintf(home_xy_pos, "T%d S-1", active_extruder_bak);
      user_send_internal_cmd((char *)home_xy_pos);
    }
  }
  else if (5 == pause_print_status) // 设置暂停打印冷却超时时间
  {
    xTimeToCoolDown = xTaskGetTickCount() + 1000 * 60 * 3;
    pause_print_status = 6;
  }
  else if (6 == pause_print_status) // 暂停超时，执行温度下降操作
  {
    if (xTaskGetTickCount() > xTimeToCoolDown)
    {
      pauseToCoolDown(0.5f);
      pause_print_status = 7;
    }
  }
  else if (7 == pause_print_status) // 暂停打印结束
    is_pause_printing = false;
}

bool PrintControl::resumeBackToPrintPos(void)
{
  static int resume_bak_pos_status = 0;

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
  {
    t_sys.idex_print_type = t_sys.idex_print_type_bak;
  }

  if (!is_heating_status)
  {
    if (0 == resume_bak_pos_status)
    {
      eb_compensate_8mm(t_sys_data_current.enable_color_mixing);
      resume_bak_pos_status = 1;
      return false;
    }
    else if (1 == resume_bak_pos_status && 0 == planner_moves_planned())
    {
      static char cmdResume_z_pos[50] = {0};
      static char cmdResume_xy_pos[50] = {0};
      static char cmdResume_eb_pos[50] = {0};

      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1 && t_sys.idex_print_type == IDEX_PRINT_TYPE_NORMAL)
      {
        if (gcode::active_extruder == 0)
        {
          (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X%0.4f Y%0.4f D1", OldPosition[X_AXIS], OldPosition[Y_AXIS]);
        }
        else if (gcode::active_extruder == 1)
        {
          (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X%0.4f Y%0.4f D1", OldPosition[X2_AXIS], OldPosition[Y_AXIS]);
        }
      }
      else
      {
        if (ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS] < OldPosition[X_AXIS] || ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS] < OldPosition[Y_AXIS])
        {
          (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X0 Y0 D1");
        }
        else
        {
          (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X%0.4f Y%0.4f D1", OldPosition[X_AXIS], OldPosition[Y_AXIS]);
        }
      }

      (void)snprintf(cmdResume_z_pos, sizeof(cmdResume_z_pos), "G1 F%d Z%0.4f D1", OldFeedRate, OldPosition[Z_AXIS]);
      OS_DELAY(50);
      user_send_internal_cmd((char *)"M2003 S0"); // 关闭坐标转换

      if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
      {
        user_send_internal_cmd((char *)cmdResume_xy_pos); //XY回到原来位置
        user_send_internal_cmd((char *)"M120"); //关闭限位检测
        user_send_internal_cmd((char *)cmdResume_z_pos); //z回到原来位置
      }
      else
      {
        user_send_internal_cmd((char *)"M120"); //关闭限位检测
        user_send_internal_cmd((char *)cmdResume_z_pos); //z回到原来位置
        user_send_internal_cmd((char *)cmdResume_xy_pos); //XY回到原来位置
      }

      OS_DELAY(50);
      (void)snprintf(cmdResume_eb_pos, sizeof(cmdResume_eb_pos), "G92 E%0.4f B%0.4f", OldPosition[E_AXIS], OldPosition[B_AXIS]);
      user_send_internal_cmd((char *)cmdResume_eb_pos); //XY回到原来位置
      user_send_internal_cmd((char *)"M2003 S1");    // 开启坐标转换
      //      g92_set_axis_position((int)E_AXIS, OldPosition[E_AXIS]);
      //      g92_set_axis_position((int)B_AXIS, OldPosition[B_AXIS]);
      OS_DELAY(50);
      resume_bak_pos_status = 2;
      return false;
    }
    else if (2 == resume_bak_pos_status && 0 == planner_moves_planned())
    {
      resume_bak_pos_status = 0; // 重置变量，避免第二次恢复打印异常
      return true;
    }

    return false;
  }
  else
    return true;
}

void PrintControl::pauseToResumeTemp(void)
{
  if (!is_pause_to_resume_temp)
  {
    if (t_gui.target_nozzle_temp[0] != temp_hotend_to_resume) //防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
    {
      temperature_set_extruder_target(temp_hotend_to_resume, 0);
      t_gui.target_nozzle_temp[0] = temp_hotend_to_resume;
    }

    if (!t_custom_services.disable_hot_bed && t_gui.target_hot_bed_temp != temp_bed_to_resume)
    {
      temperature_set_bed_target(temp_bed_to_resume);
      t_gui.target_hot_bed_temp = temp_bed_to_resume;
    }

    is_pause_to_resume_temp = true;
  }

  OS_DELAY(50);
}

bool PrintControl::isResumeTempDone(void)
{
  pauseToResumeTemp();
  int degh = (int)temperature_get_extruder_current(0);
  int deghT = (int)temperature_get_extruder_target(0);

  if (degh < deghT) return false;

  if (!t_custom_services.disable_hot_bed)
  {
    //    int degb=degBed();
    //    int degbT=degTargetBed();
    //      if((degh<deghT) || (degb<degbT)) return;//暂停后返回打印，热床不等待，2017/3/14
  }
  else
  {
    if (degh < deghT) return false;
  }

  return true;
}

void PrintControl::processResumeFinish(void)
{
  pause_print_status = 0;
  is_heating_status = false;
  is_resume_printing = false;
  is_pause_to_resume_temp = false;
  SetIsUSBPrintPause(false);
  user_send_internal_cmd((char *)"M2001");
  user_send_internal_cmd((char *)"M2004 S1");
}

void PrintControl::processResume(void)
{
  if (!is_resume_printing) return;

  switch (pause_print_status)
  {
  case 0:
    return;

  case 1:
  case 2:
  case 3:
    break;

  case 4:
  case 5:
  case 6:
    if (!resumeBackToPrintPos()) return;

    break;

  case 7:
    if (!isResumeTempDone())
      return;

    if (!resumeBackToPrintPos()) return;

    break;
  } // end switch

  processResumeFinish();
}
/////////////////////////////////////////////////////////////////////
///////////////////////PrintFileControl//////////////////////////////
/////////////////////////////////////////////////////////////////////

PrintFileControl::PrintFileControl()
{
}

void PrintFileControl::getFileName(void)
{
  if (is_poweroff_recovery_print) // 断电续打恢复打印，设置打印文件名
  {
    if (t_power_off.is_file_from_sd)   //断电续打文件在SD卡中
    {
      medium_id = SD_CARD;
      //      IsPowerOffRecoverFileInSD = 1; //打印的是SD卡中的文件
    }

    (void)strcpy(printfilepathname, t_power_off.path_file_name);
  }
  else // 正常打印获取文件名
  {
    if (UDISK == medium_id) // U盘文件
    {
      if (IsRootDir)
        (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s", CurrentPath);
      else
        (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s/", CurrentPath);

      (void)strcat(printfilepathname, SettingInfoToSYS.PrintFileName);
    }
    else if (SD_CARD == medium_id) // SD卡文件
    {
      (void)strcpy(NAND_Path, "1:/");
      (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s", NAND_Path);
      (void)strcat(printfilepathname, SDFileName);
    }
  }
}

bool PrintFileControl::open(void)
{
  bool file_open_status = true;
  getFileName();  // 获取要打印文件名

  if (SD_CARD == medium_id)
  {
    taskENTER_CRITICAL();
  }

  if (f_open(&printfile, printfilepathname, FA_READ) == FR_OK)
  {
    print_file_size = f_size(&printfile);

    // 断电续打重设文件打印位置
    if (is_poweroff_recovery_print && (t_power_off.sd_pos != 0))
    {
      file_size = print_file_size - t_power_off.sd_pos;
      (void)f_lseek(&printfile, t_power_off.sd_pos);
    }
    else
    {
      file_size = print_file_size;
    }

    //提取BMP图片
    if (strstr(printfilepathname, ".sgcode"))
      readBMPFile(printfilepathname);

    resetCmdBuf();
    // 重置gui打印状态
    t_gui.printed_time_sec = 0;
    t_gui.print_percent = 0;
    t_gui.printfile_size = print_file_size;
    t_gui.file_size = file_size;
    t_gui.used_total_material = 0;
    starttime = xTaskGetTickCount();
  }
  else
  {
    file_open_status = false;
  }

  if (SD_CARD == medium_id)
  {
    taskEXIT_CRITICAL();
  }

  return file_open_status;
}

void PrintFileControl::close(void)
{
  print_file_size = 0;
  file_size = 0;
  sd_buffersize = ReadSDSize;

  if (SD_CARD == medium_id)
  {
    is_print_sd_file = false;
    t_power_off.is_file_from_sd = 0;
    medium_id = UDISK;
    taskENTER_CRITICAL();
    (void)f_close(&printfile);
    (void)f_unlink(printfilepathname); //打印完删除上传的文件
    taskEXIT_CRITICAL();
  }
  else //从U盘读取的时候，不能添加，因为USB的控制是一个单独的任务
  {
    (void)f_close(&printfile);
  }

  //删除BMP图片
  if (strstr(printfilepathname, ".sgcode"))
  {
    taskENTER_CRITICAL();
    (void)f_unlink(BMP_PATH);
    taskEXIT_CRITICAL();
  }
}

void PrintFileControl::readBMPFile(const char *fileName)
{
  uint32_t RCount;
  MaseHeader header;
  FilesMsg msg;
  FIL *file = new (FIL);
  FIL *file1 = new (FIL);
  FRESULT f_res;
  f_res = f_open(file, fileName, FA_READ);

  if (f_res != FR_OK)
  {
    USER_EchoLogStr("File error!!\t f_res = %d\r\n", f_res);
  }
  else
  {
    // 读取header
    f_read(file, &header, sizeof(MaseHeader), &RCount);
    //定位gcode文件
    f_lseek(file, sizeof(MaseHeader) + 1 * sizeof(FilesMsg));
    f_read(file, &msg, sizeof(FilesMsg), &RCount);
    strcpy(SettingInfoToSYS.PrintFileName, printname);
    print_file_size = msg.uFileSize;

    if (is_poweroff_recovery_print && (t_power_off.sd_pos != 0))
    {
      file_size = print_file_size - t_power_off.sd_pos;
      (void)f_lseek(&printfile, t_power_off.sd_pos + msg.uFileOfs);
    }
    else
    {
      file_size = print_file_size;
      (void)f_lseek(&printfile, msg.uFileOfs);
    }

    /*
    USER_EchoLogStr("PrintFileName=   %s\n",SettingInfoToSYS.PrintFileName);
    USER_EchoLogStr("msg.szFileName=  %s\n",msg.szFileName);
    USER_EchoLogStr("msg.uFileOfs=    %d\n",msg.uFileOfs);
    USER_EchoLogStr("msg.uFileSize=   %d\n",msg.uFileSize);
    USER_EchoLogStr("print_file_size = %d\n",print_file_size);
    USER_EchoLogStr("file_size=       %d\n",file_size);
    */
    f_close(file);
  }

  delete file;
  delete file1;
  file = NULL;
  file1 = NULL;
}

//从SD卡读取gcode命令
void PrintFileControl::getGcodeBuf(void)
{
  if (is_printing)
  {
    while (file_size)
    {
      if (sd_buffersize == ReadSDSize)
      {
        HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0); //恢复电机中断的优先级
        OS_DELAY(100);
        bool isReadSuccess = readDataToBuf();
        HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0); //恢复电机中断的优先级

        //        USER_DbgLog("sd_position in ReadSDGcode=%u,file_size=%u",(print_file_size - file_size),file_size);
        if (isReadSuccess)
          break;    //读取文件失败
      }
      else
      {
        char sd_char = sd_buffer[sd_buffersize++];
        uint32_t file_position =  print_file_size - file_size;
        file_size--;
        t_gui.file_size = file_size;

        if (0 == file_size)
        {
          readFinish();
        }

        if (GetGcodeFromBuf(sd_char, file_position, (1 == t_sys_data_current.enable_color_mixing && 1 == t_sys_data_current.have_set_machine), file_size, sd_buffersize)) // 若是混色版本 且 已经设置好机器，需要解密
          break;
      }
    }
  }
}

void PrintFileControl::readFinish(void)
{
  close();                  // 读取结束，关闭文件
  st_synchronize();         // 执行完环形队列指令再执行下面指令
  reset_print_status();
  is_finish_print = true;   // 设置打印完成标志为true，用于gui页面跳转
  t_gui.used_total_material += GetCurrentPosition((int)E_AXIS);

  if (1 == t_sys_data_current.enable_color_mixing)
  {
    t_gui.used_total_material += GetCurrentPosition((int)B_AXIS);
  }

  if (1 == t_sys_data_current.enable_color_mixing)
  {
    user_send_internal_cmd((char *)"G92 E0 B0");
  }
  else
  {
    user_send_internal_cmd((char *)"G92 E0");
  }

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
  {
    t_sys.idex_print_type = IDEX_PRINT_TYPE_NORMAL;
  }

  z_down_60mm_and_xy_to_zero();            // 打印完后下降60
  SetIsUSBPrintFinished(true);     // USB联机已完成打印
}

bool PrintFileControl::readDataToBuf(void)
{
  UINT file_br;    /* File R/W count */

  if (file_size)
  {
    (void)memset(sd_buffer, 0, sizeof(sd_buffer)); //clear buffer
    sd_buffersize = 0;

    if (SD_CARD == medium_id)
    {
      taskENTER_CRITICAL();
    }

    if (f_read(&printfile, sd_buffer, (file_size <= ReadSDSize) ? file_size : sizeof(sd_buffer), &file_br) != FR_OK)
    {
      PopWarningInfo(FatfsWarning);
      return 1;
    }

    if (SD_CARD == medium_id)
    {
      taskEXIT_CRITICAL();
    }
  }

  return 0;
}

/**
 * [PrintFileControl::getPercent 获取文件打印百分比]
 * @return  [description]
 */
int PrintFileControl::getPercent(void)
{
  if (print_file_size)
    return (int)((print_file_size - file_size) / ((print_file_size + 99) / 100));
  else
    return 0;
}

/**
 * [PrintFileControl::setSDMedium 设置当前打印介质为SD卡，联机打印需要用到]
 */
void PrintFileControl::setSDMedium(void)
{
  medium_id = SD_CARD;     // 设置当前文件所在介质为sd卡
  is_print_sd_file = true; // 设置打印sd文件标志为true
}

PrintControl printControl;
PrintFileControl printFileControl;
