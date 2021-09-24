#include "controlxyz.h"
#include "planner.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include "config_motion_3d.h"
#include "user_ccm.h"
#include "user_common.h"
#include "sysconfig_data.h"
#include "flashconfig.h"
#ifdef __cplusplus
extern "C" {
#endif

static char set_xy_pos[96];
static char home_xy_pos[96];

static int set_x_pos;
static int home_x_distance;
//static int home_x_pos;
static int home_x_slow_offset;

static int set_x2_pos;
static int home_x2_distance;
//static int home_x2_pos;
//static int home_x2_slow_offset;

static int set_y_pos;
static int home_y_distance;
//static int home_y_pos;
static int home_y_slow_offset;

void control_xyz_init(void)
{
  if (1 == ccm_param::motion_3d_model.xyz_home_dir[X_AXIS])
  {
    home_x_distance = 999;
    set_x_pos = 0;
    home_x_slow_offset = home_x_distance - 5;
  }
  else
  {
    home_x_distance = 0;
    set_x_pos = 999;
    home_x_slow_offset = home_x_distance + 5;
  }

  if (1 == ccm_param::motion_3d_model.xyz_home_dir[Y_AXIS])
  {
    home_y_distance = 999;
    set_y_pos = 0;
    home_y_slow_offset = home_y_distance - 5;
  }
  else
  {
    home_y_distance = 0;
    set_y_pos = 999;
    home_y_slow_offset = home_y_distance + 5;
  }

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
  {
    if (1 == ccm_param::motion_3d_model.xyz_home_dir[X2_AXIS])
    {
      home_x2_distance = 999;
      set_x2_pos = 0;
      //      home_x2_slow_offset = home_x2_distance - 5;
    }
    else
    {
      home_x2_distance = 0;
      set_x2_pos = 999;
      //      home_x2_slow_offset = home_x2_distance + 5;
    }
  }

  //  home_x_pos = ccm_param::motion_3d_model.xyz_home_pos[0];
  //  home_y_pos = ccm_param::motion_3d_model.xyz_home_pos[1];
}

// 平台相对当前位置下降60mm，XY归零
void z_down_60mm_and_xy_to_zero(void)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.zDown60MM();
  controlXYZEB.xyMoveToZero();
}

void z_down_to_bottom_and_xy_to_zero(void)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.zDownToBottom();
  controlXYZEB.xyMoveToZero();
}

void xy_to_zero(void)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.xyMoveToZero();
}

void z_down_to_bottom(void)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.zDownToBottom();
}

void z_check_and_set_bottom(const bool isUpDownMinMin, const float zBottomValue)
{
  ControlXYZEB controlXYZEB;
  //  controlXYZEB.zCheckTheBottom(isUpDownMinMin);
  controlXYZEB.g92SetAxisPosition(3, zBottomValue);
}

void m84_disable_steppers(void)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.m84DisableXYEB();
}

void g92_set_axis_position(const int axis, const float value)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.g92SetAxisPosition(axis, value);
}

void eb_compensate_8mm(bool isColorMix)
{
  ControlXYZEB controlXYZEB;
  controlXYZEB.ebCompensate8mm(isColorMix);
}

static void send_cmd_delay(void)
{
  while (planner_moves_planned() > 0)
  {
    user_iwdg_refresh(); // ref_data_task任务执行循环延时，需要喂狗，避免触发看门狗
    OS_DELAY(100);  // 延时
  }

  OS_DELAY(50);  // 延时
}

#ifdef __cplusplus
} //extern "C" {
#endif

ControlXYZEB::ControlXYZEB()
{
}

// 平台相对当前位置下降60mm，发gcode指令
void ControlXYZEB::zDown60MM(void)
{
  user_send_internal_cmd((char *)"G91");                 // 开启相对模式
  user_send_internal_cmd((char *)"M121");                // 开启限位检测
  user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  user_send_internal_cmd((char *)"G1 F600 Z+60");        // Z增加60mm
  user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
  user_send_internal_cmd((char *)"M120");                // 关闭限位检测
  user_send_internal_cmd((char *)"G90");                 // 关闭相对模式
  send_cmd_delay();
}

