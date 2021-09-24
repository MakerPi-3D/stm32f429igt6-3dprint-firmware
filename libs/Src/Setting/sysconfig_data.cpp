#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_common.h"

#ifdef __cplusplus
extern "C" {
#endif

static FIL sysInfofile;    /* file objects */

// SysConfig
char sys_data[512] ;
uint32_t sys_data_size = 0;                        // sysconfig數據緩存大小
T_SYS t_sys;
//  int keysound=1,alarmsound=1; //按键声音，报警声音
T_SYS_DATA t_sys_data_current;


// sys_data保存数据
void sys_save_data(const char *filePath)
{
  UINT NewInfofile_wr;    /* File R/W count */
  taskENTER_CRITICAL();                                              // 进临界区
  (void)f_unlink(filePath);

  if (f_open(&sysInfofile, filePath, FA_CREATE_NEW | FA_WRITE) == FR_OK)  //打开文件
  {
    (void)f_write(&sysInfofile, sys_data, 512, &NewInfofile_wr);// 写文件
    (void)f_close(&sysInfofile);                                    // 关闭文件
  }

  taskEXIT_CRITICAL();                                              // 出临界区
}

void sys_write_info_to_sd(const char *filePath)
{
  UINT NewInfofile_wr;    /* File R/W count */
  taskENTER_CRITICAL();                                              // 进临界区

  if (f_open(&sysInfofile, filePath, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)  //打开文件
  {
    (void)f_write(&sysInfofile, sys_data, sys_data_size, &NewInfofile_wr);// 写文件
    (void)f_close(&sysInfofile);                                    // 关闭文件
  }

  taskEXIT_CRITICAL();                                              // 出临界区
  #if (1 == DEBUG_SYSCONFIG_CUSTOM)
  USER_DbgLog("%40s\r\n%s", "SysConfig::writeInfoToSD", sys_data);
  #endif
}

void sys_read_info_from_sd(const char *filePath)
{
  UINT Infofile_br;                                                  //已经读写的字节数
  taskENTER_CRITICAL();                                              //进临界区

  if (f_open(&sysInfofile, filePath, FA_READ) == FR_OK)          //打开文件（读）
  {
    /**********文件打开成功**************/
    (void)f_read(&sysInfofile, sys_data, sizeof(sys_data), &Infofile_br); //读取文件，最大512字节
    sys_data_size = Infofile_br;                                          //保存读取到的字节数
    sys_data[sys_data_size] = 0;                                           //在读取到的最后一个字节后面一个地址写0，表示文件结束
    (void)f_close(&sysInfofile);                                        //退出文件
  }
  else
  {
    USER_ErrLog("sysconfig open failed!");                        //文件打开失败，打印错误信息
  }

  taskEXIT_CRITICAL();                                               //退出临界区
}

#ifdef __cplusplus
} // extern "C" {
#endif

