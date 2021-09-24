#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <stdint.h>
#include <stdbool.h>
#include "threed_engine.h"
#ifdef __cplusplus
extern "C" {
#endif

#define OnePageNum 4

extern char CurrentPath[100];  //当前目录的路径
extern uint8_t IsRootDir;  //当前目录是否是根目录
extern int CurrentPage;  //当前GUI要显示的页面-当前目录下的文件分成多个GUI页面
extern uint8_t IsHaveNextPage;  //是否有下一页
extern uint8_t IsHaveFile[OnePageNum];  //是否有需要显示的文件名
extern uint8_t IsDir[OnePageNum];  //当前GUI显示的文件是否是目录文件
extern char DisplayFileName[OnePageNum][100];  //当前GUI要显示的文件名

extern uint8_t IsWarning;  //错误警告
extern int WarningInfoSelect; //警告信息选择
//extern int IsGUIReFresh;  //是否GUI刷新
extern uint8_t IsFinishedPowerOffRecoverReady; //是否完成了断电续打的准备
extern uint8_t IsFinishedCalculateZPosLimit;  //是否完成了Z轴的行程测量
extern uint8_t IsComputerControlToPausePrint; //电脑端控制暂停打印
extern uint8_t IsComputerControlToResumePrint; //电脑端控制继续打印
extern uint8_t IsComputerControlToStopPrint; //电脑端控制停止打印
extern uint8_t IsFinishedFilamentHeat;  //是否完成加热-进丝、退丝
extern uint8_t IsSuccessFilament;  //是否完成进丝、退丝
//extern int IsPowerOffRecoverFileInSD; //断电续打文件是否在SD卡中

extern uint8_t IsTransFile;  //是否在上传文件
extern uint8_t ChangeFilamentHeatStatus; //是否加热完成
extern uint8_t M600FilamentChangeStatus; //打印中途换料状态
extern uint8_t IsDoorOpen; //M14R03,M14S检测到门是否打开
extern uint8_t IsDisplayDoorOpenInfo; //是否显示门打开的提示信息
extern uint8_t IsFinishMatCheckCalibrate; //是否完成了断料校准
extern uint8_t IsNotHaveMatInPrint; //在打印的时候是否没料了
extern char SDFileName[50];  //电脑上传的文件名

#ifdef CAL_Z_ZERO_OFFSET
extern bool isCalZero;
//extern float z_offset_value;
#endif

extern unsigned int isOpenBeep;
extern unsigned int doorStatus;
extern unsigned int task_schedule_delay_time;

//#define LCDWidth  480             //lcd像素宽度（480*320）
//#define LCDHeight 320             //lcd像素高度（480*320）

typedef struct
{
  volatile uint8_t active_extruder;
  volatile int x_move_value;
} t_gui_respond_data;

extern t_gui_respond_data gui_respond_data;

///////////////////////////////////// PowerOffRecovery Start///////////////////////////////////////////

extern uint32_t poweroff_data_size;             // 斷電續打數據緩存大小

typedef struct
{
  char path_file_name[100];                     // 断电续打文件路徑全名
  char file_name[100];                          // 断电续打文件名
  float feed_rate;                              // 出料速度
  float x_pos;                                  // X轴位置
  float y_pos;                                  // Y轴位置
  float z_pos;                                  // Z轴位置
  float e_pos;                                  // E轴位置
  float b_pos;                                  // B轴位置
  uint32_t sd_pos;                              // 文件指针位置
  uint16_t bed_target_temp;                     // 热床目标温度
  uint16_t nozzle_target_temp;                  // 喷嘴目标温度
  uint16_t fan_speed;                           // 风扇速度
  uint16_t feed_multiply;                       // 打印速度
  uint8_t flag;                                 // 是否需要断电续打
  uint8_t is_file_from_sd;                      // 断电续打文件是否在SD卡中
  uint8_t is_power_off;                         // 是否已经断电
  uint8_t blockdetectflag;                      // 标志堵料
  uint8_t serial_flag;
} T_POWER_OFF;
///////////////////////////////////// PowerOffRecovery End///////////////////////////////////////////

///////////////////////////////////// CustomServices Start///////////////////////////////////////////
#define CAL_Z_MAX_POS_OFFSET 20                 // 校準Z最大位置時，校準值與原來MAX Z的偏移量
typedef struct
{
  uint8_t disable_abs;                          // 是否能打印ABS
  uint8_t disable_hot_bed;                      // 是否开启热床
  uint8_t enable_warning_light;                 // 是否有警示灯
  uint8_t enable_led_light;                     // 是否有LED照明
} T_CUSTOM_SERVICES;

///////////////////////////////////// CustomServices End///////////////////////////////////////////

///////////////////////////////////// GUI Start///////////////////////////////////////////
typedef struct
{
  int printed_time_sec;                         // 已打印的时间
  int used_total_material;                      // 耗材总长度
  int machine_run_time;                         // 机器运行时间
  uint16_t nozzle_temp[2];                         // 喷嘴温度
  uint16_t target_nozzle_temp[2];                  // 喷嘴目标温度
  uint16_t hot_bed_temp;                        // 热床温度
  uint16_t target_hot_bed_temp;                 // 热床目标温度
  uint16_t print_speed_value;                   // 打印速度
  uint16_t fan_speed_value;                     // 风扇速度
  uint16_t cavity_temp;
  uint16_t target_cavity_temp;
  uint16_t target_cavity_temp_on;
  int move_xyz_pos[XYZ_NUM_AXIS];                     // 移动XYZ轴
  uint8_t print_percent;                        // 打印进度-百分数
  uint8_t is_refresh_rtc;                           // 是否GUI刷新
  uint32_t printfile_size;                      // 打印文件总大小（字节）
  uint32_t file_size;                           // 打印文件剩余大小（字节）
  uint16_t cura_speed;                          // 获取到的cura软件上的速度，M117命令传送
} T_GUI;
///////////////////////////////////// GUI End///////////////////////////////////////////

extern T_CUSTOM_SERVICES t_custom_services;
//  extern T_CUSTOM_MODEL t_custom_model;
extern T_GUI t_gui;
extern T_POWER_OFF t_power_off;

extern unsigned char is_serial_full;

#ifdef __cplusplus
} //extern "C"
#endif

#endif // GLOBALVARIABLES_H









