#ifndef PLANNER_RUNNING_STATUS
#define PLANNER_RUNNING_STATUS

#include <stdio.h>
#include <string.h>
#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

inline uint32_t plan_get_sd_position()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  uint32_t sd_position = rs->sd_position;
  return (sd_position > 0) ? sd_position : 0;
}

inline uint8_t plan_get_serial_flag()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->is_serial;
}

inline float plan_get_axis_position(const int axis)
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->axis_position[axis];
}

inline int plan_get_fmultiply()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->feed_multiply;
}

inline int plan_get_fans()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->fan_speed;
}

inline float plan_get_bedtemp()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->bed_temp;
}

inline float plan_get_ext0temp()
{
  planner_running_status_t *rs = &ccm_param::runningStatus[ccm_param::block_buffer_tail];
  return rs->extruder0_temp ;
}

#ifdef __cplusplus
} // extern "C" {
#endif


#endif // PLANNER_RUNNING_STATUS

