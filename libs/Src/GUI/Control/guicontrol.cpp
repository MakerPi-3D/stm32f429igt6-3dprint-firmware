#include "guicontrol.h"
#include "globalvariables.h"
#include "user_common.h"
#include "temperature.h"
#include "interface.h"
#include "machinecustom.h"
#include "functioncustom.h"
#include "process_command.h"
#include "controlxyz.h"
#include "stm32f4xx_hal.h"
#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "user_fan.h"
#include "gcode.h"
#include "user_ccm.h"
#include "flashconfig.h"
#include "sysconfig_data.h"
#ifdef __cplusplus
extern "C" {
#endif
extern void SetFeedMultiply(int FeedMultiplyValue);

extern int IsPrint(void);
extern int IsHeating(void);

static int TempHotendToResume = 0;
static int TempBedToResume = 0;

//写喷嘴目标温度
void GUI_WNozzleTargetTemp(int NozzleTemp)
{
  if (t_gui.target_nozzle_temp[0] == NozzleTemp) //防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
    return;

  temperature_set_extruder_target((float)NozzleTemp, 0);
  t_gui.target_nozzle_temp[0] = NozzleTemp;
}

//写喷嘴目标温度
void GUI_WNozzleTargetTemp1(int NozzleTemp)
{
  if (t_gui.target_nozzle_temp[1] == NozzleTemp) //防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
    return;

  temperature_set_extruder_target((float)NozzleTemp, 1);
  t_gui.target_nozzle_temp[1] = NozzleTemp;
}

//写热床目标温度
void GUI_WHotbedTargetTemp(int Hotbedtemp)
{
  if (t_gui.target_hot_bed_temp == Hotbedtemp) //防止二次设置
    return;

  temperature_set_bed_target((float)Hotbedtemp);
  t_gui.target_hot_bed_temp = Hotbedtemp;
}

void PauseToCoolDown(float cooldownFactor)
{
  TempHotendToResume = (int)temperature_get_extruder_target(0);
  TempBedToResume = (int)temperature_get_bed_target();
  GUI_WNozzleTargetTemp((float)TempHotendToResume * cooldownFactor);

  if (!t_custom_services.disable_hot_bed)
    GUI_WHotbedTargetTemp((float)TempBedToResume * cooldownFactor);
}
void PauseToResumeTemp(void)
{
  GUI_WNozzleTargetTemp(TempHotendToResume);

  if (!t_custom_services.disable_hot_bed)
    GUI_WHotbedTargetTemp(TempBedToResume);
}

void PauseToResumeNozTemp(void)
{
  GUI_WNozzleTargetTemp(TempHotendToResume);
}

//防止长时间保持高温状态，导致材料融化使挤出头多了一层颜色20170502
void protect_nozzle(float hour)
{
  static uint32_t beginTime = 0;

  if (t_gui.nozzle_temp[0] < 45) beginTime = 0;

  if (t_gui.nozzle_temp[0] > 45 && beginTime == 0)
    beginTime = xTaskGetTickCount() / 1000;

  if ((xTaskGetTickCount() / 1000 - beginTime) > (uint16_t)(3600 * hour))
  {
    beginTime = 0;

    if (t_gui.nozzle_temp[0] > 45)
      guiControl.coolDown();
  }
}
#ifdef __cplusplus
} //extern "C" {
#endif

GUIControl::GUIControl()
{
}

// 解锁步进电机
void GUIControl::disableSteppers(void)
{
  USER_DbgLog("DisableStep ok!");
  m84_disable_steppers();
  gcode::g28_complete_flag = false;
}

// 移动光轴
void GUIControl::moveXYZ(int (&xyz_mm)[XYZ_NUM_AXIS])
{
  if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
      flash_param_t.extruder_type == EXTRUDER_TYPE_LASER ||
      flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    user_send_internal_cmd((char *)"T0 S-1");
  }

  // 判断是否已经归零
  if (!gcode::g28_complete_flag)
    user_send_internal_cmd((char *)"G28");

  // 限制xyz最小最大值
  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    if (xyz_mm[i] < ccm_param::motion_3d_model.xyz_min_pos[i])
      xyz_mm[i] = (int)ccm_param::motion_3d_model.xyz_min_pos[i];

    if (xyz_mm[i] > ccm_param::motion_3d_model.xyz_max_pos[i])
      xyz_mm[i] = (int)ccm_param::motion_3d_model.xyz_max_pos[i];
  }

  float x, y, z;
  gcode::g28_get_home_pos_adding(-1, x, y, z);

  // 发送移动指令
  if (x > 0.0f || y > 0.0f)
  {
    static char moveXYZ[55];
    memset(moveXYZ, 0, sizeof(moveXYZ));
    (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f Z%f", (float)xyz_mm[X_AXIS] + x, (float)xyz_mm[Y_AXIS] + y, (float)xyz_mm[Z_AXIS]);
    user_send_internal_cmd((char *)moveXYZ);
  }
  else
  {
    static char moveXYZ[55];
    memset(moveXYZ, 0, sizeof(moveXYZ));
    (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f", (float)xyz_mm[X_AXIS], (float)xyz_mm[Y_AXIS]);
    user_send_internal_cmd((char *)moveXYZ);
    static char moveXYZ2[55];
    memset(moveXYZ2, 0, sizeof(moveXYZ2));
    (void)snprintf(moveXYZ2, sizeof(moveXYZ2), "G1 F300 Z%f", (float)xyz_mm[Z_AXIS]);
    user_send_internal_cmd((char *)moveXYZ2);
  }
}

// 预热abs
void GUIControl::preHeatABS(void)
{
  USER_DbgLog("PreHeatABS ok!");
  GUI_WNozzleTargetTemp(abs_preheat_hotend_temp);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    GUI_WNozzleTargetTemp1(abs_preheat_hotend_temp);
  }

  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(abs_preheat_hpb_temp);
  }

  feature_set_extruder_fan_speed(0);
}

