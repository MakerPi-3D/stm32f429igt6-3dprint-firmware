#include "poweroffrecovery.h"
#include "functioncustom.h"
#include "machinecustom.h"
#include "midwaychangematerial.h"
#include "controlxyz.h"
#include "controlfunction.h"
#include "planner.h"
#include "planner_running_status.h"
#include "gcode.h"
#include "process_m_code.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_ccm.h"
#include "user_fan.h"
#include "user_common.h"
#include "flashconfig.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "globalvariables.h"
#include "temperature.h"
#include "stepper.h"

#include "PrintControl.h"
#include "process_command.h"

#define POWER_OFF_REC_FILE             ((char *)"1:/PowerOffData.txt")          /*!< 断电文件 */
#define POWER_OFF_REC_FILE_BAK         ((char *)"1:/PowerOffData_bak.txt")      /*!< 断电备份文件 */
#define POWER_OFF_REC_FILE_PATH        ((char *)"D1:")                          /*!< 文件名(含路径) */
#define POWER_OFF_REC_BED_TARGET_TEMP  ((char *)"D2:")                          /*!< 热床目标温度 */
#define POWER_OFF_REC_NOZ_TARGET_TEMP  ((char *)"D3:")                          /*!< 喷嘴目标温度 */
#define POWER_OFF_REC_FAN_SPEED        ((char *)"D4:")                          /*!< 风扇速度 */
#define POWER_OFF_REC_FEED_MULTIPLY    ((char *)"D5:")                          /*!< 打印速度 */
#define POWER_OFF_REC_FEED_RATE        ((char *)"D6:")                          /*!< 出料速度 */
#define POWER_OFF_REC_Z_POS            ((char *)"D7:")                          /*!< Z轴位置 */
#define POWER_OFF_REC_E_POS            ((char *)"D8:")                          /*!< E轴位置 */
#define POWER_OFF_REC_B_POS            ((char *)"D9:")                          /*!< B轴位置 */
#define POWER_OFF_REC_SD_POS           ((char *)"D10:")                         /*!< 文件指针位置 */
#define POWER_OFF_REC_SD_POS2          ((char *)"D11:")                         /*!< 文件指针位置2 */
#define POWER_OFF_REC_SD_POS3          ((char *)"D12:")                         /*!< 文件指针位置3 */
#define BLOCKDETECT_FLAG               ((char *)"D13:")                         /*!< 堵料标志 */
#define POWER_OFF_REC_FLAG             ((char *)"D14:")                         /*!< 标志 */
#define POWER_OFF_IS_SERIAL_FLAG       ((char *)"D15:")                         /*!< 串口标志 */
#define POWER_OFF_REC_X_POS            ((char *)"D16:")                          /*!< X轴位置 */
#define POWER_OFF_REC_Y_POS            ((char *)"D17:")                          /*!< Y轴位置 */
#define POWER_OFF_REC_TIME             ((char *)"D18:")                          /*!< Y轴位置 */
static FIL power_off_file;

//  extern uint8_t serialPrintStatus(void);
#ifdef __cplusplus
} //extern "C" {
#endif
FLASH_POWEROFF_RECOVERY_T flash_poweroff_recovery_t2;
static float e_position = 0, b_position = 0;

PowerOffOperation::PowerOffOperation()
{
  IsStartCalculateZMaxPos = false;
  isStopGetZMax = false;
  isPowerOffRecoverPrint = 0;
  isPowerOffRecoverPrintFinish = false;
}

void PowerOffOperation::startCalculateZMaxPos(void)
{
  isStopGetZMax = false;
  user_send_internal_cmd((char *)"G28 O1"); // XYZ归零
  z_down_to_bottom(); // Z下降到底部
  IsStartCalculateZMaxPos = true; // 设置开始校准Z高度
  gcode::g28_complete_flag = false; // 设置为未归零
}

void PowerOffOperation::stopGetZMaxPos(void)
{
  isStopGetZMax = true;
}

