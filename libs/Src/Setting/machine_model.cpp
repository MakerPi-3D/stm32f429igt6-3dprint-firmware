#include "machine_model.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include <stdio.h>
#include "Configuration.h"
#include "interface.h"
#include "user_ccm.h"
#include "flashconfig.h"
#include "gcode.h"
#include "ConfigurationStore.h"
#include "planner.h"
#ifdef __cplusplus
extern "C" {
#endif


static void machine_model_param_init(void);         ///< 机器型号参数初始化
static void machine_model_size_init(void);          ///< 机器型号尺寸初始化
static void machine_model_move_max_pos_init(void);  ///< 机器型号最大移动位置初始化
static void machine_model_home_dir_init(void);      ///< 机器型号归零方向初始化
static void machine_model_home_pos_init(void);      ///< 机器型号归零位置初始化
static void machine_model_other_init(void);         ///< 机器型号其他初始化
static void machine_model_log(void);                ///< 机器型号串口输出配置

extern void temperature_set_heater_maxtemp(int axis, int value);

void machine_model_init(void)
{
  machine_model_param_init();

  if (1 == t_sys_data_current.enable_color_mixing)
    ccm_param::motion_3d.axis_num = MAX_NUM_AXIS; // 混色机型为5轴

  if (1 == t_sys_data_current.enable_powerOff_recovery)
    ccm_param::motion_3d.disable_z_max_limit = false; //断电续打必须有下限位开关

  if (1 == t_sys_data_current.have_set_machine)
    ccm_param::motion_3d.enable_board_test = false; // 已经设置好机型，关闭电路板测试功能

  machine_model_size_init();
  machine_model_move_max_pos_init();
  machine_model_home_dir_init();
  machine_model_home_pos_init();
  machine_model_other_init();
  machine_model_log();
  ccm_param::motion_3d_model.extrude_maxlength = ccm_param::motion_3d_model.xyz_max_length[X_AXIS] + ccm_param::motion_3d_model.xyz_max_length[Y_AXIS];
}

static void machine_model_param_init(void)
{
  // 3d打印基本参数
  ccm_param::motion_3d.step = DEFAULT_STEP;
  ccm_param::motion_3d.axis_num = XYZ_NUM_AXIS;
  ccm_param::motion_3d.enable_check_door_open = false;
  ccm_param::motion_3d.disable_z_max_limit = true;
  ccm_param::motion_3d.enable_poweroff_up_down_min_min = false;
  ccm_param::motion_3d.enable_board_test = false;
  // 定制服务参数
  t_custom_services.disable_abs = false;
  t_custom_services.disable_hot_bed = false;
  t_custom_services.enable_warning_light = false;
  t_custom_services.enable_led_light = false;
  // 3d打印机型配置信息
  // 反转轴初始化
  ccm_param::motion_3d_model.enable_invert_dir[X_AXIS] = INVERT_X_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS] = INVERT_Y_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[Z_AXIS] = INVERT_Z_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[Z2_AXIS] = INVERT_Z2_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = INVERT_E0_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = INVERT_E1_DIR;
  ccm_param::motion_3d_model.enable_invert_dir[X2_AXIS] = INVERT_X2_DIR;
  // 最大位置初始化
  ccm_param::motion_3d_model.xyz_max_pos[X_AXIS] = X_MAX_POS;
  ccm_param::motion_3d_model.xyz_max_pos[Y_AXIS] = Y_MAX_POS;
  ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS] = Z_MAX_POS;
  ccm_param::motion_3d_model.xyz_max_pos[X2_AXIS] = X_MAX_POS;
  ccm_param::motion_3d_model.xyz_max_pos[Z2_AXIS] = Z2_MAX_POS;
  // 最小位置初始化
  ccm_param::motion_3d_model.xyz_min_pos[X_AXIS] = X_MIN_POS;
  ccm_param::motion_3d_model.xyz_min_pos[Y_AXIS] = Y_MIN_POS;
  ccm_param::motion_3d_model.xyz_min_pos[Z_AXIS] = Z_MIN_POS;
  ccm_param::motion_3d_model.xyz_min_pos[X2_AXIS] = X_MIN_POS;
  ccm_param::motion_3d_model.xyz_min_pos[Z2_AXIS] = Z2_MIN_POS;
  // 归零位置初始化
  ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] = X_MIN_POS;
  ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] = Y_MIN_POS;
  ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] = Z_MIN_POS;
  ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = X_MAX_POS;
  ccm_param::motion_3d_model.xyz_home_pos[Z2_AXIS] = Z2_MIN_POS;
  // 最大行程初始化
  ccm_param::motion_3d_model.xyz_max_length[X_AXIS] = X_MAX_LENGTH;
  ccm_param::motion_3d_model.xyz_max_length[Y_AXIS] = Y_MAX_LENGTH;
  ccm_param::motion_3d_model.xyz_max_length[Z_AXIS] = Z_MAX_LENGTH;
  ccm_param::motion_3d_model.xyz_max_length[X2_AXIS] = X_MAX_LENGTH;
  ccm_param::motion_3d_model.xyz_max_length[Z2_AXIS] = Z2_MAX_LENGTH;
  // 归零E电机反抽长度
  ccm_param::motion_3d_model.xyz_home_retract_mm[X_AXIS] = X_HOME_RETRACT_MM;
  ccm_param::motion_3d_model.xyz_home_retract_mm[Y_AXIS] = Y_HOME_RETRACT_MM;
  ccm_param::motion_3d_model.xyz_home_retract_mm[Z_AXIS] = Z_HOME_RETRACT_MM;
  ccm_param::motion_3d_model.xyz_home_retract_mm[X2_AXIS] = X_HOME_RETRACT_MM;
  ccm_param::motion_3d_model.xyz_home_retract_mm[Z2_AXIS] = Z2_HOME_RETRACT_MM;
  // 归零方向初始化
  ccm_param::motion_3d_model.xyz_home_dir[X_AXIS] = X_HOME_DIR;
  ccm_param::motion_3d_model.xyz_home_dir[Y_AXIS] = Y_HOME_DIR;
  ccm_param::motion_3d_model.xyz_home_dir[Z_AXIS] = Z_HOME_DIR;
  ccm_param::motion_3d_model.xyz_home_dir[X2_AXIS] = X2_HOME_DIR;
  ccm_param::motion_3d_model.xyz_home_dir[Z2_AXIS] = Z2_HOME_DIR;
  // 最大移动位置初始化
  ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS] = X_MAX_POS;
  ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS] = Y_MAX_POS;
  ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS] = Z_MAX_POS;
  ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS] = X_MAX_POS;
  ccm_param::motion_3d_model.xyz_move_max_pos[Z2_AXIS] = Z_MAX_POS;
  // Z原始最大位置初始化
  ccm_param::motion_3d_model.z_max_pos_origin = Z_MAX_POS;
}