// 预热pla
void GUIControl::preHeatPLA(void)
{
  USER_DbgLog("PreHeatPLA ok!");
  GUI_WNozzleTargetTemp(pla_preheat_hotend_temp);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    GUI_WNozzleTargetTemp1(pla_preheat_hotend_temp);
  }

  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(pla_preheat_hpb_temp);
  }

  feature_set_extruder_fan_speed(0);
}

// 预热bed
void GUIControl::preHeatBed(void)
{
  USER_DbgLog("PreHeatBed ok!");

  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(100);
  }

  feature_set_extruder_fan_speed(0);
}

void GUIControl::printSetForM14(void)
{
  GUI_WNozzleTargetTemp(SettingInfoToSYS.TargetNozzleTemp);
  feature_set_extruder_fan_speed(SettingInfoToSYS.FanSpeed); // 写风扇速度
  SetFeedMultiply(SettingInfoToSYS.PrintSpeed); //int PrintSpeed :100->1 200->2 写打印速度
}

void GUIControl::printSetForNotM14Left(void)
{
  GUI_WNozzleTargetTemp(SettingInfoToSYS.TargetNozzleTemp);
  SetFeedMultiply(SettingInfoToSYS.PrintSpeed); //int PrintSpeed :100->1 200->2 写打印速度

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    GUI_WNozzleTargetTemp1(SettingInfoToSYS.TargetNozzleTemp1);
  }
}

void GUIControl::printSetForNotM14Right(void)
{
  GUI_WHotbedTargetTemp(SettingInfoToSYS.TargetHotbedTemp);
  feature_set_extruder_fan_speed(SettingInfoToSYS.FanSpeed);
}

void GUIControl::printSetForCavity(void)
{
  if (t_gui.target_cavity_temp_on != SettingInfoToSYS.TargetCavityOnTemp) //防止二次设置
    t_gui.target_cavity_temp_on = SettingInfoToSYS.TargetCavityOnTemp;

  if (t_gui.target_cavity_temp_on)
  {
    if (t_gui.target_cavity_temp != SettingInfoToSYS.TargetCavityTemp) //防止二次设置
    {
      t_gui.target_cavity_temp = SettingInfoToSYS.TargetCavityTemp;
      // 设置温度
      temperature_set_cavity_target(t_gui.target_cavity_temp);
    }
  }
  else
  {
    if (t_gui.target_cavity_temp != 0) //防止二次设置
    {
      t_gui.target_cavity_temp = 0;
      temperature_set_cavity_target(t_gui.target_cavity_temp);
    }
  }
}

//冷却
void GUIControl::coolDown(void)
{
  GUI_WNozzleTargetTemp(0);

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    GUI_WNozzleTargetTemp1(0);
  }

  GUI_WHotbedTargetTemp(0);
  feature_set_extruder_fan_speed(0);
}

void GUIControl::refreshGuiInfo(void)
{
  static unsigned long NextRefresh = 0;

  if (NextRefresh < xTaskGetTickCount())
  {
    t_gui.is_refresh_rtc = 1;
    NextRefresh = xTaskGetTickCount() + 500;
  }

  // RefMachRunTime
  t_gui.machine_run_time = xTaskGetTickCount() / 1000;
  // UpdateGUITemp
  t_gui.nozzle_temp[0] = (int)temperature_get_extruder_current(0);
  t_gui.nozzle_temp[1] = (int)temperature_get_extruder_current(1);
  t_gui.target_nozzle_temp[0] = (int)temperature_get_extruder_target(0);
  t_gui.target_nozzle_temp[1] = (int)temperature_get_extruder_target(1);

  if (!t_custom_services.disable_hot_bed)
  {
    t_gui.hot_bed_temp = (int)temperature_get_bed_current();
    t_gui.target_hot_bed_temp = (int)temperature_get_bed_target();
  }

  t_gui.target_cavity_temp = (int) temperature_get_cavity_target();
  t_gui.cavity_temp = (int)temperature_get_cavity_current();
  // UpdateGUIPrintSpeed
  t_gui.print_speed_value = GetFeedMultiply();
  // UpdateGUIFanSpeed
  t_gui.fan_speed_value = feature_get_extruder_fan_speed();
}

GUIControl guiControl;