void PowerOffOperation::getZMaxPos(void)
{
  if (isStopGetZMax)
  {
    stepper_quick_stop(); // 电机快速停止
    IsStartCalculateZMaxPos = false;
    isStopGetZMax = false;
    OS_DELAY(50);
  }

  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  float ZMaxPosValue;

  if (IsStartCalculateZMaxPos) // 开始校准Z
  {
    if (0 == planner_moves_planned() && st_check_endstop_z_hit_max() && gcode::g28_complete_flag) //已碰到限位开关  //先归零，然后向下碰限位开关
    {
      ZMaxPosValue = st_get_endstops_len(Z_AXIS);
      SaveCalculateZMaxPos(ZMaxPosValue);
      //      set_cs_z_max_pos(ZMaxPosValue);
      IsFinishedCalculateZPosLimit = 1;
      //      SetCurrentPosition(Z_AXIS,ZMaxPosValue);
      IsStartCalculateZMaxPos = false;
      user_send_internal_cmd((char *)"G28");
      //串口上传信息到上位机2017.7.6
      USER_EchoLogStr("z_max_pos:%.2f\r\n", ZMaxPosValue);
      OS_DELAY(50);
    }
  }
}

//断电续打取消删除SD卡中的文件
void PowerOffOperation::deleteFileFromSD(void)
{
  if (t_power_off.is_file_from_sd)   //断电续打文件在SD卡中，删除文件
  {
    taskENTER_CRITICAL();
    (void)f_unlink(t_power_off.path_file_name);
    taskEXIT_CRITICAL();
  }
}

static void recovery_print_heating(void)
{
  static char gcodeM104OrM190CommandBuf[50] = {0};
  static char gcodeM109CommandBuf[50] = {0};
  memset(gcodeM104OrM190CommandBuf, 0, sizeof(gcodeM104OrM190CommandBuf));

  if (!t_custom_services.disable_hot_bed)
  {
    if (t_power_off.bed_target_temp > 50)
    {
      user_send_internal_cmd((char *)"M190 S50");
      (void)snprintf(gcodeM104OrM190CommandBuf, sizeof(gcodeM104OrM190CommandBuf), "M140 S%d", t_power_off.bed_target_temp);
    }
    else
    {
      (void)snprintf(gcodeM104OrM190CommandBuf, sizeof(gcodeM104OrM190CommandBuf), "M190 S%d", t_power_off.bed_target_temp);
    }

    user_send_internal_cmd((char *)gcodeM104OrM190CommandBuf); //热床加温命令
  }

  memset(gcodeM109CommandBuf, 0, sizeof(gcodeM109CommandBuf));
  (void)snprintf(gcodeM109CommandBuf, sizeof(gcodeM109CommandBuf), "M109 S%d", t_power_off.nozzle_target_temp);
  user_send_internal_cmd((char *)gcodeM109CommandBuf); //喷嘴加温命令
}

static void recovery_print_set_EB(void)
{
  eb_compensate_8mm(t_sys_data_current.enable_color_mixing);
  g92_set_axis_position((int)E_AXIS, t_power_off.e_pos);

  if (t_sys_data_current.enable_color_mixing)
  {
    g92_set_axis_position((int)B_AXIS, t_power_off.b_pos);
  }
}

static void recovery_print_set_Z(void)
{
  static char gcodeG1FCommandBuf[50] = {0};
  static char gcodeG1ZCommandBuf[50] = {0};
  static char gcodeG1XYCommandBuf[50] = {0};
  user_send_internal_cmd((char *)"M2003 S0"); // 关闭坐标转换
  memset(gcodeG1ZCommandBuf, 0, sizeof(gcodeG1ZCommandBuf));
  (void)snprintf(gcodeG1ZCommandBuf, sizeof(gcodeG1ZCommandBuf), "G92 Z%f R%u", t_power_off.z_pos, t_power_off.sd_pos);
  user_send_internal_cmd((char *)gcodeG1ZCommandBuf); //Z位置
  memset(gcodeG1XYCommandBuf, 0, sizeof(gcodeG1XYCommandBuf));
  (void)snprintf(gcodeG1XYCommandBuf, sizeof(gcodeG1XYCommandBuf), "G1 F2400 X%f Y%f Z%f", t_power_off.x_pos, t_power_off.y_pos, t_power_off.z_pos); //添加上文件位置
  user_send_internal_cmd((char *)gcodeG1XYCommandBuf);
  user_send_internal_cmd((char *)"M2003 S1");    // 开启坐标转换
  user_send_internal_cmd((char *)"M2003 S0");    // 关闭坐标转换
  memset(gcodeG1FCommandBuf, 0, sizeof(gcodeG1FCommandBuf));

  if (t_power_off.feed_rate > 2400)
    t_power_off.feed_rate = 2400; // 限制移动速度最大为40mm/s

  (void)snprintf(gcodeG1FCommandBuf, sizeof(gcodeG1FCommandBuf), "G1 F%f", t_power_off.feed_rate);
  user_send_internal_cmd((char *)gcodeG1FCommandBuf); //出料速度
  user_send_internal_cmd((char *)"M2003 S1");    // 开启坐标转换
}

