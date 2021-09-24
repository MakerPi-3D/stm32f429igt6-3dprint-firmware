#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "threed_engine.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long axis_steps_per_sqr_second[MAX_NUM_AXIS];

extern const long dropsegments;

extern int filament_load_unload_temp;
//  extern float extrude_min_temp;
//  extern int heater_0_maxtemp;
extern int pla_preheat_hotend_temp;//170
extern int pla_preheat_hpb_temp;


extern int abs_preheat_hotend_temp;//170
extern int abs_preheat_hpb_temp;//170

extern float z_home_retract_mm;

void reset_acceleration_rates(void);
void Config_ResetDefault(void);





#ifdef __cplusplus
} // extern "C" {
#endif

#endif//CONFIG_STORE_H
