#include "common.h"
#include "sysconfig_data.h"
#include "user_common.h"
#include "controlfunction.h"
#include "threed_engine.h"
#include "config_model_tables.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} //extern "C" {
#endif

namespace gui
{
  volatile menufunc_t current_display;
  volatile int rtc_flag = 0;           //实时rtc刷新信号
  static volatile int refresh_flage = 1;      //界面刷新信号

  void need_refresh(void)            //界面刷新函数
  {
    osDelay(50);
    refresh_flage = 1;
  }

  bool is_refresh(void)            //检测界面是否需要刷新
  {
    osDelay(50);

    if (refresh_flage)
    {
      refresh_flage = 0;
      return true;
    }
    else
    {
      return false;
    }
  }

  bool is_refresh_rtc(void)
  {
    osDelay(50);

    if (rtc_flag)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  void set_current_display(const volatile menufunc_t display)
  {
    need_refresh();
    current_display = display;
  }
}

#ifdef __cplusplus
extern "C" {
#endif

u16 testcolor = 0xf800;     //数值显示的颜色

volatile int print_flage = 0;        //正在打印标志
volatile int pauseprint = 0;         //暂停打印标志
//int keysound=1,alarmsound=1; //按键声音，报警声音

#define TICKCOUNT  100
int IstouchxyDown(int touchx, int touchy, int rangex, int rangey) //add by zouxb
{
  if (lcddev.id == 0x7016)
  {
    touchx *= 2.13;
    touchy *= 1.875;
    rangex *= 2.13;
    rangey *= 1.875;
  }

  if (touch.touch_flag &&
      touch.pixel_x > touchx &&
      touch.pixel_x <  rangex &&
      touch.pixel_y > touchy &&      //触摸区域
      touch.pixel_y < rangey)       //touchx<x<rangex;touchy<y<rangey
  {
    if (t_sys.key_sound)              //如果设置了按键响声
    {
      user_buzzer_buzz(TICKCOUNT);     //响铃函数
    }

    osDelay(50);
    return 1;
  }
  else
  {
    return 0;
  }
}

int IstouchxyUp(void)  //add by zouxb
{
  if (touch.up_flag)
  {
    touch.touch_flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}


int touchxy(int touchx, int touchy, int rangex, int rangey)
{
  if (lcddev.id == 0x7016)
  {
    touchx *= 2.13;
    touchy *= 1.875;
    rangex *= 2.13;
    rangey *= 1.875;
  }

  if (touch.up_flag &&
      touch.touch_flag &&
      touch.pixel_x > touchx &&
      touch.pixel_x <  rangex &&
      touch.pixel_y > touchy &&
      touch.pixel_y < rangey) //touchx<x<rangex;touchy<y<rangey  //触摸区域
  {
    if (t_sys.key_sound)              //如果设置了按键响声
    {
      user_buzzer_buzz(TICKCOUNT);     //响铃函数
    }

    osDelay(50);
    return 1;
  }
  else
  {
    return 0;
  }
}

int TouchXY_NoBeep(int touchx, int touchy, int rangex, int rangey)
{
  if (lcddev.id == 0x7016)
  {
    touchx *= 2.13;
    touchy *= 1.875;
    rangex *= 2.13;
    rangey *= 1.875;
  }

  if (touch.up_flag &&
      touch.touch_flag &&
      touch.pixel_x > touchx &&
      touch.pixel_x <  rangex &&
      touch.pixel_y > touchy &&
      touch.pixel_y < rangey) //touchx<x<rangex;touchy<y<rangey  //触摸区域
  {
    (void)OS_DELAY(200);
    return 1;
  }
  else
  {
    return 0;
  }
}

int touchrange(int touchx, int touchy, int rangex, int rangey)
{
  if (lcddev.id == 0x7016)
  {
    touchx *= 2.13;
    touchy *= 1.875;
    rangex *= 2.13;
    rangey *= 1.875;
  }

  if (touch.up_flag && touch.touch_flag &&
      touch.pixel_x > touchx &&
      touch.pixel_x < touchx + rangex &&
      touch.pixel_y > touchy &&
      touch.pixel_y < touchy + rangey)       //触摸区域
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

struct _touch touch = {0};
void guitouchscan(void)        //触摸扫描函数
{
  (void)tp_dev.scan(0);

  if (tp_dev.sta & (0x80))   //有触摸设置相关参数
  {
    touch.touch_flag = 1;
    touch.down_flag = 1;
    touch.up_flag = 0;

    if (1 == is_touch_capacitive && (lcddev.id == 0X7016)) // 7寸电容触摸
    {
      touch.pixel_x = tp_dev.x[0];
      touch.pixel_y = tp_dev.y[0];
    }
    else
    {
      touch.pixel_x = tp_dev.x[1];
      touch.pixel_y = tp_dev.y[1];
    }
  }
  else                       //没触摸设置相关参数
  {
    touch.up_flag = 1;
    touch.down_flag = 0;
  }
}

//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小
//size:字体大小
FIL hanziziku;
void Get_HzMat(const unsigned char *code, unsigned char *mat, u8 size)
{
  unsigned char qh, ql;
  unsigned char i;
  unsigned long foffset;
  unsigned int readbyte;
  u8 csize = (u8)((size / 8 + ((size % 8) ? 1 : 0)) * (size)); //得到字体一个字符对应点阵集所占的字节数
  qh = *code;
  ql = *(++code);

  if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff) //非 常用汉字
  {
    for (i = 0; i < csize; i++)*mat++ = 0x00; //填充满格

    return; //结束访问
  }

  if (ql < 0x7f)ql -= 0x40; //注意!
  else ql -= 0x41;

  qh -= 0x81;
  foffset = ((unsigned long)190 * qh + ql) * csize; //得到字库中的字节偏移量

  switch (size)
  {
  case 12:
    break;

  case 16:
    break;

  case 24:
    taskENTER_CRITICAL();
    (void)f_open(&hanziziku, "1:/GBK24.FON", FA_READ);  //打開字庫文件
    (void)f_lseek(&hanziziku, foffset);
    (void)f_read(&hanziziku, mat, csize, &readbyte);
    (void)f_close(&hanziziku);
    taskEXIT_CRITICAL();
    break;

  default:
    break;
  }
}

//设置文字显示的区域范围
void SetTextDisplayRange(int x, int y, int rangex, int rangey, struct _textrange *p)
{
  if (t_sys.lcd_ssd1963_43_480_272 && lcddev.height == 272)
  {
    X_1963(x);
    Y_1963(y);
  }

  p->x = x;
  p->y = y;
  p->rangex = rangex;
  p->rangey = rangey;
}

//设置文字显示的区域范围
void SetTextDisplayRangeNormal(int x, int y, int rangex, int rangey, struct _textrange *p)
{
  p->x = x;
  p->y = y;
  p->rangex = rangex;
  p->rangey = rangey;
}

//将指定区域内的像素读出
void ReadTextDisplayRangeInfo(struct _textrange textrange, u16 *p)
{
  int i, ii;

  for (i = 0; i < textrange.rangey; i++)
  {
    for (ii = 0; ii < textrange.rangex; ii++)
    {
      *p++ = LCD_ReadPoint((u16)(ii + textrange.x), (u16)(i + textrange.y));
    }
  }
}

//复制一份读出的像素数据
void CopyTextDisplayRangeInfo(struct _textrange textrange, const u16 *psource, u16 *p)
{
  int i, ii;

  for (i = 0; i < textrange.rangey; i++)
  {
    for (ii = 0; ii < textrange.rangex; ii++)
    {
      *p++ = *psource++;
    }
  }
}

//将合成了文字信息的像素数据重新写入指定区域
void WriteTextDisplayRangeInfo(struct _textrange textrange, const u16 *p)
{
  int i, ii;

  for (i = 0; i < textrange.rangey; i++)
  {
    for (ii = 0; ii < textrange.rangex; ii++)
    {
      LCD_Fast_DrawPoint((u16)(textrange.x + ii), (u16)(textrange.y + i), *p++);
    }
  }
}

//将单个英文字符点阵信息合成到读出的像素数据中
void WriteEnglishCharToBuf(struct _textrange textrange, const unsigned char *pstr, int XHeadPos, u16 *p, int size, u16 colour)
{
  //参数*pstr字符，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  int CharCodeNum, BitData, YPos = 0, XPos = -1;
  unsigned char CharCode;
  unsigned char CharPos = *pstr - ' ';  //偏移量

  if (size == 24) //24*12的字体
  {
    for (CharCodeNum = 0; CharCodeNum < 36; CharCodeNum++) //一个24*12字符对应36个字节数据
    {
      CharCode = asc2_2412[CharPos][CharCodeNum]; //24*12字符码  //从上到下，从左到右，高位在前

      if ((CharCodeNum % 3) == 0) //3个字节为1列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; BitData++) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          p[XHeadPos + XPos + YPos * textrange.rangex] = colour; //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        YPos++; //下一行
      }
    }
  }
}

//将单个中文字符点阵信息合成到读出的像素数据中
void WriteChineseCharToBuf(struct _textrange textrange, const unsigned char *pstr, int XHeadPos, u16 *p, int size, u16 colour)
{
  //参数*pstr字符，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  int CharCodeNum, BitData, YPos = 0, XPos = -1;
  unsigned char CharCode;
  unsigned char dzk[72]; //一个汉字的点阵数据72字节Buf

  if (size == 24)
  {
    Get_HzMat(pstr, dzk, (u8)size); //得到72字节的点阵数据

    for (CharCodeNum = 0; CharCodeNum < 72; CharCodeNum++) //72字节
    {
      CharCode = dzk[CharCodeNum];  //24*24字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; BitData++) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          p[XHeadPos + XPos + YPos * textrange.rangex] = colour; //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        YPos++; //下一行
      }
    }
  }
}

//在指定的区域写文字
void DisplayTextInRange(unsigned char *pstr, struct _textrange textrange, u16 *p, int size, u16 colour)
{
  //参数pstr字符串，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  int XPos = 0;

  while (*pstr != 0)
  {
    if (*pstr <= 126) // 英语
    {
      if (XPos <= (textrange.rangex - 12)) //防止溢出  //一个英文字符的宽度为12
      {
        WriteEnglishCharToBuf(textrange, pstr, XPos, p, size, colour); //将英文字符写入数组中
        XPos = XPos + 12; //一个英文字符的宽度为12
        pstr = pstr + 1; //一个字节
      }
    }
    else //汉字
    {
      if (XPos <= (textrange.rangex - 24)) //防止溢出  //一个中文字符的宽度为24
      {
        WriteChineseCharToBuf(textrange, pstr, XPos, p, size, colour); //将中文字符写入数组中
        XPos = XPos + 24; //一个中文字符的宽度为24
        pstr = pstr + 2; //两个字节
      }
    }
  }

  WriteTextDisplayRangeInfo(textrange, p); //将数组中的数据写到lcd中
}

//显示英文字符
void DisplayEnglishChar(const unsigned char *pstr, int XHeadPos, int YHeadPos, int size, u16 colour)
{
  //参数*pstr是字符，参数XHeadPos、YHeadPos是显示的起始位置，参数size是字体大小，colour是字体颜色
  int CharCodeNum, BitData, YPos = 0, XPos = -1;
  unsigned char CharCode;
  unsigned char CharPos = *pstr - ' ';  //偏移量

  if (size == 24) //24*12字体
  {
    for (CharCodeNum = 0; CharCodeNum < 36; CharCodeNum++) //36个字节
    {
      CharCode = asc2_2412[CharPos][CharCodeNum]; //24*12字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; BitData++) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          LCD_Fast_DrawPoint((u16)(XHeadPos + XPos), (u16)(YHeadPos + YPos), colour); //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        YPos++; //下一行
      }
    }
  }
}