static void recovery_print_close_to_saved_z(void)
{
  static char gcodeG1CommandBuf[50] = {0};
  user_send_internal_cmd((char *)"M2003 S0");    // 关闭坐标转换
  memset(gcodeG1CommandBuf, 0, sizeof(gcodeG1CommandBuf));
  //  float zUpValue = ((t_sys_data_current.poweroff_rec_z_max_value - t_power_off.z_pos) > 50) ? t_power_off.z_pos + 50 : t_sys_data_current.poweroff_rec_z_max_value;
  (void)snprintf(gcodeG1CommandBuf, sizeof(gcodeG1CommandBuf), "G92  Z%f", t_power_off.z_pos);
  user_send_internal_cmd((char *)gcodeG1CommandBuf); //设置Z最大位置
  user_send_internal_cmd((char *)"M2003 S1"); // 开启坐标转换
}

void PowerOffOperation::recoveryPrintLoop(void)
{
  static uint8_t powerOffRecoverPrintStatus = 0;
  static char gcodeM140CommandBuf[50] = {0};
  static char gcodeM104CommandBuf[50] = {0};
  gcode::g28_complete_flag = false; //设置为归零

  if (powerOffRecoverPrintStatus == 0 && (isM109HeatingComplete()) && 0 == planner_moves_planned())   //加热完成和Z轴向下完成
  {
    t_power_off.is_power_off = 1; //标志为断电状态，防止把当前sdPos写入到poweroff_data中，解决断电多次重新打印现象
    gcode::g28_complete_flag = false; //设置为归零
    //    ccm_param::motion_3d.updown_g28_first_time = 1; // 设置已经执行了上下共限位归零操作
    //    z_check_and_set_bottom(ccm_param::motion_3d.enable_poweroff_up_down_min_min, t_sys_data_current.poweroff_rec_z_max_value); // 检测z底部位置
    //    z_check_and_set_bottom(ccm_param::motion_3d.enable_poweroff_up_down_min_min, t_power_off.z_pos);
    memset(gcodeM140CommandBuf, 0, sizeof(gcodeM140CommandBuf));
    (void)snprintf(gcodeM140CommandBuf, sizeof(gcodeM140CommandBuf), "M140 S%d", t_power_off.bed_target_temp);
    user_send_internal_cmd((char *)gcodeM140CommandBuf); //设置Z最大位
    memset(gcodeM104CommandBuf, 0, sizeof(gcodeM104CommandBuf));
    (void)snprintf(gcodeM104CommandBuf, sizeof(gcodeM104CommandBuf), "M104 S%d", t_power_off.nozzle_target_temp);
    user_send_internal_cmd((char *)gcodeM104CommandBuf);

    if (flash_param_t.idex_print_type == IDEX_PRINT_TYPE_COPY || flash_param_t.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
    {
      memset(gcodeM104CommandBuf, 0, sizeof(gcodeM104CommandBuf));
      (void)snprintf(gcodeM104CommandBuf, sizeof(gcodeM104CommandBuf), "M104 T1 S%d", t_power_off.nozzle_target_temp);
      user_send_internal_cmd((char *)gcodeM104CommandBuf);
    }

    powerOffRecoverPrintStatus = 1;
    isPowerOffRecoverPrintFinish = false;
  }
  else if (powerOffRecoverPrintStatus == 1 && 0 == planner_moves_planned())
  {
    //        recovery_print_close_to_saved_z();
    powerOffRecoverPrintStatus = 2;
  }
  else if (powerOffRecoverPrintStatus == 2 && 0 == planner_moves_planned())
  {
    resetM109HeatingComplete();  //设置为未加热
    recovery_print_heating();
    powerOffRecoverPrintStatus = 3;
  }
  else if (powerOffRecoverPrintStatus == 3 && (isM109HeatingComplete()))     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
    recovery_print_set_EB();
    powerOffRecoverPrintStatus = 4;
  }
  else if (powerOffRecoverPrintStatus == 4 && 0 == planner_moves_planned())     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
    recovery_print_set_Z();
    powerOffRecoverPrintStatus = 5;
  }
  else if (powerOffRecoverPrintStatus == 5 && 0 == planner_moves_planned())     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
    feature_set_extruder_fan_speed(t_power_off.fan_speed);         //风扇速度
    SetFeedMultiply(t_power_off.feed_multiply); //打印速度
    powerOffRecoverPrintStatus = 0;
    isPowerOffRecoverPrintFinish = true;
  }
}

