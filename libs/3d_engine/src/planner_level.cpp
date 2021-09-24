#include "planner.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "vector_3.h"
#include "flashconfig.h"
#include "gcode.h"
#include "user_ccm.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define lround(x) ((x)>=0.0f?(long)((x)+0.5f):(long)((x)-0.5f))


//////////////////////////////�Զ���ƽ����//////////////////////////////////

// ƽ̨��������
matrix_3x3 plan_bed_level_matrix ;                                     ///< �ȴ�ƫ�ƾ���
// XYZ����洢����
static volatile float current_save_xyz[XYZ_NUM_AXIS] = {0.0f};         ///< ���浱ǰXYZλ�ã�У׼ǰ
static volatile float current_xyz_compensate[XYZ_NUM_AXIS] = {0.0f};   ///< ���浱ǰXYZλ�ò���
static volatile float current_xyz_real[XYZ_NUM_AXIS] = {0.0f};         ///< ���浱ǰXYZ��ʵλ�ã�У׼��
static volatile bool is_process_auto_bed_leveling = false;             ///< ִ��ƽ̨У׼����

static matrix_3x3 matrix;           ///< ��ǰ����
static int matrix_index;            ///< ��ǰ����id
static int pre_matrix_index = 0;    ///< ǰ����id
static int zero_matrix_index = 0;   ///< ������id
static vector_3 offset;
static vector_3 offset1;

extern vector_3 level_points[LEVEL_POS_COUNT];

static void _get_matrix(vector_3 vector)
{
  // ��ȡ����
  matrix.set_to_identity();
  matrix_index = gcode::g29_abl_get_matrix_index(vector);

  if (matrix_index == 0)
  {
    matrix = flash_param_t.matrix_front_left;
  }
  else if (matrix_index == 1)
  {
    matrix = flash_param_t.matrix_back_right;
  }
}

void plan_set_process_auto_bed_level_status(bool status)
{
  if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
  {
    is_process_auto_bed_leveling = status;

    if (!is_process_auto_bed_leveling)
    {
      _get_matrix(vector_3(0.0f, 0.0f, 0.0f));
      zero_matrix_index = matrix_index;
      pre_matrix_index = matrix_index;
      offset = vector_3(0.0f, 0.0f, 0.0f);
      gcode::g28_complete_flag = false;
    }
  }
}

void plan_init_offset1(void)
{
  vector_3 vector0 = level_points[LEVEL_POS_INDEX_RIGHT_FRONT];
  vector_3 vector1 = level_points[LEVEL_POS_INDEX_RIGHT_FRONT];
  vector0.apply_rotation(matrix_3x3::create_inverse(flash_param_t.matrix_front_left));
  vector1.apply_rotation(matrix_3x3::create_inverse(flash_param_t.matrix_back_right));
  vector_3 vector2 = level_points[LEVEL_POS_INDEX_LEFT_BACK];
  vector_3 vector3 = level_points[LEVEL_POS_INDEX_LEFT_BACK];
  vector2.apply_rotation(matrix_3x3::create_inverse(flash_param_t.matrix_front_left));
  vector3.apply_rotation(matrix_3x3::create_inverse(flash_param_t.matrix_back_right));
  offset1 = (vector1 - vector0) + (vector3 - vector2);
  offset1.x = offset1.x / 2.0f;
  offset1.y = offset1.y / 2.0f;
  offset1.z = offset1.z / 2.0f;
}

static void plan_apply_rotation_xyz(float &x, float &y, float &z)
{
  if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
  {
    vector_3 vector = vector_3(x, y, z);
    _get_matrix(vector);

    if (matrix_index != pre_matrix_index)
    {
      if (zero_matrix_index == matrix_index)
      {
        offset = vector_3(0.0f, 0.0f, 0.0f);
      }
      else
      {
        if (matrix_index == 1)
        {
          offset = offset1;
        }
        else
        {
          offset = vector_3(0.0f, 0.0f, 0.0f);
        }
      }
    }

    vector = vector + offset;
    vector.apply_rotation(matrix);
    x = vector.x;
    y = vector.y;
    z = vector.z;
    pre_matrix_index = matrix_index;
  }
}

// ���浱ǰλ��
static void _get_xyz_save_current(float (&plan_buffer_position)[MAX_NUM_AXIS])
{
  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    current_save_xyz[i] = plan_buffer_position[i];
  }
}

// ������ʵ�ƶ�����
static void _get_xyz_save_real(float (&plan_buffer_position)[MAX_NUM_AXIS])
{
  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    current_xyz_real[i] = plan_buffer_position[i];
  }
}