//显示中文字符
void DisplayChineseChar(const unsigned char *pstr, int XHeadPos, int YHeadPos, int size, u16 colour)
{
  //参数*pstr是中文字符，参数XHeadPos、YHeadPos是显示的起始位置，参数size是字体大小，colour是字体颜色
  int CharCodeNum, BitData, YPos = 0, XPos = -1;
  unsigned char CharCode;
  unsigned char dzk[72]; //一个汉字的点阵数据72字节Buf

  if (size == 24)
  {
    Get_HzMat(pstr, dzk, (u8)size); //得到72字节的点阵数据

    for (CharCodeNum = 0; CharCodeNum < 72; CharCodeNum++) //72字节
    {
      CharCode = dzk[CharCodeNum];  //24*24字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; BitData++) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          LCD_Fast_DrawPoint((u16)(XHeadPos + XPos), (u16)(YHeadPos + YPos), colour); //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        YPos++; //下一行
      }
    }
  }
}

void DisplayText(const unsigned char *pstr, int x, int y, int size, u16 colour)
{
  if (t_sys.lcd_ssd1963_43_480_272 && lcddev.height == 272)
  {
    X_1963(x);
    x += 1;
    Y_1963(y);
  }

  //参数pstr字符串，参数xy显示区域，参数size字体大小
  while (*pstr != 0)
  {
    if (*pstr <= 126) //英语
    {
      DisplayEnglishChar(pstr, x, y, size, colour); //英文字符
      x += size / 2;
      pstr++;
    }
    else // 汉字
    {
      DisplayChineseChar(pstr, x, y, size, colour); //中文字符
      x += size;
      pstr += 2;
    }
  }
}

void DisplayTextNormal(const unsigned char *pstr, int x, int y, int size, u16 colour)
{
  //参数pstr字符串，参数xy显示区域，参数size字体大小
  while (*pstr != 0)
  {
    if (*pstr <= 126) //英语
    {
      DisplayEnglishChar(pstr, x, y, size, colour); //英文字符
      x += size / 2;
      pstr++;
    }
    else // 汉字
    {
      DisplayChineseChar(pstr, x, y, size, colour); //中文字符
      x += size;
      pstr += 2;
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif











