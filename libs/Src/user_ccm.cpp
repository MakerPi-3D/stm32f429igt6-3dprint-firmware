#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

//IRAM2共64K，起始地址0x10000000  IRAM1共128K，起始地址0x20000000
#define CCM_PIC_BUF_SIZE_ADD           (0X10010000 - (PIC_BUF_SIZE*sizeof(char)))
#define CCM_CMD_QUEUE_BUF_ADD          (CCM_PIC_BUF_SIZE_ADD - (BUFSIZE*MAX_CMD_SIZE*sizeof(char))) // 0X10003000
#define CCM_CMD_BUF_ADD                (CCM_CMD_QUEUE_BUF_ADD - (MAX_CMD_SIZE*sizeof(char))) // 0X10006000
#define CCM_POWEROFF_DATA_ADD          (CCM_CMD_BUF_ADD - (POWER_OFF_BUF_SIZE*sizeof(char))) // 0X10006060
#define CCM_TEXT_BUF_24_12_9_0_ADD     (CCM_POWEROFF_DATA_ADD - (TEXT_BUF_24_12_9_SIZE*sizeof(unsigned short int))) // 0X100076a0
#define CCM_TEXT_BUF_24_12_9_1_ADD     (CCM_TEXT_BUF_24_12_9_0_ADD - (TEXT_BUF_24_12_9_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_9_2_ADD     (CCM_TEXT_BUF_24_12_9_1_ADD - (TEXT_BUF_24_12_9_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_9_3_ADD     (CCM_TEXT_BUF_24_12_9_2_ADD - (TEXT_BUF_24_12_9_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_0_ADD     (CCM_TEXT_BUF_24_12_9_3_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_1_ADD     (CCM_TEXT_BUF_24_12_3_0_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_2_ADD     (CCM_TEXT_BUF_24_12_3_1_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_3_ADD     (CCM_TEXT_BUF_24_12_3_2_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_4_ADD     (CCM_TEXT_BUF_24_12_3_3_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_5_ADD     (CCM_TEXT_BUF_24_12_3_4_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_6_ADD     (CCM_TEXT_BUF_24_12_3_5_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_7_ADD     (CCM_TEXT_BUF_24_12_3_6_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_TEXT_BUF_24_12_3_8_ADD     (CCM_TEXT_BUF_24_12_3_7_ADD - (TEXT_BUF_24_12_3_SIZE*sizeof(unsigned short int)))
#define CCM_GRBL_CURRENT_POSITION_ADD  (CCM_TEXT_BUF_24_12_3_8_ADD - ((MAX_NUM_AXIS)*sizeof(volatile float)))
#define CCM_GRBL_DESTINATION_ADD       (CCM_GRBL_CURRENT_POSITION_ADD - ((MAX_NUM_AXIS)*sizeof(volatile float)))
#define CCM_RUNNING_STATUS_QUEUE_ADD   (CCM_GRBL_DESTINATION_ADD - (BLOCK_BUFFER_SIZE*sizeof(planner_running_status_t)))
#define CCM_RUNNING_STATUS_ADD         (CCM_RUNNING_STATUS_QUEUE_ADD - sizeof(planner_running_status_t))
#define CCM_BLOCK_BUFFER_ADD           (CCM_RUNNING_STATUS_ADD - (BLOCK_BUFFER_SIZE*sizeof(block_t)))
#define CCM_BLOCK_BUFFER_HEAD_ADD      (CCM_BLOCK_BUFFER_ADD - 3 - sizeof(volatile unsigned char))
#define CCM_BLOCK_BUFFER_TAIL_ADD      (CCM_BLOCK_BUFFER_HEAD_ADD - 3 - sizeof(volatile unsigned char))
#define CCM_MOTION_3D_ADD              (CCM_BLOCK_BUFFER_TAIL_ADD - sizeof(volatile motion_3d_t))
#define CCM_MOTION_3D_MODEL_ADD        (CCM_MOTION_3D_ADD - sizeof(volatile motion_3d_model_t))
#define CCM_LAYER_COUNT_ADD            (CCM_MOTION_3D_MODEL_ADD - sizeof(volatile long))
#define CCM_CURRENT_LAYER_ADD          (CCM_LAYER_COUNT_ADD - sizeof(volatile long))

