#ifndef USER_CCM_H
#define USER_CCM_H

#include <stdbool.h>

#include "threed_engine.h"
#include "planner_block_buffer.h"
#include "config_motion_3d.h"

#define BUFSIZE               1   /*!< 定义环形数组大小为 256 */
#define MAX_CMD_SIZE          96    /*!< 定义最大接收字节数 96 */
#define POWER_OFF_BUF_SIZE    512                  // 数据长度不能超过512，否则溢出,以字符串形式存储，一次性写入到文件11
#define PIC_BUF_SIZE          (1024*16) //(48*32*4) // 6k大小
#define TEXT_BUF_24_12_9_SIZE (24*12*9)
#define TEXT_BUF_24_12_3_SIZE (24*12*3)

#ifdef __cplusplus
extern "C" {
#endif

// 运行状态
typedef struct
{
  volatile float axis_position[MAX_NUM_AXIS];  // 坐标位置
  volatile float bed_temp;                     // 热床温度
  volatile float extruder0_temp;               // 喷嘴温度
  volatile float extruder1_temp;               // 喷嘴温度
  volatile float feed_rate;                    // 进料速度
  volatile unsigned long sd_position;          // 文件位置
  volatile int extruder_multiply;              // 移动速度百分比
  volatile int feed_multiply;                  // 进料百分比
  volatile int fan_speed;                      // 风扇速度
  volatile unsigned char extruder;             // 喷头ID
  volatile unsigned char is_serial;            // 是否串口指令
} planner_running_status_t;




namespace ccm_param
{
  extern char PictureFileBuf[PIC_BUF_SIZE];  //处于IRAM2区的起始地址 40k 大小12k
  extern char cmdbuffer[BUFSIZE][MAX_CMD_SIZE]; /*!< 环形指令队列 */
  extern char command_buffer[MAX_CMD_SIZE];                /*!< 指令数组 */
  extern char poweroff_data[POWER_OFF_BUF_SIZE]; // 斷電續打數據緩存  处于IRAM2区的起始地址7k
  extern unsigned short int TextRangeBuf_24_12_9_0[TEXT_BUF_24_12_9_SIZE];
  extern unsigned short int TextRangeBuf_24_12_9_1[TEXT_BUF_24_12_9_SIZE];
  extern unsigned short int TextRangeBuf_24_12_9_2[TEXT_BUF_24_12_9_SIZE];
  extern unsigned short int TextRangeBuf_24_12_9_3[TEXT_BUF_24_12_9_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_0[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_1[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_2[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_3[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_4[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_5[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_6[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_7[TEXT_BUF_24_12_3_SIZE];
  extern unsigned short int TextRangeBuf_24_12_3_8[TEXT_BUF_24_12_3_SIZE];
  extern volatile float grbl_current_position[MAX_NUM_AXIS];
  extern volatile float grbl_destination[MAX_NUM_AXIS];
  extern planner_running_status_t runningStatus[BLOCK_BUFFER_SIZE];//断电续打保存参数
  extern planner_running_status_t running_status;
  extern block_t block_buffer[BLOCK_BUFFER_SIZE];            // A ring buffer for motion instfructions
  extern volatile unsigned char block_buffer_head;           // Index of the next block to be pushed
  extern volatile unsigned char block_buffer_tail;
  extern volatile motion_3d_t motion_3d;
  extern volatile motion_3d_model_t motion_3d_model;
  extern volatile long layer_count;
  extern volatile long current_layer;
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif //USER_CCM_H




