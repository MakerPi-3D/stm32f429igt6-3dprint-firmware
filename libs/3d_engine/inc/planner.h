
#ifndef planner_h
#define planner_h

#include <stdbool.h>
#include "planner_running_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint32_t max_acceleration_mm_per_s2[MAX_NUM_AXIS], // (mm/s^2) M201 XYZE
           min_segment_time_us;                // (µs) M205 B
  float axis_steps_per_mm[MAX_NUM_AXIS];    // (steps) M92 XYZE - Steps per millimeter
  float max_feedrate_mm_s[MAX_NUM_AXIS];    // (mm/s) M203 XYZE - Max speeds
  float acceleration,                       // (mm/s^2) M204 S - Normal acceleration. DEFAULT ACCELERATION for all printing moves.
        retract_acceleration,               // (mm/s^2) M204 R - Retract acceleration. Filament pull-back and push-forward while standing still in the other axes
        travel_acceleration;                // (mm/s^2) M204 T - Travel acceleration. DEFAULT ACCELERATION for all NON printing moves.
  float min_feedrate_mm_s,                  // (mm/s) M205 S - Minimum linear feedrate
        min_travel_feedrate_mm_s;           // (mm/s) M205 T - Minimum travel feedrate
  float max_xy_jerk,                        // (mm/s) M205 X - speed than can be stopped at once, if i understand correctly.
        max_z_jerk,                         // (mm/s) M205 Z - speed than can be stopped at once, if i understand correctly.
        max_e_jerk,                         // (mm/s) M205 E - speed than can be stopped at once, if i understand correctly.
        max_b_jerk;                         // (mm/s) M205 E - speed than can be stopped at once, if i understand correctly.
  bool axis_relative_modes[MAX_NUM_AXIS];   // (false/true) M82/M83
  bool relative_mode;                       // (false/true) G90/G91 - Determines Absolute or Relative Coordinates
} planner_settings_t;


extern volatile planner_settings_t planner_settings;

void planner_init(void);                                                ///< 初始化电机规划系统
void planner_set_position(const volatile float *_current_position);              ///< G92指令，设置xyzeb坐标
void planner_set_axis_position(const volatile float &value, const int axis);
void planner_set_position_basic(const volatile float *_current_position, bool is_sync_z);
void planner_set_axis_position_basic(const volatile float &value, const int axis, bool is_sync_z);
void planner_buffer_line(planner_running_status_t *running_status, bool is_sync_z);     ///< 移动指令坐标数据
bool planner_blocks_queued(void);                                       ///< 返回队列是否有数据
int planner_moves_planned(void);                                        ///< 返回当前运动队列有多少条数据
bool is_planner_moves_planned_full(void);

// 自动调平接口
void plan_buffer_line_get_xyz(float (&plan_buffer_position)[MAX_NUM_AXIS]);   ///< 获取自动调平变换坐标
void plan_set_position_get_xyz(const volatile float *_current_position, long (&position_xyz)[MAX_NUM_AXIS], bool is_sync_z);
void plan_set_process_auto_bed_level_status(bool status);
float plan_get_current_save_xyz(int axis);
float plan_get_xyz_real(int axis);
void plan_init_offset1(void);

#ifdef __cplusplus
} //extern "C" {
#endif

#endif