static void machine_model_size_init(void)
{
  //根据model, 查表获取机型尺寸
  for (int i = 0; i < XYZ_NUM_AXIS; i++)
    ccm_param::motion_3d_model.xyz_max_pos[i] = (model_size_table[t_sys_data_current.model_id][i]);

  // 保存初始Z最大位置
  ccm_param::motion_3d_model.z_max_pos_origin = ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS];

  // 自动调平高度可能小于默认最大高度，忽略修改
  if (t_sys_data_current.enable_powerOff_recovery)
  {
    if (t_sys_data_current.poweroff_rec_z_max_value > ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS] - CAL_Z_MAX_POS_OFFSET //除去+8，2017.10.8  //加8是防止校准值小于默认值太多，引起最大行程警报，2017.7.14john
        || 1 == t_sys_data_current.enable_bed_level)
      ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS] = t_sys_data_current.poweroff_rec_z_max_value;
    else
      t_sys_data_current.poweroff_rec_z_max_value = ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS];
  }

  for (int i = 0; i < XYZ_NUM_AXIS; i++)
    ccm_param::motion_3d_model.xyz_max_length[i] = ccm_param::motion_3d_model.xyz_max_pos[i];
}

static void machine_model_move_max_pos_init(void)
{
  if (K5 == t_sys_data_current.model_id)     //M2030HY实际行程200 可调行程308
  {
    ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)(ccm_param::motion_3d_model.xyz_max_pos[X_AXIS] - 3);
    ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS] = (int)(ccm_param::motion_3d_model.xyz_max_pos[X2_AXIS] - 3);
    ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)(ccm_param::motion_3d_model.xyz_max_pos[Y_AXIS] - 3);
    ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)(ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS]);
    ccm_param::motion_3d_model.xyz_move_max_pos[Z2_AXIS] = (int)(ccm_param::motion_3d_model.xyz_max_pos[Z2_AXIS]);
  }
  else
  {
    ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)ccm_param::motion_3d_model.xyz_max_pos[X_AXIS];
    ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS] = (int)ccm_param::motion_3d_model.xyz_max_pos[X2_AXIS];
    ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)ccm_param::motion_3d_model.xyz_max_pos[Y_AXIS];
    ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS];
    ccm_param::motion_3d_model.xyz_move_max_pos[Z2_AXIS] = (int)ccm_param::motion_3d_model.xyz_max_pos[Z2_AXIS];
  }
}


