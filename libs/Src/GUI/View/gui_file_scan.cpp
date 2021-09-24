#include "machinecustom.h"
#include "commonf.h"
#include  "common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "icons.h"
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif

char printname[_MAX_LFN];            //��ӡ�ļ�������
static int  catalogi = 0;      //�ļ�����

void drawup(u16 x, u16 y)        //����һҳ��ť
{
  if (lcddev.id == 0x7016)
  {
    x *= 2.13;
    y *= 1.875;
  }

  POINT_COLOR = (u16)testcolor;
  LCD_DrawLine(x - 24, y, x, y - 24);
  LCD_DrawLine(x, y - 24, x + 24, y);
  LCD_DrawLine(x - 24, y, x - 12, y);
  LCD_DrawLine(x + 12, y, x + 24, y);
  LCD_DrawLine(x - 12, y + 24, x - 12, y);
  LCD_DrawLine(x + 12, y + 24, x + 12, y);
  LCD_DrawLine(x - 12, y + 24, x + 12, y + 24);
}

void drawdown(u16 x, u16 y)      //����һҳ��ť��x yΪ���ĵ�
{
  if (lcddev.id == 0x7016)
  {
    x *= 2.13;
    y *= 1.875;
  }

  POINT_COLOR = (u16)testcolor;
  LCD_DrawLine(x - 24, y, x, y + 24);
  LCD_DrawLine(x, y + 24, x + 24, y);
  LCD_DrawLine(x - 24, y, x - 12, y);
  LCD_DrawLine(x + 12, y, x + 24, y);
  LCD_DrawLine(x - 12, y - 24, x - 12, y);
  LCD_DrawLine(x + 12, y - 24, x + 12, y);
  LCD_DrawLine(x - 12, y - 24, x + 12, y - 24);
}

void DrawChangePageArrow(void)
{
  if (CurrentPage > 1)  //�ж��Ƿ�����һҳ
  {
    drawup(415, 120);
  }

  if (IsHaveNextPage)   //�ж��Ƿ�����һҳ
  {
    drawdown(415, 225);
  }
}


void DisplayFileItem(void)
{
  int i;
  int ii;
  char printnameb[_MAX_LFN];
  catalogi = 0;

  for (i = 0; i < OnePageNum ; i++)
  {
    if (IsHaveFile[catalogi])
    {
      ++catalogi;                   //��ѯ��ǰҳ����ļ�
    }
  }

  for (ii = 0; ii < catalogi; ii++)
  {
    //��ʾ�ļ�����
    strcpy(printnameb, DisplayFileName[ii]);

    if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
    }

    if (IsDir[ii])
    {
      if (lcddev.height == 600)
        LCD_Color_Fill(80 * 2.13, (u16)(97 * 1.875 + ii * (24 + 16)) * 0.85, 80 * 2.13 + 23, (u16)(97 * 1.875 + ii * (24 + 16) + 23) * 0.85, (u16 *)DirIcon);
      else if (lcddev.height == 272)
        LCD_Color_Fill(80, (u16)(97 + ii * (24 + 16)) * 0.85, 80 + 23, (u16)(97 + ii * (24 + 16) + 23) * 0.85, (u16 *)DirIcon);
      else
        LCD_Color_Fill(80, (u16)(97 + ii * (24 + 16)), 80 + 23, (u16)(97 + ii * (24 + 16) + 23), (u16 *)DirIcon);
    }
    else
    {
      if (lcddev.height == 600)
        LCD_Color_Fill(80 * 2.13, (u16)(97 * 1.875 + ii * (24 + 16)) * 0.85, 80 * 2.13 + 23, (u16)(97 * 1.875 + ii * (24 + 16) + 23) * 0.85, (u16 *)FileIcon);
      else if (lcddev.height == 272)
        LCD_Color_Fill(80, (u16)(97 + ii * (24 + 16)) * 0.85, 80 + 23, (u16)(97 + ii * (24 + 16) + 23) * 0.85, (u16 *)FileIcon);
      else
        LCD_Color_Fill(80, (u16)(97 + ii * (24 + 16)), 80 + 23, (u16)(97 + ii * (24 + 16) + 23), (u16 *)FileIcon);
    }

    if (lcddev.height == 600)
      DisplayText((uint8_t *)printnameb, 55 * 2.13 + 24, 97 * 1.875 + ii * (24 + 16), 24, (u16)testcolor);
    else
      DisplayText((uint8_t *)printnameb, 55 + 24, 97 + ii * (24 + 16), 24, (u16)testcolor);
  }
}