void xy_move_to_home(int xy_slow_speed, int x_slow_offset, int y_slow_offset)
{
  user_send_internal_cmd((char *)"M2003 S0");    // 关闭坐标转换
  // XY快速归零
  memset(set_xy_pos, 0, sizeof(set_xy_pos));
  sprintf(set_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS]+999, (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]+999);
  user_send_internal_cmd((char *)set_xy_pos); // 设置当前位置
  memset(home_xy_pos, 0, sizeof(home_xy_pos));
  sprintf(home_xy_pos, "G1 F2400 X%d Y%d D1", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
  user_send_internal_cmd((char *)home_xy_pos); // X Y轴归零

  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
  {
    user_send_internal_cmd((char *)"T0 S-1");
    // X单轴归零
    memset(set_xy_pos, 0, sizeof(set_xy_pos));
    sprintf(set_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS]+999, (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)set_xy_pos); // 设置X当前位置
    memset(home_xy_pos, 0, sizeof(home_xy_pos));
    sprintf(home_xy_pos, "G1 F%d X%d Y%d D1", xy_slow_speed * 60, (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)home_xy_pos); // X轴归零
    user_send_internal_cmd((char *)"T1 S-1");
    // X2单轴归零
    memset(set_xy_pos, 0, sizeof(set_xy_pos));
    sprintf(set_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS]-999, (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)set_xy_pos); // 设置X2当前位置
    memset(home_xy_pos, 0, sizeof(home_xy_pos));
    sprintf(home_xy_pos, "G1 F%d X%d Y%d D1", xy_slow_speed * 60, (int)ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)home_xy_pos); // X2轴归零
    user_send_internal_cmd((char *)"T0 S-1");
  }
  else
  {
    // X单轴归零
    memset(set_xy_pos, 0, sizeof(set_xy_pos));
    sprintf(set_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS]+999, (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)set_xy_pos); // 设置X当前位置
    memset(home_xy_pos, 0, sizeof(home_xy_pos));
    sprintf(home_xy_pos, "G1 F%d X%d Y%d D1", xy_slow_speed * 60, (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]);
    user_send_internal_cmd((char *)home_xy_pos);  // X轴归零
  }

  // Y单轴归零
  memset(set_xy_pos, 0, sizeof(set_xy_pos));
  sprintf(set_xy_pos, "G92 X%d Y%d", (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS]+999);
  user_send_internal_cmd((char *)set_xy_pos);  // 设置Y当前位置
  memset(home_xy_pos, 0, sizeof(home_xy_pos));
  sprintf(home_xy_pos, "G1 F%d X%d Y%d D1", xy_slow_speed * 60, (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS], (int)ccm_param::motion_3d_model.xyz_home_pos[X_AXIS]);
  user_send_internal_cmd((char *)home_xy_pos);  // Y轴归零
  user_send_internal_cmd((char *)"M2003 S1");   // 开启坐标转换
}


void ControlXYZEB::xyMoveToZero(void)
{
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
  {
    user_send_internal_cmd((char *)"T0 S-1");
  }

  user_send_internal_cmd((char *)"M121");                     // 开启限位检测
  xy_move_to_home(20, home_x_distance, home_y_distance);      // 快速归零
  xy_move_to_home(5, home_x_slow_offset, home_y_slow_offset); // 慢速归零
  user_send_internal_cmd((char *)"G92 X0 Y0");
  user_send_internal_cmd((char *)"M120");                     // 关闭限位检测
}

void ControlXYZEB::zDownToBottom(void)
{
  user_send_internal_cmd((char *)"M121");                // 打开限位检测
  user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  user_send_internal_cmd((char *)"G1 Z999 D1");          // 平台下降命令
  user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
  user_send_internal_cmd((char *)"M120");                // 关闭限位检测
  OS_DELAY(100);                                                                             // 延时
}

void ControlXYZEB::m84DisableXYEB(void)
{
  user_send_internal_cmd((char *)"M84 X Y E B");         // 解锁XYEB
}