static void machine_model_home_dir_init(void)
{
  // K5机型X轴Home方向与普通机器相反，会导致打印模型X轴镜像
  // 正常机器home方向为Max-》Min
  // K5机器home方向为Min-》Max
  if (t_sys_data_current.model_id == K5)// || (t_sys_data_current.model_id == F400TP))
  {
    ccm_param::motion_3d_model.xyz_home_dir[X_AXIS] = 1;
  }
}

static void machine_model_home_pos_init(void)
{
  if (ccm_param::motion_3d_model.xyz_home_dir[X_AXIS] == -1)
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] = ccm_param::motion_3d_model.xyz_max_length[X_AXIS] * -0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] = X_MIN_POS;
    #endif //BED_CENTER_AT_0_0
  }
  else
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] = ccm_param::motion_3d_model.xyz_max_length[X_AXIS] * 0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS];//maxPos[0];
    #endif //BED_CENTER_AT_0_0
  }

  if (ccm_param::motion_3d_model.xyz_home_dir[X2_AXIS] == -1)
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = ccm_param::motion_3d_model.xyz_max_length[X2_AXIS] * -0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = X_MIN_POS;
    #endif //BED_CENTER_AT_0_0
  }
  else
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = ccm_param::motion_3d_model.xyz_max_length[X2_AXIS] * 0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS];//maxPos[0];
    ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] = flash_param_t.idex_extruder_0_bed_offset[1] +
        (ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS] - flash_param_t.idex_extruder_1_bed_offset[1]) -
        flash_param_t.idex_extruder_1_bed_offset[0];
    #endif //BED_CENTER_AT_0_0
  }

  if (ccm_param::motion_3d_model.xyz_home_dir[Y_AXIS] == -1)
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] = ccm_param::motion_3d_model.xyz_max_length[Y_AXIS] * -0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] = Y_MIN_POS;
    #endif //BED_CENTER_AT_0_0
  }
  else
  {
    #ifdef BED_CENTER_AT_0_0
    ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] = ccm_param::motion_3d_model.xyz_max_length[Y_AXIS] * 0.5;
    #else
    ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS]; //maxPos[1];
    #endif //BED_CENTER_AT_0_0
  }

  if (ccm_param::motion_3d_model.xyz_home_dir[Z_AXIS] == -1)
  {
    ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] = Z_MIN_POS;
  }
  else
  {
    ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS]; //maxPos[2];
  }

  if (ccm_param::motion_3d_model.xyz_home_dir[Z2_AXIS] == -1)
  {
    ccm_param::motion_3d_model.xyz_home_pos[Z2_AXIS] = Z2_MIN_POS;
  }
  else
  {
    ccm_param::motion_3d_model.xyz_home_pos[Z2_AXIS] = ccm_param::motion_3d_model.xyz_move_max_pos[Z2_AXIS]; //maxPos[2];
  }
}