namespace ccm_param
{
  char PictureFileBuf[PIC_BUF_SIZE]                                     __attribute__((at(CCM_PIC_BUF_SIZE_ADD)));          /*!< 0x1000D000 12288byte 处于IRAM2区的起始地址 40k 大小12k */
  char cmdbuffer[BUFSIZE][MAX_CMD_SIZE]                                 __attribute__((at(CCM_CMD_QUEUE_BUF_ADD)));         /*!< 0x1000A000 12288byte 环形指令队列 */
  char command_buffer[MAX_CMD_SIZE]                                     __attribute__((at(CCM_CMD_BUF_ADD)));               /*!< 0x10009FA0 96byte 指令数组 */
  char poweroff_data[POWER_OFF_BUF_SIZE]                                __attribute__((at(CCM_POWEROFF_DATA_ADD)));         /*!< 0x10009DA0 512byte 斷電續打數據緩存  处于IRAM2区的起始地址7k */
  unsigned short int TextRangeBuf_24_12_9_0[TEXT_BUF_24_12_9_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_9_0_ADD)));    /*!< 0x10008960 5184byte */
  unsigned short int TextRangeBuf_24_12_9_1[TEXT_BUF_24_12_9_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_9_1_ADD)));    /*!< 0x10007520 5184byte */
  unsigned short int TextRangeBuf_24_12_9_2[TEXT_BUF_24_12_9_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_9_2_ADD)));    /*!< 0x100060E0 5184byte */
  unsigned short int TextRangeBuf_24_12_9_3[TEXT_BUF_24_12_9_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_9_3_ADD)));    /*!< 0x10004CA0 5184byte */
  unsigned short int TextRangeBuf_24_12_3_0[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_0_ADD)));    /*!< 0x100045E0 1728byte */
  unsigned short int TextRangeBuf_24_12_3_1[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_1_ADD)));    /*!< 0x10003F20 1728byte */
  unsigned short int TextRangeBuf_24_12_3_2[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_2_ADD)));    /*!< 0x10003860 1728byte */
  unsigned short int TextRangeBuf_24_12_3_3[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_3_ADD)));    /*!< 0x100031A0 1728byte */
  unsigned short int TextRangeBuf_24_12_3_4[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_4_ADD)));    /*!< 0x10002AE0 1728byte */
  unsigned short int TextRangeBuf_24_12_3_5[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_5_ADD)));    /*!< 0x10002420 1728byte */
  unsigned short int TextRangeBuf_24_12_3_6[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_6_ADD)));    /*!< 0x10001D60 1728byte */
  unsigned short int TextRangeBuf_24_12_3_7[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_7_ADD)));    /*!< 0x100016A0 1728byte */
  unsigned short int TextRangeBuf_24_12_3_8[TEXT_BUF_24_12_3_SIZE]      __attribute__((at(CCM_TEXT_BUF_24_12_3_8_ADD)));    /*!< 0x10000FE0 1728byte */
  volatile float grbl_current_position[MAX_NUM_AXIS]                    __attribute__((at(CCM_GRBL_CURRENT_POSITION_ADD))); /*!< 0x10000FCC 20byte */
  volatile float grbl_destination[MAX_NUM_AXIS]                         __attribute__((at(CCM_GRBL_DESTINATION_ADD)));      /*!< 0x10000FB8 20byte */
  planner_running_status_t runningStatus[BLOCK_BUFFER_SIZE]             __attribute__((at(CCM_RUNNING_STATUS_QUEUE_ADD)));  /*!< 0x10000C78 832byte 断电续打保存参数 */
  planner_running_status_t running_status                               __attribute__((at(CCM_RUNNING_STATUS_ADD)));        /*!< 0x10000C44 52byte */
  block_t block_buffer[BLOCK_BUFFER_SIZE]                               __attribute__((at(CCM_BLOCK_BUFFER_ADD)));          /*!< 0x10000744 1280byte A ring buffer for motion instfructions */
  volatile unsigned char block_buffer_head                              __attribute__((at(CCM_BLOCK_BUFFER_HEAD_ADD)));     /*!< 0x10000740 1byte Index of the next block to be pushed */
  volatile unsigned char block_buffer_tail                              __attribute__((at(CCM_BLOCK_BUFFER_TAIL_ADD)));     /*!< 0x1000073C 1byte Index of the block to process now */
  volatile motion_3d_t motion_3d                                        __attribute__((at(CCM_MOTION_3D_ADD)));             /*!< */
  volatile motion_3d_model_t motion_3d_model                            __attribute__((at(CCM_MOTION_3D_MODEL_ADD)));       /*!< */
  volatile long layer_count                                             __attribute__((at(CCM_LAYER_COUNT_ADD)));           /*!< */
  volatile long current_layer                                           __attribute__((at(CCM_CURRENT_LAYER_ADD)));         /*!< */
}

#ifdef __cplusplus
} //extern "C" {
#endif