//打开文件，继续打印
void PowerOffOperation::recoveryPrint(void)
{
  if (!t_sys_data_current.enable_powerOff_recovery)
    return;

  static uint8_t isStartPowerOffRecoverPrint = 0;

  if (isPowerOffRecoverPrint && 0 == planner_moves_planned()) // 等待平台降到最低处
  {
    isStartPowerOffRecoverPrint = 1;
    isPowerOffRecoverPrint = 0;
  }

  if (isStartPowerOffRecoverPrint)
  {
    if (t_power_off.sd_pos != 0)
    {
      recoveryPrintLoop();
    }
    else
    {
      isPowerOffRecoverPrintFinish = true;
    }

    if (isPowerOffRecoverPrintFinish)
      isStartPowerOffRecoverPrint = 0;
  }

  recoveryPrintFinish();
}

void PowerOffOperation::recoveryPrintFinish(void)
{
  if (isPowerOffRecoverPrintFinish)
  {
    IsFinishedPowerOffRecoverReady = 1; // UI界面更新标志位
    PowerOffRecStartPrint(); //开始从文件读取内容继续去打印
    isPowerOffRecoverPrintFinish = false;
  }
}

//准备继续打印
void PowerOffOperation::readyToRecoveryPrint(void)
{
  stepper_quick_stop(); // 电机快速停止
  resetM109HeatingComplete(); //设置为未加热
  ccm_param::motion_3d.updown_g28_first_time = 0;
  user_send_internal_cmd((char *)"G90");
  user_send_internal_cmd((char *)"M82");
  user_send_internal_cmd((char *)"M109 S130"); // 先加热到130度，再移动喷嘴防止喷嘴与打印模具粘在一起。

  if (flash_param_t.idex_print_type == IDEX_PRINT_TYPE_COPY || flash_param_t.idex_print_type == IDEX_PRINT_TYPE_MIRROR)
  {
    user_send_internal_cmd((char *)"M109 T1 S130");
  }

  OS_DELAY(600);
  xy_to_zero();
  isPowerOffRecoverPrint = 1;
}

void PowerOffOperation::saveFileBak(void)
{
  taskENTER_CRITICAL();

  if (f_open(&power_off_file, POWER_OFF_REC_FILE_BAK, FA_CREATE_NEW | FA_WRITE) == FR_OK) //打开保存数据的文件
  {
    (void)f_lseek(&power_off_file, 0); //重新指向0
    (void)f_write(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, (UINT *)&poweroff_data_size);  //写入空白符清空原来的信息
    (void)f_sync(&power_off_file); //保存好
    (void)f_close(&power_off_file);
    #if (1 == DEBUG_POWEROFF_CUSTOM)
    USER_DbgLog("PowerOffOperation::saveFileBak Done !");
    #endif
  }
  else
  {
    USER_ErrLog("poweroff_save_file_bak open file failed!");
  }

  taskEXIT_CRITICAL();
}

