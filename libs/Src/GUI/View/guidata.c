
#include "sysconfig_data.h"
#define TouchDataFile "1:/TouchData.bin"
#define TouchDataFile43 "1:/TouchData43.bin"

#include "fatfs.h"
static FIL SaveFile;

void WriteSaveData(const uint32_t *pBufferWrite, uint32_t NumToWrite)
{
  unsigned int i;
  uint8_t DataBuf[24] = {0};
  char fileName[20];
  memset(fileName, 0, sizeof(fileName));

  if (t_sys_data_current.ui_number == 1 || t_sys_data_current.ui_number == 2)
  {
    strncpy(fileName, TouchDataFile43, strlen(TouchDataFile43));
    fileName[strlen(TouchDataFile43)] = '\0';
  }
  else
  {
    strncpy(fileName, TouchDataFile, strlen(TouchDataFile));
    fileName[strlen(TouchDataFile)] = '\0';
  }

  for (i = 0; i < NumToWrite; i++)
  {
    DataBuf[i * 4] = (uint8_t)pBufferWrite[i];
    DataBuf[i * 4 + 1] = (uint8_t)(pBufferWrite[i] >> 8);
    DataBuf[i * 4 + 2] = (uint8_t)(pBufferWrite[i] >> 16);
    DataBuf[i * 4 + 3] = (uint8_t)(pBufferWrite[i] >> 24);
  }

  taskENTER_CRITICAL();

  f_unlink(fileName);
  if (f_open(&SaveFile, fileName, FA_CREATE_NEW|FA_WRITE) == FR_OK)
  {
    (void)f_write(&SaveFile, DataBuf, NumToWrite * 4, (UINT *)NULL); //写入24字节
    (void)f_close(&SaveFile);
  }

  taskEXIT_CRITICAL();
}

void ReadSaveData(uint32_t *pBufferRead, uint32_t NumToRead)
{
  unsigned int i;
  uint8_t DataBuf[24] = {0};
  char fileName[20];
  memset(fileName, 0, sizeof(fileName));

  if (t_sys_data_current.ui_number == 1 || t_sys_data_current.ui_number == 2)
  {
    strncpy(fileName, TouchDataFile43, strlen(TouchDataFile43));
    fileName[strlen(TouchDataFile43)] = '\0';
  }
  else
  {
    strncpy(fileName, TouchDataFile, strlen(TouchDataFile));
    fileName[strlen(TouchDataFile)] = '\0';
  }

  taskENTER_CRITICAL();

  if (f_open(&SaveFile, fileName, FA_READ) == FR_OK)
  {
    (void)f_read(&SaveFile, DataBuf, NumToRead * 4, (UINT *)NULL); //读取24字节
    (void)f_close(&SaveFile);
  }

  taskEXIT_CRITICAL();

  for (i = 0; i < NumToRead; i++)
  {
    pBufferRead[i] = (uint32_t)DataBuf[i * 4];
    pBufferRead[i] += (((uint32_t)DataBuf[i * 4 + 1]) << 8);
    pBufferRead[i] += (((uint32_t)DataBuf[i * 4 + 2]) << 16);
    pBufferRead[i] += (((uint32_t)DataBuf[i * 4 + 3]) << 24);
  }
}

