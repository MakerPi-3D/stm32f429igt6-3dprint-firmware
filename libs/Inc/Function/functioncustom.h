#ifndef FUNCTIONCUSTOM_H
#define FUNCTIONCUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "stm32f4xx_hal.h"
//#include "stdbool.h"

#define DEBUG_FUNCTION_CUSTOM 1
#if (1 == DEBUG_FUNCTION_CUSTOM)
#define DEBUG_POWEROFF_CUSTOM 0
#endif

// 断电恢复接口
extern void poweroff_start_cal_z_max_pos(void);
extern void poweroff_stop_cal_z_max_pos(void);
extern void poweroff_reset_flag(void);
extern void poweroff_set_file_path_name(const char *filePathName);
extern void poweroff_set_data(void);
extern void poweroff_ready_to_recover_print(void);
extern void poweroff_delete_file_from_sd(void);

// 测试固件接口
extern void board_test_display_function(void);
extern void board_test_model_select(void);
extern void board_test_cal_heat_time_gui(void);
extern void board_test_cal_touch_count(void);
extern void board_test_pressure(void);

#ifdef __cplusplus
} //extern "C" {
#endif

#endif // FUNCTIONCUSTOM_H

