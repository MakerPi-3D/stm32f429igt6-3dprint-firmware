#ifndef JSHDISPLAY_H
#define JSHDISPLAY_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 图片包文件头结构
typedef struct SMaseFileHeader
{
  uint32_t  uFileFlag;          // 包文件头标记: 'MASE'
  uint32_t  uFileCount;         // 包内文件个数
  uint32_t  uFileListOfs;       // 文件列表偏移
  uint32_t  uMaxFileCount;      // 最大子文件个数
  uint32_t  uFileSize;          // 包文件的大小
} MaseHeader;

//包内文件信息结构
typedef struct SFilesMessage
{
  uint32_t  uFileOfs;          // 本文件在包内的偏移
  uint32_t  uFileSize;         // 本文件的大小
  char  szFileName[60];   // 本文件的文件名，包含路径
} FilesMsg;

void display_picture(int PictureName);
void DisplayLogoPicture(int PictureName);

#define BMP_PATH "0:/file2.bmp"//提取sgcode中的bmp文件后的存放路径20170920
//画进度条
#define BarWidth 382  //进度条长度
#define BarHeight 21  //进度条宽度
#define X_BEGIN 75    //进度条x轴起始位置
#define Y_BEGIN 289   //进度条y轴起始位置
uint32_t Draw_progressBar_new(uint32_t Printfilesize, uint32_t Filesize, int x, int y, int x_max, int y_max);
uint32_t Draw_progressBar(uint32_t Printfilesize, uint32_t Filesize);
void diplayBMP(unsigned int y_offset);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