void ControlXYZEB::zCheckTheBottom(const bool isUpDownMinMin)
{
  //  static char gcodeG92CommandBuf[50] = {0};
  // 上下共限位
  // 上电过程中，平台下降撞击限位，可能会出现限位压得比较死
  // 需要重新确认限位状态，避免续打z高度异常
  //  if (isUpDownMinMin)
  //  {
  //    send_cmd_delay();
  //    user_send_internal_cmd((char *)"M120");                // 关闭限位检测
  //    user_send_internal_cmd((char *)"G91");                 // 开启绝对模式
  //    user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  //    user_send_internal_cmd((char *)"G1 F120 Z-5 D1");      // 平台向上提升5mm
  //    user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
  //    OS_DELAY(100);                                                                         // 延时
  //    user_send_internal_cmd((char *)"M121");                // 开启限位检测
  //    user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  //    user_send_internal_cmd((char *)"G1 F120 Z+10 D1");     // 平台向下降10mm，直到撞击限位
  //    user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
  //    OS_DELAY(100);                                                                             // 延时
  //    user_send_internal_cmd((char *)"G90");                 // 关闭绝对模式
  //    user_send_internal_cmd((char *)"M120");                // 关闭限位检测
  //    user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  //    memset(gcodeG92CommandBuf, 0, sizeof(gcodeG92CommandBuf));
  //    (void)snprintf(gcodeG92CommandBuf, sizeof(gcodeG92CommandBuf), "G92 X%f Y%f", ccm_param::motion_3d_model.xyz_home_pos[0], ccm_param::motion_3d_model.xyz_home_pos[1]);
  //    user_send_internal_cmd((char *)gcodeG92CommandBuf);   // Y轴归零
  //    user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
  //    send_cmd_delay();
  //  }
  //  else
  {
    send_cmd_delay();
    user_send_internal_cmd((char *)"M121");
    user_send_internal_cmd((char *)"G91 ");
    user_send_internal_cmd((char *)"G1 F120 Z+999 D1");
    user_send_internal_cmd((char *)"G90");
    user_send_internal_cmd((char *)"M120");
    //     user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
    //    memset(gcodeG92CommandBuf, 0, sizeof(gcodeG92CommandBuf));
    //    (void)snprintf(gcodeG92CommandBuf, sizeof(gcodeG92CommandBuf), "G92 X%f Y%f", ccm_param::motion_3d_model.xyz_home_pos[0], ccm_param::motion_3d_model.xyz_home_pos[1]);
    //    user_send_internal_cmd((char *)gcodeG92CommandBuf);   // Y轴归零
    //    user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
    send_cmd_delay();
  }
}

void ControlXYZEB::g92SetAxisPosition(const int axis, const float value)
{
  static char gcodeCmdBuf[5][40] = {0};
  user_send_internal_cmd((char *)"M2003 S0");            // 关闭坐标转换
  memset(gcodeCmdBuf[axis], 0, sizeof(gcodeCmdBuf[axis]));
  (void)snprintf(gcodeCmdBuf[axis], sizeof(gcodeCmdBuf[axis]), "G92 %c%f", axis_codes[axis], value); // 设置XYZEB坐标
  user_send_internal_cmd((char *)gcodeCmdBuf[axis]); //E位置
  user_send_internal_cmd((char *)"M2003 S1");            // 开启坐标转换
}

void ControlXYZEB::ebCompensate8mm(bool isColorMix)
{
  user_send_internal_cmd((char *)"M2003 S0");    // 关闭坐标转换
  user_send_internal_cmd((char *)"G92 E0 B0");          // 设置eb位置

  if (isColorMix)
  {
    user_send_internal_cmd((char *)"G1 F150 E8 B8 D1");      // eb各自运动8mm
  }
  else
  {
    user_send_internal_cmd((char *)"G1 F150 E16 B0 D1");      // eb各自运动8mm
  }

  user_send_internal_cmd((char *)"G92 E0 B0");          // 重新设置eb位置
  user_send_internal_cmd((char *)"M2003 S1");    // 开启坐标转换
}

