#ifndef FILAMENTCONTROL_H
#define FILAMENTCONTROL_H

#include "cmsis_os.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
 * 进退丝操作
 */
class FilamentControl
{
public:
  FilamentControl();
  void startLoad(void);                ///< 开始进丝
  void startUnload(void);              ///< 开始退丝
  void process(void);                  ///< 执行进退丝入口
  void cancelProcess(void);            ///< 取消进退丝
  void resetStatus(void);              ///< 重置进退丝状态
private:
  void prepare(void);                  ///< 准备进退丝操作
  void processLoad(void);              ///< 执行进丝操作
  void processUnload(void);            ///< 执行退丝操作
  void exit(bool isCancel);            ///< 退出进退丝
private:
  uint8_t startLoadFlag;               /*!< 开始进丝标志位 */
  uint8_t startUnloadFlag;             /*!< 开始退丝标志位 */
  uint8_t timeOutFlag;                 /*!< 进退丝超时标志位 */
  unsigned long timeOutTickCount;      /*!< 进退丝超时计数 */
};


extern FilamentControl filamentControl;

#endif // FILAMENTCONTROL_H