static void machine_model_other_init(void)
{
  // 腔体温度与断料、堵料检测冲突
  if (1 == t_sys.enable_cavity_temp)
  {
    t_sys_data_current.enable_material_check = false;
  }

  // 如果沒有門檢測和斷電開啓，强制開啓上下共限位，兼容電路板兩種接法：
  // 1、老板衹有Zmax；2、新板door占用Zmax，Zmax、Zmin共限位
  if (t_sys_data_current.enable_powerOff_recovery && !ccm_param::motion_3d.enable_check_door_open)
  {
    ccm_param::motion_3d.enable_poweroff_up_down_min_min = true;
    ccm_param::motion_3d.updown_g28_first_time = 1;
    t_power_off.is_power_off = 0; // 设置非断电状态
  }
  else if (t_sys_data_current.enable_powerOff_recovery && ccm_param::motion_3d.enable_check_door_open)
  {
  }

  if (t_sys_data_current.model_id == P2_Pro)
  {
    ccm_param::motion_3d_model.enable_invert_dir[X_AXIS] = true;
    ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS] = true;
    t_sys.lcd_ssd1963_43_480_272 = 1;
    t_sys_data_current.enable_color_mixing = true;
    flash_param_t.extruder_type = EXTRUDER_TYPE_DUAL;
    t_sys.lcd_type = LCD_TYPE_43_480272_SIZE;
    ccm_param::motion_3d.disable_z_max_limit = true;
  }

  if (t_sys_data_current.model_id == P3_Pro)
  {
    ccm_param::motion_3d_model.enable_invert_dir[X_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[X2_AXIS] = true;
    ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[Z_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[Z2_AXIS] = false;
    t_sys.lcd_ssd1963_43_480_272 = 1;
    t_sys_data_current.enable_color_mixing = true;
    t_sys.lcd_type = LCD_TYPE_43_480272_SIZE;
    ccm_param::motion_3d.disable_z_max_limit = true;
    ccm_param::motion_3d.enable_poweroff_up_down_min_min = false;
    temperature_set_heater_maxtemp(0, 325);
    temperature_set_heater_maxtemp(1, 325);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      t_sys.is_idex_extruder = 1;
      gcode::active_extruder = 0;
      ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = false;
      ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = true;
      planner_settings.axis_steps_per_mm[E_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_IDEX;
      planner_settings.axis_steps_per_mm[B_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_IDEX;
      axis_steps_per_sqr_second[E_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[E_AXIS] * planner_settings.axis_steps_per_mm[E_AXIS]);
      axis_steps_per_sqr_second[B_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[B_AXIS] * planner_settings.axis_steps_per_mm[B_AXIS]);
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      gcode::active_extruder = 0;
      ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
      ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = false;
      planner_settings.axis_steps_per_mm[E_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_MIX;
      planner_settings.axis_steps_per_mm[B_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_MIX;
      axis_steps_per_sqr_second[E_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[E_AXIS] * planner_settings.axis_steps_per_mm[E_AXIS]);
      axis_steps_per_sqr_second[B_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[B_AXIS] * planner_settings.axis_steps_per_mm[B_AXIS]);
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      gcode::active_extruder = 1;// 激光头使用喷头2
    }
    else
    {
      flash_param_t.extruder_type = EXTRUDER_TYPE_DUAL;
      t_sys.is_idex_extruder = 1;
      ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
      flash_param_t.flag = 1;
    }

    #if defined(TYPE_IDEX)
    //idex结构
    flash_param_t.extruder_type = EXTRUDER_TYPE_DUAL;
    t_sys.is_idex_extruder = 1;
    ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
    #elif defined(TYPE_MIX)
    //混色结构
    flash_param_t.extruder_type = EXTRUDER_TYPE_MIX;
    #endif
  }

  if ((K5 == t_sys_data_current.model_id))
  {
    ccm_param::motion_3d_model.enable_invert_dir[X_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS] = true;
    t_custom_services.enable_led_light = true;   //有LED灯照明功能
    t_sys.lcd_ssd1963_43_480_272 = 1;
  }

  if (F400TP == t_sys_data_current.model_id)
  {
    ccm_param::motion_3d_model.enable_invert_dir[X_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[X2_AXIS] = true;
    ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS] = true;
    ccm_param::motion_3d_model.enable_invert_dir[Z_AXIS] = true;
    ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = false;
    ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = false;
    t_custom_services.enable_led_light = true;   //有LED灯照明功能
    t_sys.lcd_ssd1963_43_480_272 = 1;
    t_sys_data_current.enable_color_mixing = true;
    ccm_param::motion_3d.disable_z_max_limit = true;
    ccm_param::motion_3d.enable_poweroff_up_down_min_min = false;
    t_sys.lcd_type = LCD_TYPE_7_1024600_SIZE;
    //idex结构
    //   if (flash_param_t.extruder_type != EXTRUDER_TYPE_DUAL)
    {
      flash_param_t.extruder_type = EXTRUDER_TYPE_DUAL;
      t_sys.is_idex_extruder = 1;
      ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
      ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = true;
      flash_param_t.flag = 1;
    }
  }
}

static void machine_model_log(void)
{
  #if (1 == DEBUG_SERVICES_CUSTOM)
  USER_DbgLog("%40s", "########custom services start########");
  USER_DbgLog("%40s = %d", "step", ccm_param::motion_3d.step);
  USER_DbgLog("%40s = %d", "axisNum", ccm_param::motion_3d.axis_num);
  USER_DbgLog("%40s = %d", "enableInvertXDir", ccm_param::motion_3d_model.enable_invert_dir[X_AXIS]);
  USER_DbgLog("%40s = %d", "enableInvertYDir", ccm_param::motion_3d_model.enable_invert_dir[Y_AXIS]);
  USER_DbgLog("%40s = %d", "enableInvertZDir", ccm_param::motion_3d_model.enable_invert_dir[Z_AXIS]);
  USER_DbgLog("%40s = %d", "enableInvertE0Dir", ccm_param::motion_3d_model.enable_invert_dir[E_AXIS]);
  USER_DbgLog("%40s = %d", "enableInvertE1Dir", ccm_param::motion_3d_model.enable_invert_dir[B_AXIS]);
  USER_DbgLog("%40s = %d", "enableInvertX2Dir", ccm_param::motion_3d_model.enable_invert_dir[X2_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "maxPos", ccm_param::motion_3d_model.xyz_max_pos[X_AXIS], ccm_param::motion_3d_model.xyz_max_pos[X2_AXIS], ccm_param::motion_3d_model.xyz_max_pos[Y_AXIS], ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "minPos", ccm_param::motion_3d_model.xyz_min_pos[X_AXIS], ccm_param::motion_3d_model.xyz_min_pos[X2_AXIS], ccm_param::motion_3d_model.xyz_min_pos[Y_AXIS], ccm_param::motion_3d_model.xyz_min_pos[Z_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "homePos", ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS], ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS], ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "maxLength", ccm_param::motion_3d_model.xyz_max_length[X_AXIS], ccm_param::motion_3d_model.xyz_max_length[X2_AXIS], ccm_param::motion_3d_model.xyz_max_length[Y_AXIS], ccm_param::motion_3d_model.xyz_max_length[Z_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "homeRetractMM", ccm_param::motion_3d_model.xyz_home_retract_mm[X_AXIS], ccm_param::motion_3d_model.xyz_home_retract_mm[X2_AXIS], ccm_param::motion_3d_model.xyz_home_retract_mm[Y_AXIS], ccm_param::motion_3d_model.xyz_home_retract_mm[Z_AXIS]);
  USER_DbgLog("%40s = %6.2f,%6.2f,%6.2f,%6.2f", "moveMaxPos", ccm_param::motion_3d_model.xyz_move_max_pos[X_AXIS], ccm_param::motion_3d_model.xyz_move_max_pos[X2_AXIS], ccm_param::motion_3d_model.xyz_move_max_pos[Y_AXIS], ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS]);
  USER_DbgLog("%40s = %d,%d,%d,%d", "homeDir", ccm_param::motion_3d_model.xyz_home_dir[X_AXIS], ccm_param::motion_3d_model.xyz_home_dir[X2_AXIS], ccm_param::motion_3d_model.xyz_home_dir[Y_AXIS], ccm_param::motion_3d_model.xyz_home_dir[Z_AXIS]);
  USER_DbgLog("%40s", "########custom services end########");
  #endif
}

#ifdef __cplusplus
} //extern "C" {
#endif


