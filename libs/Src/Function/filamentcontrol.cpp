#include "filamentcontrol.h"
#include "globalvariables.h"
#include "stepper.h"
#include "temperature.h"
#include "machinecustom.h"
#include "controlfunction.h"
#include "controlxyz.h"
#include "planner.h"
#include "planner_running_status.h"
#include "config_model_tables.h"
#include "math.h"

#include "PrintControl.h"
#include "process_command.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "ConfigurationStore.h"

#include "process_m_code.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "gcode.h"
#include "user_common.h"
#include "flashconfig.h"
#ifdef __cplusplus
extern "C" {
#endif

void TransFileReady(void)
{
  //关闭蜂鸣器
  user_buzzer_set_alarm_status(false); //以防刚完成打印又传输文件造成的蜂鸣器鸣叫

  //关闭进丝或退丝，以防在进丝或退丝界面时上传文件
  if (!t_custom_services.disable_hot_bed)
  {
    temperature_set_bed_target((float)0);
    t_gui.target_hot_bed_temp = 0;
  }

  filamentControl.resetStatus();
}

static bool is_process_load_unload_done = false;

#ifdef __cplusplus
} //extern "C" {
#endif

FilamentControl::FilamentControl()
{
  startLoadFlag = 0;
  startUnloadFlag = 0;
  timeOutFlag = 0;
  timeOutTickCount = 0;
}

// 准备进退丝操作
void FilamentControl::prepare(void)
{
  IsSuccessFilament = 0;
  IsFinishedFilamentHeat = 0;
  temperature_set_extruder_target((float)filament_load_unload_temp, 0);
  t_gui.target_nozzle_temp[0] = filament_load_unload_temp;
  gcode::active_extruder = 0;

  // 喷嘴预热230度
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    temperature_set_extruder_target((float)filament_load_unload_temp, 1);
    t_gui.target_nozzle_temp[1] = filament_load_unload_temp;
  }

  if (!gcode::g28_complete_flag)                                                         // 进退丝，平台未归零，先归零
  {
    user_send_internal_cmd((char*)"G28");            // xyz归零
  }

  if (st_get_position_length(X_AXIS) > 0.0f || st_get_position_length(Y_AXIS) > 0.0f)               // XY是否在零点
  {
    if (P3_Pro == t_sys_data_current.model_id)
    {
      user_send_internal_cmd((char*)"M2003 S0");     // 关闭坐标转换
      user_send_internal_cmd((char*)"G1 F2400 X0 Y0"); // 移动XY到零点
      user_send_internal_cmd((char*)"M2003 S1");     // 开启坐标转换
    }
    else
    {
      xy_to_zero();
    }
  }

  if (st_get_position_length(Z_AXIS) < 50.0f)                                                // 获取平台当前实际高度，小于50mm，下降到50mm位置
  {
    user_send_internal_cmd((char*)"M2003 S0");     // 关闭坐标转换
    user_send_internal_cmd((char*)"G1 F600 Z50");    // z下降到50mm位置
    user_send_internal_cmd((char*)"M2003 S1");     // 开启坐标转换
  }

  user_send_internal_cmd((char*)"M83");              // 设置喷嘴E、B为绝对模式
}

// 开始进丝
void FilamentControl::startLoad(void)
{
  if (!startLoadFlag) // 避免重复触发
    prepare();

  startLoadFlag = 1;
}

// 开始退丝
void FilamentControl::startUnload(void)
{
  if (!startUnloadFlag) // 避免重复触发
    prepare();

  startUnloadFlag = 1;
}

// 执行进丝操作
void FilamentControl::processLoad(void)
{
  bool is_heating_done = (int)temperature_get_extruder_current(0) >= (t_gui.target_nozzle_temp[0] - 3);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    is_heating_done = is_heating_done && (int)temperature_get_extruder_current(1) >= (t_gui.target_nozzle_temp[1] - 3);
  }

  if (is_heating_done)   //最后几度要等待挺长时间的，不再等待
  {
    if (!timeOutFlag)
    {
      USER_EchoLogStr("M701 start load\r\n");//串口上传信息到上位机2017.7.6
      timeOutTickCount = xTaskGetTickCount() + 120 * 1000UL; //120s
      timeOutFlag = 1;
    }
  }

  if (timeOutFlag && !is_process_load_unload_done)
  {
    bool is_above_min_temp = (int)temperature_get_extruder_current(0) >= ccm_param::motion_3d.extrude_min_temp;

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      is_above_min_temp = is_above_min_temp && (int)temperature_get_extruder_current(1) >= ccm_param::motion_3d.extrude_min_temp;
    }

    if (is_above_min_temp)
    {
      if (planner_moves_planned() > 1)   //只有1个有效block的时候，继续发送进丝命令
      {
        return;
      }
      else
      {
        user_send_internal_cmd((char*)"G92 E0 B0");

        if (1 == t_sys_data_current.enable_color_mixing)
        {
          user_send_internal_cmd((char*)"G1 F140 E15 B15");//2017.4.24更改为140，增大挤出头拉力
        }
        else
        {
          user_send_internal_cmd((char*)"G1 F140 E10 B10");//2017.4.24更改为140，增大挤出头拉力
        }
      }
    }
  }
}


