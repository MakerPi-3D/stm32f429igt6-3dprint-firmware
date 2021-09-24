#include "touch.h"
#include "user_common.h"
#include "math.h"
#include "guidata.h"
#include "controlfunction.h"
#include "ft5206.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"
#include "user_board_pin.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//������������֧��ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A�ȣ� ����
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/7
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//V1.1 20140721
//����MDK��-O2�Ż�ʱ,�����������޷���ȡ��bug.��TP_Write_Byte�������һ����ʱ,�������.
//////////////////////////////////////////////////////////////////////////////////

volatile uint8_t is_touch_capacitive = 0;

extern void display_picture(int PictureName);

static void tp_pin_init(void)
{
  if (FT5206_Init() == 1)
  {
    is_touch_capacitive = 1;
    tp_dev.scan = FT5206_Scan;  //ɨ�躯��ָ��GT9147������ɨ��
    tp_dev.touchtype |= 0X80;   //������
    tp_dev.touchtype |= lcddev.dir & 0X01; //������������
  }
  else
  {
    is_touch_capacitive = 0;
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    /*
    PH7     ------> T_PEN
    PG3     ------> T_MISO
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    /*
    PI8     ------> T_CS
    PI3     ------> T_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    /*
    PH6     ------> T_SCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
  }
}

_m_tp_dev tp_dev =
{
  TP_Init,
  TP_Scan,
  TP_Adjust,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};
//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX = 0X90;
u8 CMD_RDY = 0XD0;

//SPIд����
//������ICд��1byte����
//num:Ҫд�������
void TP_Write_Byte(u8 num)
{
  u8 count;

  for (count = 0; count < 8; count++)
  {
    if (num & 0x80)
    {
      user_pin_tp_mosi_ctrl(true);
    }
    else
    {
      user_pin_tp_mosi_ctrl(false);
    }

    num <<= 1;
    user_pin_tp_sck_ctrl(false);
    user_delay_us(1);
    user_pin_tp_sck_ctrl(true);  //��������Ч
  }
}
//SPI������
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������
u16 TP_Read_AD(u8 CMD)
{
  u8 count = 0;
  u16 Num = 0;
  user_pin_tp_sck_ctrl(false);    //������ʱ��
  user_pin_tp_mosi_ctrl(false);   //����������
  user_pin_tp_cs_ctrl(false);     //ѡ�д�����IC
  TP_Write_Byte(CMD);//����������
  user_delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
  user_pin_tp_sck_ctrl(false);
  user_delay_us(1);
  user_pin_tp_sck_ctrl(true);    //��1��ʱ�ӣ����BUSY
  user_delay_us(1);
  user_pin_tp_sck_ctrl(false);

  for (count = 0; count < 16; count++) //����16λ����,ֻ�и�12λ��Ч
  {
    Num <<= 1;
    user_pin_tp_sck_ctrl(false);  //�½�����Ч
    user_delay_us(1);
    user_pin_tp_sck_ctrl(true);

    if (user_pin_tp_miso_read())Num++;
  }

  Num >>= 4;  //ֻ�и�12λ��Ч.
  user_pin_tp_cs_ctrl(true);   //�ͷ�Ƭѡ
  return (Num);
}
//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5  //��ȡ����
#define LOST_VAL 1      //����ֵ
u16 TP_Read_XOY(u8 xy)
{
  u16 i, j;
  u16 buf[READ_TIMES];
  u16 sum;
  u16 temp;

  for (i = 0; i < READ_TIMES; i++)buf[i] = TP_Read_AD(xy);

  for (i = 0; i < READ_TIMES - 1; i++) //����
  {
    for (j = i; j < READ_TIMES; j++)
    {
      if (buf[i] > buf[j]) //��������
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  sum = 0;

  for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)sum += buf[i];

  temp = sum / (READ_TIMES - 2 * LOST_VAL);
  return temp;
}
//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
u8 TP_Read_XY(u16 *x, u16 *y)
{
  u16 xtemp, ytemp;
  xtemp = TP_Read_XOY(CMD_RDX);
  ytemp = TP_Read_XOY(CMD_RDY);
  //if(xtemp<100||ytemp<100)return 0;//����ʧ��
  *x = xtemp;
  *y = ytemp;
  return 1;//�����ɹ�
}
//����2�ζ�ȡ������IC,�������ε�ƫ��ܳ���
//ERR_RANGE,��������,����Ϊ������ȷ,�����������.
//�ú����ܴ�����׼ȷ��
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
#define ERR_RANGE 50 //��Χ 
u8 TP_Read_XY2(u16 *x, u16 *y)
{
  u16 x1, y1;
  u16 x2, y2;
  u8 flag;
  flag = TP_Read_XY(&x1, &y1);

  if (flag == 0)return (0);

  flag = TP_Read_XY(&x2, &y2);

  if (flag == 0)return (0);

  if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //ǰ�����β�����+-50��
      && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
  {
    *x = (x1 + x2) / 2;
    *y = (y1 + y2) / 2;
    return 1;
  }
  else return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//��LCD�����йصĺ���
//��һ��������
//����У׼�õ�
//x,y:����
//color:��ɫ
void TP_Drow_Touch_Point(u16 x, u16 y, u16 color)
{
  POINT_COLOR = color;
  LCD_DrawLine(x - 12, y, x + 13, y); //����
  LCD_DrawLine(x, y - 12, x, y + 13); //����
  LCD_DrawPoint(x + 1, y + 1);
  LCD_DrawPoint(x - 1, y + 1);
  LCD_DrawPoint(x + 1, y - 1);
  LCD_DrawPoint(x - 1, y - 1);
  LCD_Draw_Circle(x, y, 6); //������Ȧ
}
//��һ�����(2*2�ĵ�)
//x,y:����
//color:��ɫ
void TP_Draw_Big_Point(u16 x, u16 y, u16 color)
{
  POINT_COLOR = color;
  LCD_DrawPoint(x, y); //���ĵ�
  LCD_DrawPoint(x + 1, y);
  LCD_DrawPoint(x, y + 1);
  LCD_DrawPoint(x + 1, y + 1);
}
//////////////////////////////////////////////////////////////////////////////////

#ifdef LCD_DEBUG //¬������LCDҪ��
  extern uint32_t Lcd_Count_touch;
#endif
//��������ɨ��
//tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 TP_Scan(u8 tp)
{
  taskENTER_CRITICAL();

  if (user_pin_tp_pen_read()) //�а�������
  {
    if (tp)(void)TP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0]); //��ȡ��������

    if (TP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0])) //��ȡ��Ļ����
    {
      tp_dev.x[1] = (unsigned short)(tp_dev.xfac * tp_dev.x[0] + tp_dev.xoff); //�����ת��Ϊ��Ļ����
      tp_dev.y[1] = (unsigned short)(tp_dev.yfac * tp_dev.y[0] + tp_dev.yoff);
    }

    if ((tp_dev.sta & TP_PRES_DOWN) == 0) //֮ǰû�б�����
    {
      tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES; //��������
      //tp_dev.x[4]=tp_dev.x[0];//��¼��һ�ΰ���ʱ������
      //tp_dev.y[4]=tp_dev.y[0];
    }

    #ifdef LCD_DEBUG
    //    if(Lcd_Count_touch[0]>=65535) Lcd_Count_touch[1]+=1;
    Lcd_Count_touch++;
    #endif
  }
  else
  {
    if (tp_dev.sta & TP_PRES_DOWN) //֮ǰ�Ǳ����µ�
    {
      tp_dev.sta &= ~(1 << 7); //��ǰ����ɿ�
    }
    else //֮ǰ��û�б�����
    {
      tp_dev.x[4] = 0;
      tp_dev.y[4] = 0;
      //tp_dev.x[0]=0xffff;
      //tp_dev.y[0]=0xffff;
    }
  }

  taskEXIT_CRITICAL();
  return tp_dev.sta & TP_PRES_DOWN; //���ص�ǰ�Ĵ���״̬
}
//////////////////////////////////////////////////////////////////////////
//����У׼����
void TP_Save_Adjdata(void)
{
  float iyfac, ixfac;
  u16 ioa = 0X0A;
  u32 buffer[4];
  ReadSaveData(buffer, 4);

  /////////////////////////////////////////�޸ĸ���
  if (tp_dev.xfac < 0)
  {
    ixfac = -tp_dev.xfac;
    ioa |= (1 << 8);      //x�Ḻ����־
  }
  else
  {
    ixfac = tp_dev.xfac;
    ioa &= (~(1 << 8));   //
  }

  buffer[0] = (u32)(ixfac * 100000000);

  /////////////////////
  if (tp_dev.yfac < 0)
  {
    iyfac = -tp_dev.yfac;
    ioa |= (1 << 9);      //y�Ḻ����־
  }
  else
  {
    iyfac = tp_dev.yfac;
    ioa &= (~(1 << 9));   //
  }

  buffer[1] = (u32)(iyfac * 100000000);
  //////////////////////////////////////////////////////////////////
  buffer[2] = (int)(tp_dev.xoff & 0xffff) | (int)tp_dev.yoff << 16;
  buffer[3] = (int)tp_dev.touchtype | (int)ioa << 16;
  WriteSaveData(buffer, 4);
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{
  int tempfac;
  u32 buffer[4];
  ReadSaveData(buffer, 4);
  tempfac = buffer[3] >> 16;

  if ((tempfac & 0xff) == 0X0A) //�������Ѿ�У׼����
  {
    ///////////////////////////////////////////////////////�޸ĸ���
    if (tempfac & (1 << 8))
    {
      tp_dev.xfac = -(float)buffer[0] / 100000000; //�õ�xУ׼����
    }
    else
    {
      tp_dev.xfac = (float)buffer[0] / 100000000; //�õ�xУ׼����
    }

    //////////////////////////////////////////
    if (tempfac & (1 << 9))
    {
      tp_dev.yfac = -(float)buffer[1] / 100000000; //�õ�yУ׼����
    }
    else
    {
      tp_dev.yfac = (float)buffer[1] / 100000000; //�õ�yУ׼����
    }

    ///////////////////////////////////////////////////////////////////
    tp_dev.xoff = (short)(buffer[2] & 0xffff);
    tp_dev.yoff = (short)(buffer[2] >> 16);

    if (1 == is_touch_capacitive) // ����������
      return 1;

    tp_dev.touchtype = (u8)(buffer[3] & 0xff);

    if (tp_dev.touchtype) //X,Y��������Ļ�෴
    {
      CMD_RDX = 0XD0;
      CMD_RDY = 0X90;

      if ((lcddev.id != 0X4342) && (lcddev.id != 0X7016))
      {
        tp_dev.touchtype = 0;
        CMD_RDX = 0X90;
        CMD_RDY = 0XD0;
        return 0;
      }
    }
    else            //X,Y��������Ļ��ͬ
    {
      CMD_RDX = 0X90;
      CMD_RDY = 0XD0;

      if ((lcddev.id == 0X4342) || (F1000TP == t_sys_data_current.model_id) || (F400TP == t_sys_data_current.model_id))
      {
        tp_dev.touchtype = 1;
        CMD_RDX = 0XD0;
        CMD_RDY = 0X90;
        return 0;
      }
    }

    return 1;
  }

  return 0;
}

//������У׼����
//�õ��ĸ�У׼����
void TP_Adjust(void)
{
  u16 pos_temp[4][2];//���껺��ֵ
  u8  cnt;
  u16 d1, d2;
  //u32 tem1,tem2;
  int tem1, tem2;
  float fac;
  u16 outtime = 0;
  cnt = 0;
  POINT_COLOR = BLUE;
  BACK_COLOR = WHITE;
  POINT_COLOR = RED; //��ɫ
  POINT_COLOR = BLACK;
  //////////////////////////////////////////////////////////////////////////////
  display_picture(15);
  user_delay_ms(1000);
  TP_Drow_Touch_Point(50, 50, RED); //����1
  tp_dev.sta = 0; //���������ź�
  tp_dev.xfac = 0; //xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������

  while (1) //�������10����û�а���,���Զ��˳�
  {
    (void)tp_dev.scan(1);//ɨ����������

    if ((tp_dev.sta & 0xc0) == TP_CATH_PRES) //����������һ��(��ʱ�����ɿ���.)
    {
      outtime = 0;
      tp_dev.sta &= ~(1 << 6); //��ǰ����Ѿ����������.
      //////////////////////////////////////////////////LCD_Buzz

      ////////////////////////////////////////////////////
      if (tp_dev.x[0] != 0 && tp_dev.x[0] != 4095) //���������쳣�����»�ȡ
      {
        user_buzzer_buzz(100);
        pos_temp[cnt][0] = tp_dev.x[0];
        pos_temp[cnt][1] = tp_dev.y[0];
        cnt++;
      }

      switch (cnt)
      {
      case 1:
        TP_Drow_Touch_Point(50, 50, 0x1992);      //�����1
        TP_Drow_Touch_Point(lcddev.width - 50, 50, RED); //����2
        break;

      case 2:
        TP_Drow_Touch_Point(lcddev.width - 50, 50, 0x1992); //�����2
        TP_Drow_Touch_Point(50, lcddev.height - 50, RED); //����3
        break;

      case 3:
        TP_Drow_Touch_Point(50, lcddev.height - 50, 0x1992);  //�����3
        TP_Drow_Touch_Point(lcddev.width - 50, lcddev.height - 50, RED); //����4
        break;

      case 4:  //ȫ���ĸ����Ѿ��õ�
        //�Ա����
        tem1 = abs(pos_temp[0][0] - pos_temp[1][0]); //x1-x2
        tem2 = abs(pos_temp[0][1] - pos_temp[1][1]); //y1-y2
        tem1 *= tem1;
        tem2 *= tem2;
        d1 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�1,2�ľ���
        tem1 = abs(pos_temp[2][0] - pos_temp[3][0]); //x3-x4
        tem2 = abs(pos_temp[2][1] - pos_temp[3][1]); //y3-y4
        tem1 *= tem1;
        tem2 *= tem2;
        d2 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�3,4�ľ���
        fac = (float)d1 / d2;

        //if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
        if (fac < 0.85f || fac > 1.15f || d1 == 0 || d2 == 0) //���ϸ�
        {
          cnt = 0;
          TP_Drow_Touch_Point(lcddev.width - 50, lcddev.height - 50, 0x1992); //�����4
          ///////////////////////////////////////////////////////////////////////////////////////////
          display_picture(16);
          user_delay_ms(250);
          display_picture(15);
          TP_Drow_Touch_Point(50, 50, RED); //����1
          continue;
        }

        tem1 = abs(pos_temp[0][0] - pos_temp[2][0]); //x1-x3
        tem2 = abs(pos_temp[0][1] - pos_temp[2][1]); //y1-y3
        tem1 *= tem1;
        tem2 *= tem2;
        d1 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�1,3�ľ���
        tem1 = abs(pos_temp[1][0] - pos_temp[3][0]); //x2-x4
        tem2 = abs(pos_temp[1][1] - pos_temp[3][1]); //y2-y4
        tem1 *= tem1;
        tem2 *= tem2;
        d2 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�2,4�ľ���
        fac = (float)d1 / d2;

        //if(fac<0.95||fac>1.05)//���ϸ�
        if (fac < 0.85f || fac > 1.15f) //���ϸ�
        {
          cnt = 0;
          TP_Drow_Touch_Point(lcddev.width - 50, lcddev.height - 50, 0x1992); //�����4
          //////////////////////////////////////////////////////////////////////////
          display_picture(16);
          user_delay_ms(1000);
          display_picture(15);
          TP_Drow_Touch_Point(50, 50, RED); //����1
          continue;
        }//��ȷ��

        //�Խ������
        tem1 = abs(pos_temp[1][0] - pos_temp[2][0]); //x1-x3
        tem2 = abs(pos_temp[1][1] - pos_temp[2][1]); //y1-y3
        tem1 *= tem1;
        tem2 *= tem2;
        d1 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�1,4�ľ���
        tem1 = abs(pos_temp[0][0] - pos_temp[3][0]); //x2-x4
        tem2 = abs(pos_temp[0][1] - pos_temp[3][1]); //y2-y4
        tem1 *= tem1;
        tem2 *= tem2;
        d2 = (unsigned short)sqrt((float)(tem1 + tem2)); //�õ�2,3�ľ���
        fac = (float)d1 / d2;

        //if(fac<0.95||fac>1.05)//���ϸ�
        if (fac < 0.85f || fac > 1.15f) //���ϸ�
        {
          cnt = 0;
          TP_Drow_Touch_Point(lcddev.width - 50, lcddev.height - 50, 0x1992); //�����4
          ////////////////////////////////////////////////////////////////////////////
          display_picture(16);
          user_delay_ms(1000);
          display_picture(15);
          TP_Drow_Touch_Point(50, 50, RED); //����1
          continue;
        }//��ȷ��

        //������
        tp_dev.xfac = (float)(lcddev.width - 100) / (pos_temp[1][0] - pos_temp[0][0]); //�õ�xfac
        tp_dev.xoff = (short)(lcddev.width - tp_dev.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2; //�õ�xoff
        tp_dev.yfac = (float)(lcddev.height - 100) / (pos_temp[2][1] - pos_temp[0][1]); //�õ�yfac
        tp_dev.yoff = (short)(lcddev.height - tp_dev.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2; //�õ�yoff

        if (abs((int)tp_dev.xfac) > 2 || abs((int)tp_dev.yfac) > 2) //������Ԥ����෴��.
        {
          cnt = 0;
          TP_Drow_Touch_Point(lcddev.width - 50, lcddev.height - 50, 0x1992); //�����4
          display_picture(16);
          user_delay_ms(1000);
          display_picture(15);
          TP_Drow_Touch_Point(50, 50, RED); //����1
          continue;
        }

        POINT_COLOR = BLUE;
        ////////////////////////////////////////////////////////////////////////
        display_picture(17);
        user_delay_ms(1000);
        TP_Save_Adjdata();
        return;//У�����

      default:
        break;
      }
    }

    user_delay_ms(10);
    outtime++;

    if (outtime > 60000) //ʮ���Ӻ��Զ��˳�
    {
      (void)TP_Get_Adjdata();
      break;
    }
  }
}
//��������ʼ��
//����ֵ:0,û�н���У׼
//       1,���й�У׼
u8 TP_Init(void)
{
  tp_pin_init();
  (void)TP_Read_XY(&tp_dev.x[0], &tp_dev.y[0]); //��һ�ζ�ȡ��ʼ��

  if (TP_Get_Adjdata())
    return 0;//�Ѿ�У׼
  else         //δУ׼?
  {
    user_pin_lcd_backlight_ctrl(true);
    //LCD_LED=1; //��������
    TP_Adjust();    //��ĻУ׼
    TP_Save_Adjdata();
  }

  (void)TP_Get_Adjdata();
  return 1;
}


