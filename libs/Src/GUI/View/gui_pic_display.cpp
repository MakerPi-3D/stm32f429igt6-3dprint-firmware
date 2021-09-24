#include "machinecustom.h"
#include "jshdisplay.h"
#include "globalvariables.h"
#include "user_common.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"
#include "user_ccm.h"

#ifdef __cplusplus
extern "C" {
#endif

// 图片包文件头结构
typedef struct Pic_SMaseFileHeader
{
  uint32_t  uFileFlag;          // 包文件头标记: 'MASE'
  uint32_t  uFileCount;         // 包内文件个数
  uint32_t  uFileListOfs;       // 文件列表偏移
  uint32_t  uMaxFileCount;      // 最大子文件个数
  uint32_t  uFileSize;          // 包文件的大小
} Pic_MaseHeader;

//包内文件信息结构
typedef struct Pic_SFilesMessage
{
  uint32_t  uFileOfs;          // 本文件在包内的偏移
  uint32_t  uFileSize;         // 本文件的大小
  char  szFileName[60];   // 本文件的文件名，包含路径
} Pic_FilesMsg;
Pic_FilesMsg   Pic_fileMsg;      //文件信息结构

enum PIC_TYPE
{
  DISPLAY_PIC = 0, // 界面显示
  LOGO_PIC = 1,   // LOGO显示
  MODEL_PIC,      //模型预览图片显示
};

static FIL PictureFile;

/***********************指定区域显示 bmp图片*********************
* @name    : diplayBMP
* @brief   : 将bmp源图画到3.5寸TFT屏幕上，bmp图片大小210x230；显示区域为X:254-479，Y:0-234；可以设置下面的宏定义偏移图片位置
* @param   : y_offset: 显示图片时指定y的偏移位置，取值范围理论上是0~320,但要图片正常显示就应该根据图片高度设置
* @retval  : 无
* @notice  ：宏定义用来设置BMP文件大小，行数，列数，以及画图时X轴相对于原点的偏移位置；
             结构体用来画图时暂存bmp文件的各项数据，实现画图；
* @author  ：John
* @date    ：2017/09/26
* @versions：V1.1
****************************************************************/
//#include "PrintControl.h"
//位图文件头结构体，14个字节。BMP图片文件固定包含的数据
//typedef  struct  tagBITMAPFILEHEADER{
//    uint16_t bfType;            //指定文件类型，0x424D==”BM”，处于文件最开始的两个字节
//    uint32_t bfSize;            //指定文件大小，包括这14个字节
//    uint16_t bfReserved1;       //保留字，无内容
//    uint16_t bfReserved2;       //保留字，为内容
//    uint32_t bfOffBits;         //从文件头到图像实际数据偏移字节数，即前三部分之和
//}BMPfileheader;                 // BITMAPFILEHEADER;

//#define BMPsize 96600     //210x230像素16位图的实际图像数据大小
//#define BMPROWNUM  230    //行数
//#define BMPROWBYTE 200*2  //每行所占字节数，列数*2
#define X_OFFSET 270      //x轴偏移位置
//#define Y_OFFSET 4        //y轴偏移位置

//typedef struct S_DISPLAYBMP
//{
//  uint32_t file_size;          //缓存bmp图片的位图数据大小
//  uint32_t index;              //缓存bmp图片位图数据偏移地址
//  uint16_t currentcolor;       //缓存当前bmp像素点的颜色值
//  int32_t currentRow;          //缓存画图时的当前行
//  uint32_t RCount;             //使用读文件函数f_read时需要提供参数记录读取到的字节数
//} S_displaybmp;

//void diplayBMP(int y_offset)
//{
//  char * buf;
//  S_displaybmp *displaybmp;
//  FRESULT res=FR_DISK_ERR;

//  taskENTER_CRITICAL();   //会关闭中断，包括电机中断，打开文件消耗的时间较长，会造成电机停顿感

//  res = f_open(&PictureFile, BMP_PATH, FA_READ);
//  if(res != FR_OK)
//  {
//    USER_ErrLog("Open file fail!!\t f_res = %d",res);
//    taskEXIT_CRITICAL();
//    return;
//  }

//  displaybmp = (S_displaybmp *)malloc(sizeof(S_displaybmp));
//  displaybmp->currentRow = BMPROWNUM;

//  // 获取文件大小
//  displaybmp->file_size = BMPsize;
//  buf = (char *)malloc(BMPROWBYTE);
//  f_read(&PictureFile, buf, 14, &displaybmp->RCount);

//  //获取位图实际数据位置
//  displaybmp->index = *(uint16_t *) (buf + 10);          //获取bmp像素位置
//  displaybmp->index |= (*(uint16_t *) (buf + 12)) << 16;  //根据位图文件头结构体得来
//  f_lseek(&PictureFile, displaybmp->index);                 //设置bmp像素位置
//  while(displaybmp->file_size>0)
//  {
//    displaybmp->currentRow--;//当前行减1
//    if(displaybmp->currentRow >= 0) //行数未画完
//    {
//      f_lseek(&PictureFile, displaybmp->index);                 //设置bmp像素位置
//      f_read(&PictureFile,buf,BMPROWBYTE,&displaybmp->RCount);   //读取bmp像素
//    }
//    else  break;//跳出循环
//    //往屏幕画一行的像素点
//    for(uint16_t i=0; i < BMPROWBYTE; i+=2)
//    {
//      displaybmp->currentcolor = *(uint16_t*)(buf+i);
//      LCD_Fast_DrawPoint(X_OFFSET+(i>>1),displaybmp->currentRow+y_offset,displaybmp->currentcolor);
//    }
//    displaybmp->file_size-=BMPROWBYTE;//文件减一行的字节数
//    displaybmp->index+=BMPROWBYTE;//文件偏移加一行字节数
//  }
//  res = f_close(&PictureFile);
//  if(res) {;}

//  free(buf);
//  free(displaybmp);
//  taskEXIT_CRITICAL();
//}

static void _displayPicture(int PictureName, int picType, unsigned int y_offset)
{
  unsigned int LCDWidth = 480;
  //参数p数组地址，参数jsh备用，参数psize数组大小
  taskENTER_CRITICAL();   //会关闭中断，包括电机中断，打开文件消耗的时间较长，会造成电机停顿感

  if ((F400TP == t_sys_data_current.model_id) || (F1000TP == t_sys_data_current.model_id))
    LCDWidth = 1024;

  if (DISPLAY_PIC == picType)
  {
    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id) //中文图片
    {
      if ((F400TP == t_sys_data_current.model_id) || (F1000TP == t_sys_data_current.model_id))
        f_open(&PictureFile, "1:/C_Picture_70.bin", FA_READ);  //打开图片文件
      else if (K5 == t_sys_data_current.model_id || P2_Pro == t_sys_data_current.model_id || P3_Pro == t_sys_data_current.model_id)
        (void)f_open(&PictureFile, "1:/C_Picture_43.bin", FA_READ);  //打开图片文件
      else
        (void)f_open(&PictureFile, "1:/C_Picture.bin", FA_READ);  //打开图片文件
    }
    else if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) //日文图片
      (void)f_open(&PictureFile, "1:/J_Picture.bin", FA_READ);  //打开图片文件
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id) //英文图片
    {
      if (K5 == t_sys_data_current.model_id || P2_Pro == t_sys_data_current.model_id || P3_Pro == t_sys_data_current.model_id)
        (void)f_open(&PictureFile, "1:/EN_Picture_43.bin", FA_READ);  //打开图片文件
      else
        (void)f_open(&PictureFile, "1:/EN_Picture.bin", FA_READ);  //打开图片文件
    }
    else if (PICTURE_IS_KOREA == t_sys_data_current.pic_id) //韩文图片
      (void)f_open(&PictureFile, "1:/KOR_Picture.bin", FA_READ);  //打开图片文件
    else if (PICTURE_IS_RUSSIA == t_sys_data_current.pic_id) //俄文图片
      (void)f_open(&PictureFile, "1:/RUS_Picture.bin", FA_READ);  //打开图片文件
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id) //俄文图片
    {
      if (K5 == t_sys_data_current.model_id)
        (void)f_open(&PictureFile, "1:/C_T_Picture_43.bin", FA_READ);  //打开图片文件
      else
        (void)f_open(&PictureFile, "1:/C_T_Picture.bin", FA_READ);  //打开图片文件
    }
  }
  else if (LOGO_PIC == picType)
    (void)f_open(&PictureFile, "1:/L_Picture.bin", FA_READ);  //打开图片文件
  else if (MODEL_PIC == picType)
  {
    f_open(&PictureFile, BMP_PATH, FA_READ);
    LCDWidth = 200;//预览图片的宽是200
  }

  unsigned int RCount;
  unsigned int ColorCount = 0;
  uint16_t CurrentColor, CurrentColorNum;
  uint16_t iwidth, iheight;
  uint16_t iprewidth = 0, ipreheight = 0;
  uint32_t PictureSize;
  uint32_t x_offset = 0;

  if (MODEL_PIC != picType)
  {
    //    (void)f_lseek(&PictureFile,(PictureName-1)*sizeof(FilesMsg)+sizeof(MaseHeader));
    //    (void)f_read (&PictureFile, &fileMsg, sizeof(FilesMsg), &RCount);
    //    (void)f_lseek(&PictureFile,fileMsg.uFileOfs);
    (void)f_lseek(&PictureFile, (PictureName - 1)*sizeof(Pic_FilesMsg) + sizeof(Pic_MaseHeader));
    (void)f_read(&PictureFile, &Pic_fileMsg, sizeof(Pic_FilesMsg), &RCount);

    if (Pic_fileMsg.uFileOfs)
      (void)f_lseek(&PictureFile, Pic_fileMsg.uFileOfs);

    PictureSize = Pic_fileMsg.uFileSize;
  }
  else
  {
    PictureSize = f_size(&PictureFile) - (t_sys.lcd_ssd1963_43_480_272 ? (200 * 25) : 0);
    x_offset = X_OFFSET;
    iprewidth += x_offset;
    ipreheight += y_offset;
  }

  while (PictureSize > 0)
  {
    if (PictureSize >= PIC_BUF_SIZE)
    {
      (void)f_read(&PictureFile, ccm_param::PictureFileBuf, PIC_BUF_SIZE, &RCount);
      PictureSize = PictureSize - PIC_BUF_SIZE;
    }
    else
    {
      (void)f_read(&PictureFile, ccm_param::PictureFileBuf, PictureSize, &RCount);
      PictureSize = 0;
    }

    for (int i = 0; i < RCount; i++)
    {
      CurrentColor = (ccm_param::PictureFileBuf[i] << 8);    //16（565）位颜色的高八位
      ++i;
      CurrentColor |= ccm_param::PictureFileBuf[i];          //16（565）位颜色的低八位
      ++i;
      CurrentColorNum = (ccm_param::PictureFileBuf[i] << 8); //这个颜色的个数的高八位
      ++i;
      CurrentColorNum |= ccm_param::PictureFileBuf[i];     //这个颜色的个数的低八位
      ColorCount += CurrentColorNum;
      iwidth = ColorCount % LCDWidth + x_offset; //压缩bmp时是按顺序一行行（一行Width个像素）压缩bmp的，所以像素个数的对LCDWidth求余就得出每行有几个这样的像素。
      iheight = ColorCount / LCDWidth + y_offset; //按压缩算法规律，对LCDWidth除，即可得出该种颜色的行数

      //以下使用lcd_fill显示颜色，根据条件分三种情况:
      if (iheight > ipreheight) //某种像素个数超过一行（480个）
      {
        if (iheight > ipreheight + 1) //某种像素个数超过两行（480*2）
        {
          LCD_Fill_Picture(iprewidth, ipreheight, LCDWidth - 1 + x_offset, ipreheight, CurrentColor); //从上一行接着画点，直到行尾
          LCD_Fill_Picture(0 + x_offset, ipreheight + 1, LCDWidth - 1 + x_offset, iheight - 1, CurrentColor); //画某个像素：长为LCDWidth，宽为（iheight-1）-（ipreheight+1）的矩形
          LCD_Fill_Picture(0 + x_offset, iheight, iwidth, iheight, CurrentColor); //画某个像素最后一行
        }
        else//if(iheight==ipreheight+1)//某个像素个数刚好一行（480个）
        {
          LCD_Fill_Picture(iprewidth, ipreheight, LCDWidth - 1 + x_offset, iheight - 1, CurrentColor); //从上一行接着画点，知道行尾
          LCD_Fill_Picture(0 + x_offset, iheight, iwidth, iheight, CurrentColor);             //从新的一行开头画点,直到该像素在该行结束
        }
      }
      else //if(ipreheight=iheight)//某个像素个数不超过一行
      {
        LCD_Fill_Picture(iprewidth, ipreheight, iwidth, iheight, CurrentColor);
      }

      iprewidth = iwidth;
      ipreheight = iheight;
    }
  }

  f_close(&PictureFile);
  taskEXIT_CRITICAL();
}

void display_picture(int PictureName)
{
  _displayPicture(PictureName, DISPLAY_PIC, 0);
}

void DisplayLogoPicture(int PictureName)
{
  _displayPicture(PictureName, LOGO_PIC, 0);
}

void diplayBMP(unsigned int y_offset)
{
  _displayPicture(0, MODEL_PIC, y_offset);
}

uint32_t Draw_progressBar_new(uint32_t Printfilesize, uint32_t Filesize, int x, int y, int x_max, int y_max)
{
  uint32_t PrintPercent;
  static uint32_t Pre_PrintPercent = 1;
  PrintPercent = (uint32_t)((float)(Printfilesize - Filesize) / Printfilesize * (float)x_max);

  if (PrintPercent != Pre_PrintPercent)
  {
    LCD_Fill(x, y, x + PrintPercent, y + y_max, YELLOW);
    Pre_PrintPercent = PrintPercent;
    return 1;
  }

  return 0;
}

uint32_t Draw_progressBar(uint32_t Printfilesize, uint32_t Filesize)
{
  return Draw_progressBar_new(Printfilesize, Filesize, X_BEGIN, Y_BEGIN, BarWidth, BarHeight);
}

#ifdef __cplusplus
} //extern "C" {
#endif