void DisplayCurrentPage(void)
{
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%2d", CurrentPage); //�����ӡ��ǰҳ��

  if (CurrentPage < 10)
  {
    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 193, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
    else if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 231, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id) //Ӣ��ͼƬ
      DisplayText((uint8_t *)buffer, 231, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
    else if (PICTURE_IS_KOREA == t_sys_data_current.pic_id) //�n��ͼƬ
      DisplayText((uint8_t *)buffer, 231, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
    else if (PICTURE_IS_RUSSIA == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 231, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 193, 262, 24, (u16)testcolor); //��ʾ��ǰҳ�루С��10��
  }
  else
  {
    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 198, 262, 24, (u16)testcolor);
    else if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 237, 262, 24, (u16)testcolor);
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id) //Ӣ��ͼƬ
      DisplayText((uint8_t *)buffer, 237, 262, 24, (u16)testcolor);
    else if (PICTURE_IS_KOREA == t_sys_data_current.pic_id) //�n��ͼƬ
      DisplayText((uint8_t *)buffer, 237, 262, 24, (u16)testcolor);
    else if (PICTURE_IS_RUSSIA == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 237, 262, 24, (u16)testcolor);
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id) //����ͼƬ
      DisplayText((uint8_t *)buffer, 198, 262, 24, (u16)testcolor);
  }
}

uint8_t ClickBackButton(void)
{
  if (touchxy(0, 0, 150, 65))     //���ذ�ť
  {
    if (IsRootDir)    //���Ƿ�Ϊ��Ŀ¼
    {
      gui::set_current_display(maindisplayF);
      return 1;
    }
    else                      //���Ǹ�Ŀ¼�������ϼ�Ŀ¼
    {
      respond_gui_send_sem(BackLastDirValue);
      gui::set_current_display(filescanF);
      return 1;
    }
  }

  return 0;
}

uint8_t ClickFileItem(void)
{
  int ii;

  if (touchxy(55, 97, 371, 252)) //�ж��Ƿ�ѡ����Ŀ
  {
    for (ii = 0; ii < catalogi; ii ++)
    {
      if (touchrange(55, (97 + ii * 40), 315, 40))   //��һ���ж�
      {
        if (IsDir[ii])       //���Ƿ�Ϊ�ļ���
        {
          //���ļ��У����»�����ݣ����ļ���
          strcpy(SettingInfoToSYS.DirName, DisplayFileName[ii]);
          respond_gui_send_sem(OpenDirValue);
          gui::set_current_display(filescanF);
          return 1;
        }
        else    //�����ļ��У���ת���ļ���ӡ����
        {
          strcpy(printname, DisplayFileName[ii]);

          if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL && t_sys.is_idex_extruder == 1)
          {
            gui::set_current_display(gui_p3_pro_idex_model_select);
          }
          else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
          {
            respond_gui_send_sem(SwitchLaser);
            gui::set_current_display(printconfirmF);
          }
          else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
          {
            gui::set_current_display(gui_p3_pro_mix_model_select);
          }
          else
          {
            gui::set_current_display(printconfirmF);
          }

          return 1;
        }
      }
    }

    return 0;
  }

  return 0;
}

uint8_t ChangePage(void)
{
  if (CurrentPage > 1) //���Ƿ�����һҳ����ť����Ч
  {
    if (touchxy(395, 90, 480, 160))
    {
      respond_gui_send_sem(LastPageValue);
      gui::set_current_display(filescanF);
      return 1;
    }
  }

  if (IsHaveNextPage)  //���Ƿ�����һҳ����ť����Ч
  {
    if (touchxy(395, 160, 480, 250))
    {
      respond_gui_send_sem(NextPageValue);
      gui::set_current_display(filescanF);
      return 1;
    }
  }

  return 0;
}

void filescanF(void)
{
  if (gui::is_refresh())
  {
    display_picture(5);
    osDelay(100); //�ȴ�ɨ���ļ����
    DrawChangePageArrow(); //��ʾ��ҳ��ͷ
    DisplayFileItem(); //��ʾ�ļ����ļ�����Ŀ
    DisplayCurrentPage(); //��ʾ��ǰҳ��
  }

  if (ClickBackButton()) //����˷��ؼ�����
    return;

  if (ClickFileItem()) //������ļ����ļ�����Ŀ����
    return;

  if (ChangePage()) //�������һҳ����һҳ����
    return;

  if (gui::is_refresh_rtc()) //SD���γ�����������
  {
    if (!user_usb_host_is_mount())
    {
      gui::set_current_display(maindisplayF);
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

