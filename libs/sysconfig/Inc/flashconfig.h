#ifndef FLASHCONFIG_H
#define FLASHCONFIG_H

#include <stdint.h>
#include "vector_3.h"

#ifdef __cplusplus
extern "C" {
#endif
#define FLASH_POWEROFF_DATA_START_ADDR ((uint32_t)0x080C0000)
#define FLASH_POWEROFF_DATA_END_ADDR   ((uint32_t)0x080c0200)
#define FLASH_POWEROFF_DATA_SIZE  ((uint16_t)0x200)
#define FLASH_PARAM_VERSION ((uint32_t)20210318) //版本号

#define IDEX_EXT_BED_MIN_INDEX 0
#define IDEX_EXT_BED_MAX_INDEX 1

#define LEVEL_POS_COUNT 5
#define LEVEL_POS_INDEX_MIDDLE 0
#define LEVEL_POS_INDEX_LEFT_FRONT 1
#define LEVEL_POS_INDEX_LEFT_BACK 2
#define LEVEL_POS_INDEX_RIGHT_BACK 3
#define LEVEL_POS_INDEX_RIGHT_FRONT 4

typedef struct
{
  uint8_t flag;                         // 写flash标准位
  uint8_t is_process_bedlevel;
  uint8_t extruder_type;                // 喷嘴类型
  uint8_t para8[29];
  uint16_t para16[32];
  uint32_t version;
  uint32_t para32[31];
  float axis_steps_per_unit_factor[5];  // 脉冲当量系数
  float dual_extruder_1_offset[3];      // 双头--喷头2与喷头1偏移量
  float dual_extruder_0_offset[3];      //
  float bed_level_matrix_value[9];
  float dual_home_pos_adding[3];        // 归零点偏移，主要用于Z归零
  float paraF[9];
  matrix_3x3 matrix_front_left;
  matrix_3x3 matrix_left_back;
  matrix_3x3 matrix_back_right;
  matrix_3x3 matrix_right_front;
  float idex_ext1_ext0_offset[3];       // Idex喷头2偏移喷头1数值
  float idex_extruder_0_bed_offset[2];  // Idex喷头1热床位置偏移量
  float idex_extruder_1_bed_offset[2];  // Idex喷头2热床位置偏移量
  float mix_extruder_0_bed_offset[2];   // 混色喷头热床位置偏移量
  float laser_extruder_0_bed_offset[2]; // 激光头热床位置偏移量
  float idex_ext0_home_pos_adding[3];   // Idex喷头1归零点偏移，主要用于红外检测Z归零
  float mix_ext0_home_pos_adding[3];    // 混色喷头1归零点偏移，主要用于红外检测Z归零
  float laser_ext0_home_pos_adding[3];  // 激光喷头1归零点偏移，主要用于红外检测Z归零
  float laser_extruder_1_bed_offset[2]; // 激光头热床位置偏移量
  float level_pos_z[LEVEL_POS_COUNT];
  float paraF1[5];
  char pathFileName[100];                            /*!< 文件路径名 */
  char fileName[100];                                /*!< 文件名 */
  unsigned char idex_print_type;                     /*!< 打印类型 */
} FLASH_PARAM_T;

extern FLASH_PARAM_T flash_param_t;

extern void flash_read_data(void);
extern void flash_param_process(void);


typedef struct
{
  int bedTargetTemp;                                 /*!< 热床目标温度 */
  int nozzleTargetTemp;                              /*!< 喷嘴目标温度 */
  int fanSpeed;                                      /*!< 风扇速度 */
  int feedMultiply;                                  /*!< 进料速度百分比 */
  float feedRate;                                    /*!< 进料速度 */
  float xPos;                                        /*!< x位置 */
  float yPos;                                        /*!< y位置 */
  float zPos;                                        /*!< z位置 */
  float ePos;                                        /*!< e位置 */
  float bPos;                                        /*!< b位置 */
  uint32_t sdPos;                                    /*!< 文件位置 */
  uint8_t flag;                                      /*!< 断电标志位 */
  uint8_t serialFlag;                                /*!< 串口标志位 */
  uint32_t blockflag;                                /*!< 堵料标志位 */

} FLASH_POWEROFF_RECOVERY_T;


extern FLASH_POWEROFF_RECOVERY_T flash_poweroff_recovery_t;
extern void flash_save_poweroff_data(void);
extern void flash_erase_poweroff_data(void);

#ifdef __cplusplus
} //extern "C"
#endif


#endif // FLASHCONFIG_H