void PowerOffOperation::resetFlag(void)
{
  if (t_power_off.flag)
    saveFileBak();

  for (int i = 0; i < POWER_OFF_BUF_SIZE; i++) //初始化填入空白符到BUF
    ccm_param::poweroff_data[i] = ' ';

  taskENTER_CRITICAL();

  if (f_open(&power_off_file, POWER_OFF_REC_FILE, FA_READ | FA_WRITE) == FR_OK) //打开保存数据的文件
  {
    (void)f_lseek(&power_off_file, 0); //重新指向0
    (void)f_write(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, (UINT *)&poweroff_data_size);  //写入空白符清空原来的信息
    (void)f_sync(&power_off_file); //保存好
    (void)f_lseek(&power_off_file, 0); //重新指向0
    (void)f_write(&power_off_file, "D14:0", sizeof("D14:0"), (UINT *)&poweroff_data_size);  //重写入标志
    (void)f_sync(&power_off_file); //保存好标志信息
    (void)f_close(&power_off_file);
    #if (1 == DEBUG_POWEROFF_CUSTOM)
    USER_DbgLog("PowerOffOperation::resetFlag Done !");
    #endif
  }
  else
  {
    USER_ErrLog("PowerOffSave::resetFlag(void) open file failed!");
  }

  taskEXIT_CRITICAL();
}