static void _get_xy_compensate_max(float (&plan_buffer_position)[MAX_NUM_AXIS], int axis)
{
  // ת����������������ֵ�����ƶ������浱ǰ��ֵ���Ա���һ�β���
  float max_value = ccm_param::motion_3d_model.xyz_max_pos[axis];

  if (axis == X2_AXIS)
  {
    max_value = ccm_param::motion_3d_model.xyz_home_pos[axis];
  }

  if (plan_buffer_position[axis] > max_value)
  {
    current_xyz_compensate[axis] = plan_buffer_position[axis] - max_value;
    plan_buffer_position[axis] = max_value;
  }
  else // ȷ���Ƿ���Ҫ�������������ֵ
  {
    plan_buffer_position[axis] += current_xyz_compensate[axis];
    current_xyz_compensate[axis] = 0.0f;
  }
}

static void _get_xy_compensate_min(float (&plan_buffer_position)[MAX_NUM_AXIS], int axis)
{
  // ת���������С����Сֵ�����ƶ������浱ǰ��ֵ���Ա���һ�β���
  if (plan_buffer_position[axis] < ccm_param::motion_3d_model.xyz_min_pos[axis]) // ת������X���Ϊ���������ƶ������浱ǰ��ֵ���Ա���һ�β���
  {
    current_xyz_compensate[axis] = ccm_param::motion_3d_model.xyz_min_pos[axis] - plan_buffer_position[axis];
    plan_buffer_position[axis] = ccm_param::motion_3d_model.xyz_min_pos[axis];
  }
  else // ȷ���Ƿ���Ҫ�������������ֵ
  {
    plan_buffer_position[axis] += current_xyz_compensate[axis];
    current_xyz_compensate[axis] = 0.0f;
  }
}

// ��ȡxy����ֵ
static void _get_xy_compensate(float (&plan_buffer_position)[MAX_NUM_AXIS])
{
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1 && 1 == gcode::active_extruder)
  {
    plan_apply_rotation_xyz(plan_buffer_position[X2_AXIS], plan_buffer_position[Y_AXIS], plan_buffer_position[Z_AXIS]);
    _get_xy_compensate_max(plan_buffer_position, X2_AXIS);
  }
  else
  {
    plan_apply_rotation_xyz(plan_buffer_position[X_AXIS], plan_buffer_position[Y_AXIS], plan_buffer_position[Z_AXIS]);
    _get_xy_compensate_min(plan_buffer_position, X_AXIS);
  }

  _get_xy_compensate_min(plan_buffer_position, Y_AXIS);
}

void plan_buffer_line_get_xyz(float (&plan_buffer_position)[MAX_NUM_AXIS])
{
  // ���浱ǰλ��
  _get_xyz_save_current(plan_buffer_position);

  if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
  {
    if (is_process_auto_bed_leveling)
    {
      _get_xy_compensate(plan_buffer_position);
    }
    else
    {
      // ��ƽģʽ�£���ת�����꣬����xyz����ֵ
      for (int i = 0; i < XYZ_NUM_AXIS; i++)
      {
        current_xyz_compensate[i] = 0.0f;
      }
    }
  }

  // ������ʵ�ƶ�����
  _get_xyz_save_real(plan_buffer_position);
}

void plan_set_position_get_xyz(const volatile float *_current_position, long (&position_xyz)[MAX_NUM_AXIS], bool is_sync_z)
{
  float auto_bed_level_position[MAX_NUM_AXIS] = {0.0f};

  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    auto_bed_level_position[i] = _current_position[i];
  }

  plan_buffer_line_get_xyz(auto_bed_level_position);

  for (int i = 0; i < XYZ_NUM_AXIS; i++)
  {
    if (X2_AXIS == i)
    {
      if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1 && 1 == gcode::active_extruder)
      {
        position_xyz[i] = (long)(auto_bed_level_position[i] * planner_settings.axis_steps_per_mm[i]);
        position_xyz[i] = (long)lround(position_xyz[i]);
      }
    }
    else
    {
      position_xyz[i] = (long)(auto_bed_level_position[i] * planner_settings.axis_steps_per_mm[i]);
      position_xyz[i] = (long)lround(position_xyz[i]);
    }
  }

  if (is_sync_z)
  {
    position_xyz[Z2_AXIS] = position_xyz[Z_AXIS];
  }
}

float plan_get_current_save_xyz(int axis)
{
  return current_save_xyz[axis];
}

float plan_get_xyz_real(int axis)
{
  return current_xyz_real[axis];
}

#ifdef __cplusplus
} //extern "C" {
#endif


