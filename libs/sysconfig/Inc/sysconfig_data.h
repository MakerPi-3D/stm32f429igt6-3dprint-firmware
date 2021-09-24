#ifndef SYSCONFIG_DATA_H
#define SYSCONFIG_DATA_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LCD_TYPE_35_480320_SIZE 0 // 3.5寸 480*320
#define LCD_TYPE_43_480272_SIZE 1 // 4.3寸 480*272
#define LCD_TYPE_7_1024600_SIZE 2 // 7寸 1024*600

#define BED_LEVEL_PRESSURE_SENSOR 3 //红外激光头

#define EXTRUDER_TYPE_NONE 0
#define EXTRUDER_TYPE_SINGLE 1
#define EXTRUDER_TYPE_MIX 2
#define EXTRUDER_TYPE_DUAL 3
#define EXTRUDER_TYPE_LASER 4


#define IDEX_PRINT_TYPE_NORMAL 0
#define IDEX_PRINT_TYPE_COPY 1
#define IDEX_PRINT_TYPE_MIRROR 2

#define MIX_PRINT_TYPE_GRADIENT_COLOR 0
#define MIX_PRINT_TYPE_FIX_PROPORTION 1
#define MIX_PRINT_TYPE_RANDOM 2
///////////////////////////////////// SysConfig Start///////////////////////////////////////////

typedef struct
{
  char boot_ver_str[20];                   // boot版本
  char app2_ver_str[20];                   // app2版本
  char build_date_str[20];                 // 制作时间
  unsigned char have_set_machine;          // 是否已經設置機器
  unsigned char model_id;                  // 要改变为哪种机型
  unsigned char pic_id;                    // 要改变为哪种图片 中文图片或日文图片
  unsigned char enable_powerOff_recovery;  // 是否有斷電功能
  float poweroff_rec_z_max_value;          // 斷電z最大高度
  unsigned char enable_color_mixing;       // 是否有混色功能
  unsigned char enable_material_check;     // 是否有断料检测功能
  float material_chk_vol_value;            //
  unsigned char enable_block_detect;       // 是否開啓堵料檢測
  float bed_level_z_at_left_front;         //
  float bed_level_z_at_right_front;        //
  float bed_level_z_at_left_back;          //
  float bed_level_z_at_right_back;         //
  float bed_level_z_at_middle;             //
  float pid_output_factor;                 // 加热系数
  unsigned char enable_bed_level;          // 是否开启自动调平
  unsigned char enable_soft_filament;      // 是否使用软料
  unsigned char enable_LOGO_interface;     // 是否开启开机LOGO界面
  unsigned char logo_id;                   // logo图片的id号
  unsigned char custom_model_id;           // 定制机型id
  unsigned char buzzer_value;              // 按键声、报警声开关
  float z_offset_value;                    // Z零点偏移
  unsigned char enable_v5_extruder;
  unsigned char enable_cavity_temp;
  unsigned char enable_type_of_thermistor;
  unsigned char enable_high_temp;
  unsigned char ui_number;
  unsigned char is_2GT;
  unsigned char IsMechanismLevel;
  unsigned char IsLaser;
  unsigned char IsLaserMode;
  unsigned char cf34;
  unsigned char cf35;
  unsigned char cf36;
  unsigned char cf37;
  unsigned char cf38;
  unsigned char cf39;
  unsigned char cf40;
} T_SYS_DATA;

// SysConfig
typedef struct
{
  char model_str[30];                           // 机型字符串
  char function_str[35];                        // 功能字符串
  char version_str[30];                         // 版本字符串
  int key_sound;
  int alarm_sound;
  unsigned char enable_cavity_temp;            // 是否开启腔体温度开关
  unsigned char is_bed_level_down_to_zero;     // 平台是否下归零
  unsigned char is_detect_extruder_thermistor; // 是否检测热敏电阻脱离
  unsigned char is_planner_slow_down;          // 是否planner减速
  unsigned short serial_moves_queued;          // 串口移动指令数
  unsigned char lcd_ssd1963_43_480_272;
  unsigned int print_time_save;
  unsigned char enable_color_buf;
  unsigned char lcd_type;
  unsigned char is_idex_extruder;  //是否为idex结构
  unsigned char idex_print_type;  //是否为idex结构
  unsigned char idex_print_type_bak;  //是否为idex结构
  unsigned char mix_print_type;  //是否为idex结构
} T_SYS;

extern char sys_data[512];                      // sysconfig數據緩存
extern uint32_t sys_data_size;                  // sysconfig數據緩存大小

extern T_SYS t_sys;
extern T_SYS_DATA t_sys_data_current;

///////////////////////////////////// SysConfig End///////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 默认配置
#define DEFAULT_MODEL               0 // 默认配置//0:M14 1:M2030 2:M2041 3:M2048 4:M3145 5:M4141 6:M4040 7:M4141S 8:AMP410W 9:M14R03 10:M2030HY 11:M14S 12:M3145S    //4141S机型的X轴方向不一样
#define DEFAULT_COLORMIXING         0 // 1:打开混色功能 0:关闭混色功能
#define DEFAULT_POWEROFFRECOVERY    0 // 1:打开断电续打功能 0:关闭断电续打功能
#define DEFAULT_MATCHECK            0 // 1:打开断料检测功能 0：关闭断料检测功能
#define DEFAULT_STEP                1 // 1: 电机16细分 2： 电机32细分
#define DEFAULT_NUMAXIS             4 // 4: 基础版4个电机 5：混色版5个电机

// GUI圖片ID
#define PICTURE_IS_CHINESE  1
#define PICTURE_IS_JAPANESE 2
#define PICTURE_IS_ENGLISH  3
#define PICTURE_IS_KOREA    4
#define PICTURE_IS_RUSSIA   5
#define PICTURE_IS_CHINESE_TRADITIONAL   6

extern void sys_save_data(const char *filePath);
extern void sys_write_info_to_sd(const char *filePath);
extern void sys_read_info_from_sd(const char *filePath);
#ifdef __cplusplus
} //extern "C" {
#endif
#endif


