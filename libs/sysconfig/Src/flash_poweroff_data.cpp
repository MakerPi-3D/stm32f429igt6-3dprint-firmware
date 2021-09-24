#include "user_common.h"
#include "flashconfig.h"
#include "user_common.h"
#include "ConfigurationStore.h"
#include "vector_3.h"
#include "flashconfig.h"
#include "globalvariables.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


FLASH_POWEROFF_RECOVERY_T flash_poweroff_recovery_t;


/*
  断电时保存数据到flash，只保存，不擦除
  擦拭flash动作在每次开始打印前、停止打印按钮、取消续打按钮，打印完成后，因为断电瞬间来不及做擦除动作
*/
void flash_save_poweroff_data(void)
{
  uint16_t i;
  uint8_t writeTimes;
  uint32_t address;
  uint8_t isRight;
  uint32_t *pd;
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
  writeTimes = 1;

  while (writeTimes--)
  {
    //    FLASH_If_EraseSectors(FLASH_POWEROFF_DATA_START_ADDR, FLASH_POWEROFF_DATA_END_ADDR);
    address = FLASH_POWEROFF_DATA_START_ADDR;
    pd = (uint32_t *)(&flash_poweroff_recovery_t);

    for (i = 0; i < sizeof(flash_poweroff_recovery_t) / 4; i++)
    {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *pd);
      address += 4;
      pd ++;
    }

    isRight = 1;
    address = FLASH_POWEROFF_DATA_START_ADDR;
    pd = (uint32_t *)(&flash_poweroff_recovery_t);

    for (i = 0; i < sizeof(flash_poweroff_recovery_t) / 4; i++)
    {
      if ((*(__IO uint32_t *) address) != *pd)
      {
        isRight = 0;
      }

      address += 4;
      pd ++;
    }

    if (isRight)
    {
      break;
    }
  }

  HAL_FLASH_Lock();//上锁写保护
}

/*
  擦拭flash 断电续打的数据
*/
void flash_erase_poweroff_data(void)
{
  uint32_t address;
  uint32_t *pd;
  uint16_t i;
  address = FLASH_POWEROFF_DATA_START_ADDR;
  pd = (uint32_t *)(&flash_poweroff_recovery_t);

  for (i = 0; i < sizeof(flash_poweroff_recovery_t) / 4; i++)
  {
    *pd = *((volatile uint32_t *) address);
    address += 4;
    pd ++;
  }

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
  FLASH_If_EraseSectors(FLASH_POWEROFF_DATA_START_ADDR, FLASH_POWEROFF_DATA_END_ADDR);
  HAL_FLASH_Lock();
  t_power_off.flag = 0;
}

#ifdef __cplusplus
}
#endif


