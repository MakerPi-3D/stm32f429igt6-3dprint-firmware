#ifndef POWEROFFRECOVERY_H
#define POWEROFFRECOVERY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//#include "stm32f4xx_hal.h"

#ifdef __cplusplus
} //extern "C" {
#endif

/**
 * 断电保存、恢复、校正Z高度操作
 */
class PowerOffOperation
{
public:
  PowerOffOperation();

  void startCalculateZMaxPos(void);                  ///< 校正Z高度
  void getZMaxPos(void);                             ///< 获取Z高度值
  void stopGetZMaxPos(void);                         ///< 获取Z高度值
  void resetFlag(void);                              ///< 重置断电标志位
  void setFilePathName(const char *filePathName);    ///< 设置文件路径名称
  void setData(void);                                ///< 设置保存数据
  void syncData(void);                               ///< 保存断电数据
  void readyToRecoveryPrint(void);                   ///< 恢复打印前，执行一些准备操作
  void recoveryPrint(void);                          ///< 恢复打印
  void recoveryPrintLoop(void);                      ///< 恢复打印流程
  void recoveryPrintFinish(void);                    ///< 恢复打印
  void deleteFileFromSD(void);                       ///< 删除SD卡断电文件
private:
  void setDataBuf(void);                             ///< 设置断电数据缓存
  void saveFileBak(void);                            ///< 恢复打印（或者取消打印）时，先将断电文件备份，误操作时可恢复打印
private:
  bool IsStartCalculateZMaxPos;                      /*!< 是否开启校正Z高度 */
  char powerOffFilePathName[100];                    /*!< 断电文件路径名 */
  bool isPowerOffRecoverPrint;                       /*!< 是否为断电恢复操作 */
  bool isPowerOffRecoverPrintFinish;                 /*!< 是否为断电恢复操作 */
  bool isStopGetZMax;                                /*!< 是否停止校准z高度标志位 */
};

/**
 * 读取PowerOffData.txt文件，解析文件内容，初始化对应结构体
 */
class PowerOffRecovery
{
public:
  PowerOffRecovery();

  void init(void);                                   ///< 初始化断电数据
  void initGlobalVariables(void);                    ///< 初始化全局变量
private:
  void readPowerOffData(void);                       ///< 读取断电保存数据
  void explainPowerOffData(void);                    ///< 解析断电保存数据
  void getFlag(void);                                ///< 获取断电标志位
  void getSerialFlag(void);                          ///< 获取串口标志位
  void getBedTargetTemp(void);                       ///< 获取热床目标温度
  void getNozzleTargetTemp(void);                    ///< 获取喷嘴目标温度
  void getFanSpeed(void);                            ///< 获取风扇速度
  void getFeedMultiply(void);                        ///< 获取进料速度比
  void getFeedRate(void);                            ///< 获取进料速度
  void getXPos(void);                                ///< 获取X轴位置
  void getYPos(void);                                ///< 获取Y轴位置
  void getZPos(void);                                ///< 获取Z轴位置
  void getEPos(void);                                ///< 获取E轴位置
  void getBPos(void);                                ///< 获取B轴位置
  void getSDPos(void);                               ///< 获取文件位置
  void getPathFileName(void);                        ///< 获取文件路径名
  void powerOffRecoveryLog(void);                    ///< 打印已经读取断电数据
  void getblockflag(void);                           ///< 获取堵料状态
  void getPrintTime(void);
private:
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
  char pathFileName[100];                            /*!< 文件路径名 */
  char fileName[100];                                /*!< 文件名 */
  uint8_t flag;                                      /*!< 断电标志位 */
  uint8_t serialFlag;                                /*!< 串口标志位 */
  uint32_t blockflag;                                /*!< 堵料标志位 */
};

extern PowerOffOperation powerOffOperation;

#endif // POWEROFFRECOVERY_H