void PowerOffOperation::syncData(void)
{
  //  if (f_open(&power_off_file, POWER_OFF_REC_FILE, FA_READ | FA_WRITE) == FR_OK) //打开保存数据的文件
  //  {
  //    (void)f_write(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, (UINT *)&poweroff_data_size);
  //    (void)f_sync(&power_off_file);
  //    (void)f_close(&power_off_file);
  //    #if (1 == DEBUG_POWEROFF_CUSTOM)
  //    USER_DbgLog("PowerOffOperation::syncData Done !");
  //    USER_DbgLog("%40s\r\n%s", "poweroff_data_str_in_syncData", ccm_param::poweroff_data);
  //    #endif
  //  }
  flash_poweroff_recovery_t.bedTargetTemp = (int)temperature_get_bed_target();                     /*!< 热床目标温度 */
  flash_poweroff_recovery_t.nozzleTargetTemp = (int)temperature_get_extruder_target(0);            /*!< 喷嘴目标温度 */
  flash_poweroff_recovery_t.fanSpeed = plan_get_fans();                                            /*!< 风扇速度 */
  flash_poweroff_recovery_t.feedMultiply = plan_get_fmultiply();                                   /*!< 进料速度百分比 */
  flash_poweroff_recovery_t.feedRate = (float)GetFeedRate();                                       /*!< 进料速度 */
  flash_poweroff_recovery_t.xPos = (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(X_AXIS) : plan_get_axis_position((int)X_AXIS);                                 /*!< x位置 */
  flash_poweroff_recovery_t.yPos = ((BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(Y_AXIS) : plan_get_axis_position((int)Y_AXIS));                                 /*!< y位置 */
  flash_poweroff_recovery_t.zPos = (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(Z_AXIS) : plan_get_axis_position((int)Z_AXIS);                                 /*!< z位置 */
  flash_poweroff_recovery_t.ePos =  e_position;                                /*!< e位置 */
  flash_poweroff_recovery_t.bPos =  b_position;                                /*!< b位置 */
  flash_poweroff_recovery_t.sdPos = plan_get_sd_position();                                        /*!< 文件位置 */
  flash_poweroff_recovery_t.flag = 1;                                               /*!< 断电标志位 */
  flash_poweroff_recovery_t.serialFlag = plan_get_serial_flag();                                   /*!< 串口标志位 */
  flash_poweroff_recovery_t.blockflag = IsNotHaveMatInPrint;                                       /*!< 堵料标志位 */
  flash_save_poweroff_data();
}

void PowerOffOperation::setFilePathName(const char *filePathName)
{
  memcpy(powerOffFilePathName, filePathName, strlen(filePathName));
  powerOffFilePathName[strlen(filePathName)] = 0;
  //  snprintf(poweroff_data, sizeof(ccm_param::poweroff_data), "D1:%s\r\n",filePathName); // 设置文件名
  //  poweroff_data_size = strlen(ccm_param::poweroff_data); // 获取文件名字符串长度
}

void PowerOffOperation::setData(void)
{
  if (t_sys_data_current.enable_powerOff_recovery)
  {
    static int _current_block_buffer_tail = -1;
    static int bed_target = 0;
    static int hotend_target = 0;
    static bool _isPausePrint = false;

    if (IsPrint())
    {
      if (_current_block_buffer_tail == ccm_param::block_buffer_tail && (bed_target != (int)temperature_get_bed_target() || hotend_target != (int)temperature_get_extruder_target(0)))
      {
        setDataBuf();
        bed_target = (int)temperature_get_bed_target();
        hotend_target = (int)temperature_get_extruder_target(0);
      }

      if (_current_block_buffer_tail != ccm_param::block_buffer_tail) // 當前運動隊列變化，保存數據
      {
        setDataBuf();
        //        taskENTER_CRITICAL();
        //        (void)f_write(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, (UINT *)&poweroff_data_size);
        //        taskEXIT_CRITICAL();
        _current_block_buffer_tail = ccm_param::block_buffer_tail;
        #if (1 == DEBUG_POWEROFF_CUSTOM)
        USER_DbgLog("PowerOffOperation::setData IsPrint !");
        #endif
      }

      _isPausePrint = false;
    }
    else if (IsPausePrint() || IsMidWayChangeMat()) // 暂停打印或者中途换料，先保存断电数据
    {
      if (!_isPausePrint) // 暫停打印只保存一次
      {
        setDataBuf();
        //        taskENTER_CRITICAL();
        //        (void)f_write(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, (UINT *)&poweroff_data_size);
        syncData();
        //        taskEXIT_CRITICAL();
        #if (1 == DEBUG_POWEROFF_CUSTOM)
        USER_DbgLog("PowerOffOperation::setData IsPausePrint !");
        #endif
      }

      _current_block_buffer_tail = -1;
      _isPausePrint = true;
    }
    else
    {
      _current_block_buffer_tail = -1;
      _isPausePrint = false;
    }
  }
}

void PowerOffOperation::setDataBuf(void)
{
  static unsigned long setDataTimeOut = 0;
  static uint32_t sd_position2 = 0, sd_position3 = 0;
  //  static float e_position = 0, b_position = 0; //2017517缓存E、B电机的位置，保存上一次的值，因为sd的位置是上上次的

  if (setDataTimeOut < xTaskGetTickCount())
  {
    for (int i = 0; i < POWER_OFF_BUF_SIZE; i++) //初始化填入空白符到BUF
      ccm_param::poweroff_data[i] = ' ';

    (void)snprintf(&ccm_param::poweroff_data[0], sizeof(ccm_param::poweroff_data), \
                   "D1:%s\r\nD2:%d\r\nD3:%d\r\nD4:%d\r\nD5:%d\r\nD6:%f\r\nD7:%f\r\nD8:%f\r\nD9:%f\r\nD10:%d\r\nD11:%d\r\nD12:%d\r\nD13:%d\r\nD14:1\r\nD15:%d\r\nD16:%f\r\nD17:%f\r\nD18:%ld", \
                   powerOffFilePathName, (int)temperature_get_bed_target(), (int)temperature_get_extruder_target(0), plan_get_fans(), plan_get_fmultiply(),
                   (float)GetFeedRate(), ((BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(Z_AXIS) : plan_get_axis_position((int)Z_AXIS)), e_position, b_position, (int)plan_get_sd_position(), sd_position2, sd_position3,
                   IsNotHaveMatInPrint, (plan_get_serial_flag()),
                   ((BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(X_AXIS) : plan_get_axis_position((int)X_AXIS)),
                   ((BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level) ? st_get_position_length(Y_AXIS) : plan_get_axis_position((int)Y_AXIS)), printControl.getTime()); //2017516记录三个sd位置
    sd_position3 = sd_position2;
    sd_position2 = plan_get_sd_position();

    if (t_sys_data_current.enable_color_mixing)
    {
      e_position = plan_get_axis_position((int)E_AXIS) - 8.0f;//减4为了补偿在返回打印途中“漏掉”的耗材
      b_position = plan_get_axis_position((int)B_AXIS) - 8.0f;
    }
    else
    {
      e_position = plan_get_axis_position((int)E_AXIS) - 0.0f;//减4为了补偿在返回打印途中“漏掉”的耗材
      b_position = plan_get_axis_position((int)B_AXIS) - 0.0f;
    }

    setDataTimeOut = xTaskGetTickCount() + 10;
  }

  #if (1 == DEBUG_POWEROFF_CUSTOM)
  //  USER_DbgLog("%40s\r\n%s", "poweroff_data_in_setDataBuf",ccm_param::poweroff_data);
  USER_DbgLog("X%f Y%f Z%f E%f \r\n",
              plan_get_axis_position(X_AXIS),
              plan_get_axis_position(Y_AXIS),
              plan_get_axis_position(Z_AXIS),
              plan_get_axis_position(E_AXIS));
  #endif
}


PowerOffRecovery::PowerOffRecovery()
{
  bedTargetTemp = 0;
  nozzleTargetTemp = 0;
  fanSpeed = 0;
  feedMultiply = 0;
  feedRate = 0.0f;
  xPos = 0.0f;
  yPos = 0.0f;
  zPos = 0.0f;
  ePos = 0.0f;
  bPos = 0.0f;
  sdPos = 0;
  memset(pathFileName, 0, sizeof(pathFileName));
  memset(fileName, 0, sizeof(fileName));
  flag = 0;
  blockflag = 0;
}

// 初始化断电数据
void PowerOffRecovery::init(void)
{
  if (t_sys_data_current.enable_powerOff_recovery)
  {
    //    readPowerOffData();
    explainPowerOffData();
    powerOffRecoveryLog();
  }
}

// 初始化全局变量
void PowerOffRecovery::initGlobalVariables(void)
{
  t_power_off.bed_target_temp = bedTargetTemp;
  t_power_off.nozzle_target_temp = nozzleTargetTemp;
  t_power_off.fan_speed = fanSpeed;
  t_power_off.feed_multiply = feedMultiply;
  t_power_off.feed_rate = feedRate;
  t_power_off.x_pos = xPos;
  t_power_off.y_pos = yPos;
  t_power_off.z_pos = zPos;
  t_power_off.e_pos = ePos;
  t_power_off.b_pos = bPos;
  t_power_off.sd_pos = sdPos;
  t_power_off.serial_flag = serialFlag;
  (void)strcpy(t_power_off.path_file_name, pathFileName);
  (void)strcpy(t_power_off.file_name, fileName);
  t_power_off.flag = flash_poweroff_recovery_t2.flag;

  if ((uint8_t)(pathFileName[0] - '0') == 0)
    t_power_off.is_file_from_sd = 0;

  t_power_off.blockdetectflag = blockflag;
}

// 打印已经读取断电数据
void PowerOffRecovery::powerOffRecoveryLog(void)
{
  #if (1 == DEBUG_POWEROFF_CUSTOM)
  USER_DbgLog("%40s", "########poweroff recovery start########");
  USER_DbgLog("%40s = %d", "poweroff_bed_target_temp", bedTargetTemp);
  USER_DbgLog("%40s = %d", "poweroff_nozzle_target_temp", nozzleTargetTemp);
  USER_DbgLog("%40s = %d", "poweroff_fan_speed", fanSpeed);
  USER_DbgLog("%40s = %d", "poweroff_feed_multiply", feedMultiply);
  USER_DbgLog("%40s = %f", "poweroff_feed_rate", feedRate);
  USER_DbgLog("%40s = %f", "poweroff_z_pos", zPos);
  USER_DbgLog("%40s = %f", "poweroff_e_pos", ePos);
  USER_DbgLog("%40s = %f", "poweroff_b_pos", bPos);
  USER_DbgLog("%40s = %d", "poweroff_sd_pos", sdPos);
  USER_DbgLog("%40s = %d", "poweroff_flag", flag);
  USER_DbgLog("%40s = %s", "poweroff_path_file_name", pathFileName);
  USER_DbgLog("%40s = %s", "poweroff_file_name", fileName);
  USER_DbgLog("%40s", "########poweroff recovery end########");
  #endif
}

// 读取断电保存数据
void PowerOffRecovery::readPowerOffData(void)
{
  for (int i = 0; i < POWER_OFF_BUF_SIZE; i++) //初始化填入空白符到BUF
    ccm_param::poweroff_data[i] = ' ';

  taskENTER_CRITICAL();

  if (f_open(&power_off_file, POWER_OFF_REC_FILE, FA_READ | FA_WRITE) == FR_OK) //打开保存数据的文件
  {
    (void)f_read(&power_off_file, ccm_param::poweroff_data, POWER_OFF_BUF_SIZE, &poweroff_data_size);
    ccm_param::poweroff_data[poweroff_data_size] = 0;
    f_close(&power_off_file);
  }
  else
  {
    USER_ErrLog("PowerOffFile Open Failed!");
  }

  taskEXIT_CRITICAL();
}

/*
从片内flash中读出断电保存的数据
*/

static void flash_read_poweroff_data(void)
{
  uint32_t address;
  uint32_t *pd;
  uint16_t i;
  address = FLASH_POWEROFF_DATA_START_ADDR;
  pd = (uint32_t *)(&flash_poweroff_recovery_t2);

  for (i = 0; i < sizeof(flash_poweroff_recovery_t2) / 4; i++)
  {
    *pd = *((volatile uint32_t *) address);
    address += 4;
    pd ++;
  }
}



// 解析断电保存数据
void PowerOffRecovery::explainPowerOffData(void)
{
  flash_read_poweroff_data();
  getFlag();//获取标志位
  getSerialFlag();

  if (serialFlag) // 串口标志位开，断电标志关闭
    flag = 0;

  if (flag || serialFlag) //需要续打
  {
    getBedTargetTemp();
    getNozzleTargetTemp();
    getFanSpeed();
    getFeedMultiply();
    getFeedRate();
    getXPos();
    getYPos();
    getZPos();
    getEPos();
    getBPos();
    getSDPos();
    getPathFileName();
    (void)strcpy(fileName, flash_param_t.fileName);
    getblockflag();
    //    getPrintTime();
  }
}



// 获取断电标志位
void PowerOffRecovery::getFlag(void)
{
  flag = flash_poweroff_recovery_t2.flag;
}

// 获取串口标志位
void PowerOffRecovery::getSerialFlag(void)
{
  serialFlag = flash_poweroff_recovery_t2.serialFlag;
}

// 获取热床目标温度
void PowerOffRecovery::getBedTargetTemp(void)
{
  bedTargetTemp = flash_poweroff_recovery_t2.bedTargetTemp;
}

// 获取喷嘴目标温度
void PowerOffRecovery::getNozzleTargetTemp(void)
{
  nozzleTargetTemp = flash_poweroff_recovery_t2.nozzleTargetTemp;
}

// 获取风扇速度
void PowerOffRecovery::getFanSpeed(void)
{
  fanSpeed = flash_poweroff_recovery_t2.fanSpeed;
}

// 进料速度百分比
void PowerOffRecovery::getFeedMultiply(void)
{
  feedMultiply = flash_poweroff_recovery_t2.feedMultiply;
}

// 进料速度
void PowerOffRecovery::getFeedRate(void)
{
  feedRate = flash_poweroff_recovery_t2.feedRate;
}

// 获取Z轴位置
void PowerOffRecovery::getZPos(void)
{
  zPos = flash_poweroff_recovery_t2.zPos;
}

// 获取X轴位置
void PowerOffRecovery::getXPos(void)
{
  xPos = flash_poweroff_recovery_t2.xPos;
}

// 获取Y轴位置
void PowerOffRecovery::getYPos(void)
{
  yPos = flash_poweroff_recovery_t2.yPos;
}

// 获取E轴位置
void PowerOffRecovery::getEPos(void)
{
  ePos = flash_poweroff_recovery_t2.ePos;
}

// 获取B轴位置
void PowerOffRecovery::getBPos(void)
{
  bPos = flash_poweroff_recovery_t2.bPos;
}

// 获取文件位置
void PowerOffRecovery::getSDPos(void)
{
  sdPos = flash_poweroff_recovery_t2.sdPos;
}

// 获取文件位置
void PowerOffRecovery::getPrintTime(void)
{
  char *TimeInfoPtr;
  TimeInfoPtr = strstr((char *)ccm_param::poweroff_data, POWER_OFF_REC_TIME); //D12:34556
  TimeInfoPtr = TimeInfoPtr + strlen(POWER_OFF_REC_TIME);
  t_sys.print_time_save = (uint32_t)atol(TimeInfoPtr); //转换成长整型
}

// 获取文件路径名
void PowerOffRecovery::getPathFileName(void)
{
  (void)strcpy(pathFileName, flash_param_t.pathFileName);
}

// 获取
void PowerOffRecovery::getblockflag(void)
{
  blockflag = flash_poweroff_recovery_t2.blockflag;
}

PowerOffOperation powerOffOperation;

