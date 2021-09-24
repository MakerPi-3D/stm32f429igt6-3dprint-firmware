#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flashconfig.h"
#include "user_common.h"
#include "ConfigurationStore.h"
#include "vector_3.h"
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif

// 0x080ff800~0x081000000  2kb
#define FLASH_PARAM_START_ADDR ((uint32_t)0x080ff800)
#define FLASH_PARAM_END_ADDR   ((uint32_t)0x080fffff)
#define FLASH_PARAM_PAGE_SIZE  ((uint16_t)0x800)


FLASH_PARAM_T flash_param_t;

extern matrix_3x3 plan_bed_level_matrix;

void flash_save_data(void)
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
    FLASH_If_EraseSectors(FLASH_PARAM_START_ADDR, FLASH_PARAM_END_ADDR);
    address = FLASH_PARAM_START_ADDR;
    pd = (uint32_t *)(&flash_param_t);

    for (i = 0; i < sizeof(flash_param_t) / 4; i++)
    {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *pd);
      address += 4;
      pd ++;
    }

    isRight = 1;
    address = FLASH_PARAM_START_ADDR;
    pd = (uint32_t *)(&flash_param_t);

    for (i = 0; i < sizeof(flash_param_t) / 4; i++)
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

void flash_read_data(void)
{
  uint32_t address;
  uint32_t *pd;
  uint16_t i;
  address = FLASH_PARAM_START_ADDR;
  pd = (uint32_t *)(&flash_param_t);

  for (i = 0; i < sizeof(flash_param_t) / 4; i++)
  {
    *pd = *((volatile uint32_t *) address);
    address += 4;
    pd ++;
  }

  USER_EchoLogStr("flash_param_t.version ===== %d\n", flash_param_t.version);

  if (flash_param_t.version == FLASH_PARAM_VERSION)
  {
    flash_param_t.flag = 0;

    //    for (int i = 0; i < 5; i++)
    //    {
    //      axis_steps_per_unit[i] = axis_steps_per_unit[i] * flash_param_t.axis_steps_per_unit_factor[i];
    //      USER_EchoLogStr("axis = %d, axis_steps_per_unit = %f, factor = %f\n", i, axis_steps_per_unit[i], flash_param_t.axis_steps_per_unit_factor[i]);
    //    }

    if (flash_param_t.is_process_bedlevel == 1)
    {
      for (int i = 0; i < 9; i++)
      {
        plan_bed_level_matrix.matrix[i] = flash_param_t.bed_level_matrix_value[i];
      }
    }
    else
    {
      plan_bed_level_matrix.set_to_identity();

      if (flash_param_t.is_process_bedlevel != 2)
      {
        flash_param_t.matrix_back_right.set_to_identity();
        flash_param_t.matrix_front_left.set_to_identity();
        flash_param_t.matrix_left_back.set_to_identity();
        flash_param_t.matrix_right_front.set_to_identity();
        plan_bed_level_matrix.set_to_identity();

        for (int i = 0; i < 9; i++)
        {
          flash_param_t.bed_level_matrix_value[i] = plan_bed_level_matrix.matrix[i];
        }
      }
    }

    for (int i = 0; i < 3; i++)
    {
      if (isnan(flash_param_t.dual_home_pos_adding[i]))
      {
        flash_param_t.dual_home_pos_adding[i] = 0.0f;
      }

      if (isnan(flash_param_t.dual_extruder_1_offset[i]))
      {
        flash_param_t.dual_extruder_1_offset[i] = 0.0f;
      }

      if (isnan(flash_param_t.idex_ext1_ext0_offset[i]))
      {
        flash_param_t.idex_ext1_ext0_offset[i] = flash_param_t.dual_extruder_1_offset[i];
      }

      if (isnan(flash_param_t.mix_ext0_home_pos_adding[i]))
      {
        flash_param_t.mix_ext0_home_pos_adding[i] = 0.0f;
      }

      if (isnan(flash_param_t.laser_ext0_home_pos_adding[i]))
      {
        flash_param_t.laser_ext0_home_pos_adding[i] = 0.0f;
      }
    }

    for (int i = 0; i < 2; i++)
    {
      if (isnan(flash_param_t.idex_extruder_0_bed_offset[i]))
      {
        flash_param_t.idex_extruder_0_bed_offset[i] = 0.0f;
      }

      if (isnan(flash_param_t.idex_extruder_1_bed_offset[i]))
      {
        flash_param_t.idex_extruder_1_bed_offset[i] = 0.0f;
      }

      if (isnan(flash_param_t.mix_extruder_0_bed_offset[i]))
      {
        flash_param_t.mix_extruder_0_bed_offset[i] = 0.0f;
      }

      if (isnan(flash_param_t.laser_extruder_0_bed_offset[i]))
      {
        flash_param_t.laser_extruder_0_bed_offset[i] = 0.0f;
      }

      if (isnan(flash_param_t.laser_extruder_1_bed_offset[i]))
      {
        flash_param_t.laser_extruder_1_bed_offset[i] = 0.0f;
      }
    }

    //设置默认值

    if (flash_param_t.idex_extruder_0_bed_offset[0] == 0) // Idex喷头1热床位置偏移量
      flash_param_t.idex_extruder_0_bed_offset[0] = 50;

    if (flash_param_t.idex_extruder_0_bed_offset[1] == 0) // Idex喷头1热床位置偏移量
      flash_param_t.idex_extruder_0_bed_offset[1] = 350;

    if (flash_param_t.idex_extruder_1_bed_offset[0] == 0) // Idex喷头2热床位置偏移量
      flash_param_t.idex_extruder_1_bed_offset[0] = 6;

    if (flash_param_t.idex_extruder_1_bed_offset[1] == 0) // Idex喷头2热床位置偏移量
      flash_param_t.idex_extruder_1_bed_offset[1] = 306;

    if (flash_param_t.mix_extruder_0_bed_offset[0] == 0) // 混色喷头热床位置偏移量
      flash_param_t.mix_extruder_0_bed_offset[0] = 55;

    if (flash_param_t.mix_extruder_0_bed_offset[1] == 0) // 混色喷头热床位置偏移量
      flash_param_t.mix_extruder_0_bed_offset[1] = 350;

    if (flash_param_t.laser_extruder_0_bed_offset[0] == 0) // 激光头1热床位置偏移量
      flash_param_t.laser_extruder_0_bed_offset[0] = 50;

    if (flash_param_t.laser_extruder_0_bed_offset[1] == 0) // 激光头1热床位置偏移量
      flash_param_t.laser_extruder_0_bed_offset[1] = 350;

    if (flash_param_t.laser_extruder_1_bed_offset[0] == 0) // 激光头2热床位置偏移量
      flash_param_t.laser_extruder_1_bed_offset[0] = 6;

    if (flash_param_t.laser_extruder_1_bed_offset[1] == 0) // 激光头2热床位置偏移量
      flash_param_t.laser_extruder_1_bed_offset[1] = 310;

    for (int i = 0; i < 2; i++)//Z不设置
    {
      if (flash_param_t.idex_ext0_home_pos_adding[i] == 0)   // Idex喷头1归零点偏移，主要用于红外检测Z归零
        flash_param_t.idex_ext0_home_pos_adding[i] = 60;

      if (flash_param_t.mix_ext0_home_pos_adding[i] == 0)   // 混色喷头1归零点偏移，主要用于红外检测Z归零
        flash_param_t.mix_ext0_home_pos_adding[i] = 60;

      if (flash_param_t.laser_ext0_home_pos_adding[i] == 0) // 激光喷头1归零点偏移，主要用于红外检测Z归零
        flash_param_t.laser_ext0_home_pos_adding[i] = 60;
    }
  }
  else
  {
    memset((char *)&flash_param_t, 0, sizeof(flash_param_t));
    flash_param_t.version = FLASH_PARAM_VERSION;

    for (int i = 0; i < 5; i++)
    {
      flash_param_t.axis_steps_per_unit_factor[i] = 1.0f;
    }

    flash_save_data();
  }
}

void flash_param_process(void)
{
  //  static bool get_flash_once = false;
  static uint32_t flash_save_timeout = 0;
  //  if (!get_flash_once)
  //  {
  //    taskENTER_CRITICAL();
  //    flash_read_data();
  //    taskEXIT_CRITICAL();
  //    get_flash_once = true;
  //  }
  //  else
  {
    if (flash_save_timeout < MILLIS())
    {
      flash_save_timeout = MILLIS() + 1000;

      //USER_EchoLogStr("axis_steps_per_unit = %f, factor = %f\n" , axis_steps_per_unit[0], flash_param_t.axis_steps_per_unit_factor[0]);
      if (flash_param_t.flag == 1)
      {
        USER_EchoLogStr("flash_param save\n");
        flash_param_t.flag = 0;
        taskENTER_CRITICAL();
        flash_save_data();
        taskEXIT_CRITICAL();
      }
    }
  }
}


#ifdef __cplusplus
}
#endif


