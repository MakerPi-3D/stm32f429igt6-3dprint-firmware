#include "user_ccm.h"
#include "planner.h"
#include "stepper.h"
#include "process_command.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "flashconfig.h"
#include "config_model_tables.h"
#include "Configuration.h"
#include "gcodebufferhandle.h"
#include "threed_engine.h"
#include "globalvariables.h"
#include "gcode.h"
#include "stepper_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

extern float add_homeing[XYZ_NUM_AXIS];
extern unsigned long previous_xTaskGetTickCount_cmd;

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gcode
{
  volatile bool g28_complete_flag = false;

  static volatile bool home_all_axis = false;
  static volatile bool home_hit_z_endstop = false;
  static volatile bool home_code_seen_xyz[XYZ_NUM_AXIS] = {false};
  static volatile float home_code_value_xyz[XYZ_NUM_AXIS] = {0.0f};
  static volatile bool level_is_rise_z = false; //判断调平模式是否抬升Z轴

  static void g28_set_curr_position(void)
  {
    for (int i = 0; i < MAX_NUM_AXIS; i++)
    {
      ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
    }

    planner_set_position(ccm_param::grbl_current_position);
    st_synchronize();
  }

  // Z激光限位，先上升Z，移动xy，再Z归零
  static void laser_z_rise_first(void)
  {
    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    {
      if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        if (!level_is_rise_z)
        {
          g28_set_curr_position();
          planner_set_position(ccm_param::grbl_current_position);
          st_synchronize();
          st_enable_endstops(false);
          ccm_param::grbl_destination[(int)Z_AXIS] = ccm_param::grbl_current_position[Z_AXIS] + 5;

          if (ccm_param::grbl_destination[Z_AXIS] > ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS] + add_homeing[Z_AXIS])
          {
            ccm_param::grbl_destination[Z_AXIS] = ccm_param::motion_3d_model.xyz_max_pos[Z_AXIS] + add_homeing[Z_AXIS];
          }

          process_buffer_line_normal(ccm_param::grbl_destination, homing_feedrate[Z_AXIS] / 60);
          st_synchronize();
          st_enable_endstops(true);
          ccm_param::grbl_destination[(int)Z_AXIS] = ccm_param::grbl_current_position[Z_AXIS];
          planner_set_position(ccm_param::grbl_current_position);
          st_synchronize();
          level_is_rise_z = true;
        }
      }
    }
  }

  static void home_quick(void)
  {
    #ifdef QUICK_HOME
    float quit_home_feedrate = homing_feedrate[X_AXIS];

    if ((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS]) && parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS]))) //first diagonal move
    {
      // 设置当前点XY为零点
      ccm_param::grbl_destination[(int)X_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS];
      ccm_param::grbl_destination[(int)Y_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS];
      g28_set_curr_position();
      laser_z_rise_first();
      st_enable_endstops(true);
      // xy再次往负方向移动，直至撞击限位
      ccm_param::grbl_destination[(int)X_AXIS] = 1.5f * ccm_param::motion_3d_model.xyz_max_length[(int)X_AXIS] * ccm_param::motion_3d_model.xyz_home_dir[(int)X_AXIS];
      ccm_param::grbl_destination[(int)Y_AXIS] = 1.5f * ccm_param::motion_3d_model.xyz_max_length[(int)Y_AXIS] * ccm_param::motion_3d_model.xyz_home_dir[(int)Y_AXIS];
      quit_home_feedrate = homing_feedrate[X_AXIS];

      if (homing_feedrate[Y_AXIS] < quit_home_feedrate)
      {
        quit_home_feedrate = homing_feedrate[Y_AXIS];
      }

      process_buffer_line_normal(ccm_param::grbl_destination, quit_home_feedrate / 60);
      st_synchronize();
      st_enable_endstops(false);
      ccm_param::grbl_current_position[X_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS]; // axis_is_at_home
      ccm_param::grbl_current_position[Y_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS]; // axis_is_at_home
      planner_set_position(ccm_param::grbl_current_position);
      st_synchronize();
      st_enable_endstops(true);
      ccm_param::grbl_destination[X_AXIS] = ccm_param::grbl_current_position[X_AXIS];
      ccm_param::grbl_destination[Y_AXIS] = ccm_param::grbl_current_position[Y_AXIS];
      process_buffer_line_normal(ccm_param::grbl_destination, quit_home_feedrate / 60);
      st_synchronize();
      st_enable_endstops(false);

      for (int i = 0; i < XYZ_NUM_AXIS; i++)
      {
        ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
      }

      planner_set_position(ccm_param::grbl_current_position);
      st_synchronize();
    }

    #endif
  }

  void g28_get_home_pos_adding(const int dir, float &x, float &y, float &z)
  {
    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      if (P2_Pro == t_sys_data_current.model_id)
      {
        x = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS] -
            dir * flash_param_t.dual_home_pos_adding[0]; // axis_is_at_home
        y = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS] -
            dir * flash_param_t.dual_home_pos_adding[1]; // axis_is_at_home
        z = ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] + add_homeing[Z_AXIS] -
            dir * flash_param_t.dual_home_pos_adding[2]; // axis_is_at_home
      }
      else
      {
        x = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS] -
            dir * flash_param_t.idex_ext0_home_pos_adding[0]; // axis_is_at_home
        y = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS] -
            dir * flash_param_t.idex_ext0_home_pos_adding[1]; // axis_is_at_home
        z = ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] + add_homeing[Z_AXIS] -
            dir * flash_param_t.idex_ext0_home_pos_adding[2]; // axis_is_at_home
      }
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      x = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS] -
          dir * flash_param_t.mix_ext0_home_pos_adding[0]; // axis_is_at_home
      y = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS] -
          dir * flash_param_t.mix_ext0_home_pos_adding[1]; // axis_is_at_home
      z = ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] + add_homeing[Z_AXIS] -
          dir * flash_param_t.mix_ext0_home_pos_adding[2]; // axis_is_at_home
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      x = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS] + add_homeing[X_AXIS] -
          dir * flash_param_t.laser_ext0_home_pos_adding[0]; // axis_is_at_home
      y = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS] + add_homeing[Y_AXIS] -
          dir * flash_param_t.laser_ext0_home_pos_adding[1]; // axis_is_at_home
      z = ccm_param::motion_3d_model.xyz_home_pos[Z_AXIS] + add_homeing[Z_AXIS] -
          dir * flash_param_t.laser_ext0_home_pos_adding[2]; // axis_is_at_home
    }
    else
    {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
    }
  }

  // Z归零时，xy偏移，确保Z处于平台限位范围
  void home_pos_adding_move(const int dir)
  {
    float x, y, z;
    g28_get_home_pos_adding(dir, x, y, z);
    ccm_param::grbl_current_position[X_AXIS] = x;
    ccm_param::grbl_current_position[X2_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];
    ccm_param::grbl_current_position[Y_AXIS] = y;
    planner_set_position(ccm_param::grbl_current_position);
    st_synchronize();
    st_enable_endstops(false);
    ccm_param::grbl_destination[X_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X_AXIS];
    ccm_param::grbl_destination[Y_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[Y_AXIS];
    ccm_param::grbl_destination[X2_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];;
    process_buffer_line_normal(ccm_param::grbl_destination, homing_feedrate[X_AXIS] / 60);
    st_enable_endstops(true);
    st_synchronize();
    ccm_param::grbl_current_position[X_AXIS] = ccm_param::grbl_destination[X_AXIS];
    ccm_param::grbl_current_position[Y_AXIS] = ccm_param::grbl_destination[Y_AXIS];
  }

  void home_axis_move_to(const int axis, bool is_sync_z)
  {
    const int axis_home_dir = ccm_param::motion_3d_model.xyz_home_dir[axis];
    const float axis_feedrate = homing_feedrate[axis];
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    ccm_param::grbl_current_position[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    planner_set_position_basic(ccm_param::grbl_current_position, is_sync_z);
    st_synchronize();
    st_enable_endstops(true);
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis] + 1.5f * ccm_param::motion_3d_model.xyz_max_length[axis] * axis_home_dir;
    process_buffer_line_basic(ccm_param::grbl_destination, axis_feedrate / 60, 100, 100, 0, is_sync_z);
    st_synchronize();
    st_enable_endstops(false);
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    ccm_param::grbl_current_position[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    planner_set_position_basic(ccm_param::grbl_current_position, is_sync_z);
    st_synchronize();
  }

  void home_axis_move_to(const int axis)
  {
    home_axis_move_to(axis, true);
  }

  void home_axis_process_basic(int axis, bool is_sync_z, float feedrate_factor, bool is_retract)
  {
    const int axis_home_dir = ccm_param::motion_3d_model.xyz_home_dir[axis];
    const float axis_feedrate = homing_feedrate[axis] * feedrate_factor;

    if (axis == Z_AXIS) // Z归零，1号头偏移到指定xy位置
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        // home_pos_adding_move(1);
      }
    }
    else if (axis == X_AXIS)
    {
      if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        home_axis_move_to(X2_AXIS);
        g28_set_curr_position();
      }
    }
    else if (axis == X2_AXIS)
    {
      if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        home_axis_move_to(X_AXIS);
        g28_set_curr_position();
      }
    }

    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    ccm_param::grbl_current_position[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    planner_set_position_basic(ccm_param::grbl_current_position, is_sync_z);
    st_synchronize();
    st_enable_endstops(true);
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis] + 1.5f * ccm_param::motion_3d_model.xyz_max_length[axis] * axis_home_dir;
    process_buffer_line_basic(ccm_param::grbl_destination, axis_feedrate / 60, 100, 100, 0, is_sync_z);
    st_synchronize();
    st_enable_endstops(false);
    ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    ccm_param::grbl_current_position[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis];
    planner_set_position_basic(ccm_param::grbl_current_position, is_sync_z);
    st_synchronize();

    if (is_retract)
    {
      ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis] - ccm_param::motion_3d_model.xyz_home_retract_mm[axis] * axis_home_dir;
      process_buffer_line_basic(ccm_param::grbl_destination, axis_feedrate / 60, 100, 100, 0, is_sync_z);
      st_synchronize();
      st_enable_endstops(true);
      ccm_param::grbl_destination[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis] + 2.0f * ccm_param::motion_3d_model.xyz_home_retract_mm[axis] * axis_home_dir;
      process_buffer_line_basic(ccm_param::grbl_destination, axis_feedrate / 2 / 60, 100, 100, 0, is_sync_z);
      st_synchronize();
    }

    st_enable_endstops(false);
    ccm_param::grbl_current_position[axis] = ccm_param::motion_3d_model.xyz_home_pos[axis] + add_homeing[axis]; // axis_is_at_home
    ccm_param::grbl_destination[axis] = ccm_param::grbl_current_position[axis];

    if (axis == Z_AXIS) // Z归零结束，1号头xy回到限位处
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        // home_pos_adding_move(-1);
      }
    }
  }

  void home_axis_process(int axis, bool is_sync_z)
  {
    home_axis_process_basic(axis, is_sync_z, 1.0f, true);
  }

  static void home_axis(void)
  {
    if ((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS])))
    {
      g28_set_curr_position();
      home_axis_process(X_AXIS, true);

      if ((flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1) ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX) // idex 双头
      {
        g28_set_curr_position();
        home_axis_process(X2_AXIS, true);
      }
    }

    if ((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS])))
    {
      g28_set_curr_position();
      home_axis_process(Y_AXIS, true);
    }

    if ((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS])))
    {
      #if (Z_HOME_DIR == 1)
      //TODO
      #elif (Z_HOME_DIR == -1)                      // If homing towards BED do Z last

      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_MIX ||
          flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
      {
        if (!home_all_axis)
        {
          if (!(parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS])))
          {
            g28_set_curr_position();
            home_axis_process(X_AXIS, true);
          }

          if (!(parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS])))
          {
            g28_set_curr_position();
            home_axis_process(Y_AXIS, true);
          }
        }
      }

      g28_set_curr_position();
      home_axis_process(Z_AXIS, true);
      osDelay(500);

      if (!stepper_axis_xyz_read_min(Z_AXIS) || !stepper_axis_xyz_read_min(Z2_AXIS))
      {
        osDelay(500);
        g28_set_curr_position();
        home_axis_process_basic(Z_AXIS, false, 0.5f, false);
        osDelay(500);
        g28_set_curr_position();
        home_axis_process_basic(Z2_AXIS, false, 0.5f, false);
      }

      #endif
    }
  }

  /**
   * 上下共限位操作，新电路板 Zmin和Zmax共用引脚，原来的Zmax改为Door\n
   * 需注意：\n
   * 1、开启门检测时，最大限位只能插Zmax；未开启门检测时，Zmax、Door可通用\n
   * 2、以前的机器，有的是插Zmax、有的插Door，需要考虑兼容性\n
   * Bug：\n
   * 1、开机，处于最小限位时，归零会撞击喷嘴，问题随机出现\n
   * 2、校正Z高度时，连续撞击Z最小限位，校正完成，问题随机出现
   */
  //static void get_hit_z_endstop_status(void)
  //{
  //  if(motion_3d.enable_poweroff_up_down_min_min //使能了上下共限位
  //      && motion_3d.updown_g28_first_time //上电后第一次归零
  //      && st_get_z_max_endstops_status()) //已经确定上下共限位，则不用判断Z_MAX状态了2017.10.26
  //  {
  //    home_hit_z_endstop = true;
  //  }
  //}

  static void is_at_z_max(void)
  {
    if (!ccm_param::motion_3d.updown_g28_first_time)
    {
      if (home_all_axis || home_code_seen_xyz[Z_AXIS])
      {
        // 修復停止打印時，Z降到限位位置時，打印第二個圖，Z軸判斷當前位置為零點，導致Z無法上升
        // 1、判斷當前是否處於限位狀態
        // 2、如果當前Z位置大於零
        // 以上2點可判斷Z處於下限位，前提：Z已經判斷過零點位置
        if (st_get_z_max_endstops_status() && ccm_param::grbl_current_position[Z_AXIS] > /*motion_3d_model.xyz_max_pos[Z_AXIS] - */ CAL_Z_MAX_POS_OFFSET)
        {
          st_enable_endstops(false);
          float _feedrate = homing_feedrate[Z_AXIS] / 2 ;
          planner_set_position(ccm_param::grbl_current_position);
          st_synchronize();
          ccm_param::grbl_current_position[Z_AXIS] -= 10;
          process_buffer_line_normal(ccm_param::grbl_current_position, _feedrate / 60);
          st_synchronize();
        }
      }
    }
    else
    {
      // 上电第一次归零确认z最大限位状态
      if (st_get_z_max_endstops_status())
      {
        home_hit_z_endstop = true;
      }
    }
  }

  static void home_get_axis_status(void)
  {
    for (int i = 0; i < XYZ_NUM_AXIS; i++)
    {
      home_code_seen_xyz[i] = parseGcodeBufHandle.codeSeen(axis_codes[i]);

      if (home_code_seen_xyz[i])
      {
        if (parseGcodeBufHandle.codeValueLong() != 0)
        {
          home_code_value_xyz[i] = parseGcodeBufHandle.codeValue();
        }
        else
        {
          home_code_value_xyz[i] = (int)ccm_param::motion_3d_model.xyz_home_pos[i];
        }

        if (X_AXIS == i)
        {
          home_code_value_xyz[X2_AXIS] = (int)ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];
        }
      }
    }

    home_all_axis = !(home_code_seen_xyz[X_AXIS] || home_code_seen_xyz[Y_AXIS] || home_code_seen_xyz[Z_AXIS]);

    if (home_all_axis)
    {
      home_code_value_xyz[X2_AXIS] = (int)ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];
    }

    for (int i = 0; i < ccm_param::motion_3d.axis_num; i++)
    {
      ccm_param::grbl_current_position[i] = ccm_param::grbl_destination[i];
    }
  }

  static void home_set_curr_position(void)
  {
    //    // 设置xyz归零坐标
    //    float x, y, z;
    //    g28_get_home_pos_adding(-1, x, y, z);
    if (home_all_axis || home_code_seen_xyz[X_AXIS])
    {
      ccm_param::grbl_current_position[X_AXIS] = home_code_value_xyz[X_AXIS] + add_homeing[X_AXIS];
      ccm_param::grbl_destination[X_AXIS] = home_code_value_xyz[X_AXIS] + add_homeing[X_AXIS];
      ccm_param::grbl_current_position[X2_AXIS] = home_code_value_xyz[X2_AXIS] + add_homeing[X2_AXIS];
      ccm_param::grbl_destination[X2_AXIS] = home_code_value_xyz[X2_AXIS] + add_homeing[X2_AXIS];
    }

    if (home_all_axis || home_code_seen_xyz[Y_AXIS])
    {
      ccm_param::grbl_current_position[Y_AXIS] = home_code_value_xyz[Y_AXIS] + add_homeing[Y_AXIS];
      ccm_param::grbl_destination[Y_AXIS] = home_code_value_xyz[Y_AXIS] + add_homeing[Y_AXIS];
    }

    if (home_all_axis || home_code_seen_xyz[Z_AXIS])
    {
      ccm_param::grbl_current_position[Z_AXIS] = home_code_value_xyz[Z_AXIS] + add_homeing[Z_AXIS];
      ccm_param::grbl_destination[Z_AXIS] = home_code_value_xyz[Z_AXIS] + add_homeing[Z_AXIS];
      //      ccm_param::grbl_current_position[Z_AXIS] -= z;
      //      ccm_param::grbl_destination[Z_AXIS] -= z;
    }

    planner_set_position(ccm_param::grbl_current_position);
    st_synchronize();
  }

  void _level_zero_compensation(void)
  {
    // 设置归零点坐标
    float x, y, z;
    g28_get_home_pos_adding(-1, x, y, z);
    vector_3 vector = vector_3(x, y, 0.0f);
    // 归零点坐标所在矩阵
    int matrix_index = 0;
    matrix_3x3 matrix = plan_bed_level_matrix;
    matrix_3x3 matrix_inverse; // 逆矩阵
    matrix_index = gcode::g29_abl_get_matrix_index(vector);

    if (matrix_index == 0)
    {
      matrix = flash_param_t.matrix_front_left;
    }
    else
    {
      matrix = flash_param_t.matrix_back_right;
    }

    matrix_inverse = matrix_3x3::create_inverse(matrix);
    // 归零点坐标矩阵变换
    vector.apply_rotation(matrix);
    vector.x = 0.0f;
    vector.y = 0.0f;
    vector.z *= -1;
    // 零点坐标逆矩阵运算
    vector.apply_rotation(matrix_inverse);

    // 设置零点坐标当前位置坐标
    if (home_all_axis || home_code_seen_xyz[X_AXIS])
    {
      ccm_param::grbl_current_position[X_AXIS] = vector.x;
      ccm_param::grbl_destination[X_AXIS] = vector.x;
      ccm_param::grbl_current_position[X2_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];
      ccm_param::grbl_destination[X2_AXIS] = ccm_param::motion_3d_model.xyz_home_pos[X2_AXIS];
    }

    if (home_all_axis || home_code_seen_xyz[Y_AXIS])
    {
      ccm_param::grbl_current_position[Y_AXIS] = vector.y;
      ccm_param::grbl_destination[Y_AXIS] = vector.y;
    }

    if (home_all_axis || home_code_seen_xyz[Z_AXIS])
    {
      ccm_param::grbl_current_position[Z_AXIS] = vector.z - z;
      ccm_param::grbl_destination[Z_AXIS] = vector.z - z;
      ccm_param::grbl_current_position[Z2_AXIS] = vector.z - z;
      ccm_param::grbl_destination[Z2_AXIS] = vector.z - z;
    }

    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    {
      plan_set_process_auto_bed_level_status(true);
      planner_set_position(ccm_param::grbl_current_position);
      st_synchronize();
      plan_set_process_auto_bed_level_status(false);
    }
  }

  void g28_process(void)
  {
    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    {
      level_is_rise_z = false; // 喷头抬升
      plan_set_process_auto_bed_level_status(false);
    }

    g28_complete_flag = false;
    previous_xTaskGetTickCount_cmd = xTaskGetTickCount();
    home_get_axis_status();

    if (t_sys_data_current.enable_powerOff_recovery)
    {
      is_at_z_max(); // 判断是否在z最大位置
    }

    st_enable_endstops(true);
    #if (Z_HOME_DIR == 1)                      // If homing away from BED do Z first
    // TODO
    #elif (Z_HOME_DIR == -1)
    // TODO
    #endif
    home_quick();       // 快速归零
    laser_z_rise_first(); // 激光头归零，未抬高Z50mm，先抬高Z，然后激光头X归零
    home_axis();        // 各轴归零
    #ifdef ENDSTOPS_ONLY_FOR_HOMING
    st_enable_endstops(false);
    #endif
    //    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    //    {
    //      _level_zero_compensation();
    //    }
    //    else
    {
      // 设置xyz归零坐标
      home_set_curr_position();
    }
    previous_xTaskGetTickCount_cmd = xTaskGetTickCount();

    if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
    {
      plan_set_process_auto_bed_level_status(true);

      if (home_all_axis || home_code_seen_xyz[Z_AXIS])
      {
        // 设置xyz归零坐标
        float x, y, z;
        g28_get_home_pos_adding(-1, x, y, z);
        ccm_param::grbl_current_position[Z_AXIS] += z;
        ccm_param::grbl_destination[Z_AXIS] += z;
        process_buffer_line_normal(ccm_param::grbl_current_position, homing_feedrate[Z_AXIS] / 60);
        st_synchronize();
        ccm_param::grbl_current_position[Z_AXIS] = home_code_value_xyz[Z_AXIS] + add_homeing[Z_AXIS];
        ccm_param::grbl_destination[Z_AXIS] = home_code_value_xyz[Z_AXIS] + add_homeing[Z_AXIS];
        planner_set_position(ccm_param::grbl_current_position);
        st_synchronize();
      }
    }

    g28_complete_flag = true;
  }


}