// 执行退丝操作
void FilamentControl::processUnload(void)
{
  static int filament_mm_count = 0;
  timeOutTickCount = xTaskGetTickCount() + 60 * 1000UL; //60s
  bool is_heating_done = (int)temperature_get_extruder_current(0) >= (t_gui.target_nozzle_temp[0] - 3);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    is_heating_done = is_heating_done && (int)temperature_get_extruder_current(1) >= (t_gui.target_nozzle_temp[1] - 3);
  }

  //if((int)degHotend(0)>=(FilamentTemp-3))  //最后几度要等待挺长时间的，不再等待
  if (is_heating_done)   //最后几度要等待挺长时间的，不再等待
  {
    if (!timeOutFlag)
    {
      USER_EchoLogStr("M702 start unload\r\n");//串口上传信息到上位机2017.7.6
      timeOutFlag = 1;
      user_send_internal_cmd((char*)"G92 E0 B0");
      user_send_internal_cmd((char*)"G1 F200 E50 B50");

      if (1 == t_sys_data_current.enable_color_mixing)
      {
        if (P3_Pro == t_sys_data_current.model_id)
        {
          user_send_internal_cmd((char*)"G92 E0 B0");
          user_send_internal_cmd((char*)"G1 F8400 E-15 B-15");//2017425退丝刚开始，快速退丝；B电机离喷嘴近点测试得退5mm以内最合适，E电机离喷嘴较远，退多一点
          user_send_internal_cmd((char*)"G92 E0 B0");
          user_send_internal_cmd((char*)"G1 F2500 E-70 B-70");
        }
        else
        {
          user_send_internal_cmd((char*)"G92 E0 B0");
          user_send_internal_cmd((char*)"G1 F8400 E-15 B-5");//2017425退丝刚开始，快速退丝；B电机离喷嘴近点测试得退5mm以内最合适，E电机离喷嘴较远，退多一点
          user_send_internal_cmd((char*)"G92 E0 B0");
          user_send_internal_cmd((char*)"G1 F2500 E-65 B-70");
        }
      }

      filament_mm_count = 80;
    }
  }

  if (timeOutFlag && !is_process_load_unload_done)
  {
    bool is_above_min_temp = (int)temperature_get_extruder_current(0) >= ccm_param::motion_3d.extrude_min_temp;

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      is_above_min_temp = is_above_min_temp && (int)temperature_get_extruder_current(1) >= ccm_param::motion_3d.extrude_min_temp;
    }

    //if((int)degHotend(0)>=(FilamentTemp-30))
    if (is_above_min_temp)
    {
      if (planner_moves_planned() > 1)
      {
        return;
      }
      else     //只有1个有效block的时候，继续发送退丝命令
      {
        user_send_internal_cmd((char*)"G1 F500 E-10 B-10");
        filament_mm_count += 10;

        if (filament_mm_count > 400)
        {
          timeOutTickCount = 0;
        }
      }
    }
  }
}


// 执行进退丝入口
void FilamentControl::process(void)
{
  if (!startLoadFlag && !startUnloadFlag) // 进退丝标志位都为false，退出
    return;

  if (startLoadFlag)
    processLoad();

  if (startUnloadFlag)
    processUnload();

  //进丝成功或退丝成功
  if (timeOutFlag)
  {
    // 判断进丝或退丝加热是否完成
    IsFinishedFilamentHeat = 1;

    if (xTaskGetTickCount() > timeOutTickCount)
    {
      is_process_load_unload_done = true;
    }

    if (is_process_load_unload_done && planner_moves_planned() == 0)
    {
      // 退出进退丝操作
      exit(false);
      // 判断进丝或退丝是否完成
      IsSuccessFilament = 1;
    }
  }

  OS_DELAY(50);
}

// 重置进退丝状态
void FilamentControl::resetStatus(void)
{
  // 设置目标温度为0
  temperature_set_extruder_target((float)0, 0);
  t_gui.target_nozzle_temp[0] = 0;

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    temperature_set_extruder_target((float)0, 1);
    t_gui.target_nozzle_temp[1] = 0;
  }

  // 重置进退丝状态变量
  startLoadFlag = 0;
  startUnloadFlag = 0;
  timeOutFlag = 0;
  timeOutTickCount = 0;
  is_process_load_unload_done = false;
}

void FilamentControl::exit(bool isCancel)
{
  // 串口上传信息到上位机2017.7.6
  if (!isCancel)
    USER_EchoLogStr("M701 M702 finish\r\n");

  // 退出进丝或退丝操作
  user_send_internal_cmd((char*)"G1 F2400");      // 设置速度为7200mm/min
  user_send_internal_cmd((char*)"M82");           // 关闭绝对模式
  user_send_internal_cmd((char*)"G92 E0 B0");     // 重置E、B坐标值为0
  // 重置进退丝状态
  resetStatus();
}

void FilamentControl::cancelProcess(void)
{
  stepper_quick_stop(); // 电机快速停止
  exit(true);
}

FilamentControl filamentControl;









