#include "commonf.h"
#include "globalvariables.h"
#include "common.h"
#include  "interface.h"

#include "user_common.h"
#include "jshdisplay.h"
#include "sysconfig_data.h"
#define MAX_FILE_COUNT 150
#define SGNF (((((('S'<<8)|'G')<<8)|'N')<<8)|'F')

#ifdef __cplusplus
extern "C" {
#endif


void extractBMP(char *filePathName)
{
  uint32_t RCount;
  MaseHeader header;
  FilesMsg msg;
  FIL *file;
  FIL *file1;
  FRESULT f_res;

  if (strstr(filePathName, ".sgcode") == NULL)
    return;

  file = (FIL *)malloc(sizeof(FIL));
  file1 = (FIL *)malloc(sizeof(FIL));
  USER_EchoLogStr("filePathName=   %s\r\n", filePathName);
  f_open(file, filePathName, FA_READ);

  if (f_size(file) < 100)
  {
    USER_EchoLogStr("File error!!");
  }
  else
  {
    // ��ȡheader
    f_read(file, &header, sizeof(MaseHeader), &RCount);
    // ��ȡFileMsg
    f_lseek(file, sizeof(MaseHeader) + 0 * sizeof(FilesMsg));
    f_read(file, &msg, sizeof(FilesMsg), &RCount);
    //��fileƫ�Ƶ�bmp�ļ����ݿ�ʼ��
    f_lseek(file, msg.uFileOfs);
    //�������ļ�
    f_res = f_open(file1, BMP_PATH, FA_WRITE | FA_CREATE_ALWAYS);
    USER_EchoLogStr("Create file1= %d\n", f_res);

    if (f_res == FR_NOT_ENOUGH_CORE)
    {
      user_mount_Udisk();
      f_close(file1);
      f_close(file);
      free(file);
      free(file1);
      file = NULL;
      file1 = NULL;
      return;
    }

    for (sys_data_size = 512; msg.uFileSize > sys_data_size; msg.uFileSize -= 512)
    {
      f_read(file, sys_data, sys_data_size, &RCount);
      f_lseek(file, msg.uFileOfs += 512);
      f_write(file1, sys_data, sys_data_size, &RCount);
    }

    f_read(file, sys_data, msg.uFileSize, &RCount);
    f_lseek(file, msg.uFileOfs += msg.uFileSize);
    f_write(file1, sys_data, msg.uFileSize, &RCount);
    f_close(file1);
    USER_EchoLogStr("msg.szFileName=  %s\n", msg.szFileName);
    USER_EchoLogStr("msg.uFileOfs=    %d\n", msg.uFileOfs);
    USER_EchoLogStr("msg.uFileSize=   %d\n", msg.uFileSize);
    //�ָ�sys_data�������ݣ��޸�������������sysconfig.txt�ļ����ı������2017.10.18
    f_open(file1, (char *)"0:/SysConfig.txt", FA_OPEN_EXISTING | FA_READ); //���ļ�
    memset(sys_data, 0, sizeof(sys_data));
    f_read(file1, sys_data, sizeof(sys_data), &RCount);
    f_close(file1);
  }

  f_close(file);
  free(file);
  free(file1);
  file = NULL;
  file1 = NULL;
}

#define CONFIRM 1 //ȷ������ֵ
#define CANCEL  2 //ȡ������ֵ
void printconfirmF(void)
{
  char *printnameb;   //�ļ�������
  uint32_t keybuf = 0;//����ֵ����
  static uint32_t sgcodenamelen = 0;//sgcode��ʾ���ַ�������ͨgcode��12�������Ե�ѡ��sgcode�ļ�ʱ���ñ�����ֵ12
  int length;//�����ʾ�ļ������ֽ�����Ϊ����������ʾ����ʾ�����м�

  /**************************����ˢ��****************************/
  if (gui::is_refresh())
  {
    sgcodenamelen = 0;
    printnameb = (char *)malloc(_MAX_LFN);

    if (IsRootDir)
      (void)snprintf(printnameb, _MAX_LFN, "%s", CurrentPath);
    else
      (void)snprintf(printnameb, _MAX_LFN, "%s/", CurrentPath);

    (void)strcat(printnameb, printname);
		
    /*ȷ��ѡ��Ĵ�ӡ�ļ��󣬽��ļ�����·�������浽flash���Ա�ϵ�����ʱ����*/
    strcpy(flash_param_t.pathFileName, printnameb);
    strcpy(flash_param_t.fileName, printname);
		flash_param_t.idex_print_type = t_sys.idex_print_type;
    flash_param_t.flag = 1;

    //    char printnameb[_MAX_LFN];

    if (strstr(printname, ".sgcode"))
    {
      display_picture(98);
      extractBMP(printnameb);
      sgcodenamelen = 12;
    }
    else
      display_picture(6);

    strcpy(printnameb, printname);

    if (strlen(printnameb) > MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen)
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 3] = '.';
    }

    length = (int)strlen(printnameb);
    DisplayText((uint8_t *)printnameb, (t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - (length / 2) * 12 - (sgcodenamelen * 10), 105, 24, (u16)testcolor); //240-(length/2)*12    ��Ϊ����������ʾ���м�

    if (sgcodenamelen)
      diplayBMP(t_sys.lcd_ssd1963_43_480_272 ? 24 : 44);

    free(printnameb);
    printnameb = NULL;
  }

  /*************************�������***********************************/
  if (sgcodenamelen)
  {
    if (touchxy(0, 240, 135, 285))
      keybuf = CONFIRM;

    if (touchxy(136, 240, 260, 285))
    {
      USER_EchoLogStr("f_unlink = %d\n", f_unlink(BMP_PATH));
      keybuf = CANCEL;
    }
  }
  else
  {
    if (touchxy(136, 200, 240, 285)) keybuf = CONFIRM;

    if (touchxy(240, 200, 425, 285)) keybuf = CANCEL;
  }

  /************************����ֵ����ִ��*****************************/
  if (keybuf == CONFIRM)
  {
    if (IsDoorOpen)
    {
      gui::set_current_display(DoorOpenWarning_StartPrint);
    }
    else
    {
      print_flage = 1;
      strcpy(SettingInfoToSYS.PrintFileName, printname);
      respond_gui_send_sem(FilePrintValue);
      gui::set_current_display(maindisplayF);
    }

    return ;
  }

  if (keybuf == CANCEL)
  {
    gui::set_current_display(filescanF);
    return ;
  }

  if (gui::is_refresh_rtc())
  {
    if (!user_usb_host_is_mount())
    {
      USER_EchoLogStr("f_unlink = %d\n", f_unlink(BMP_PATH));
      gui::set_current_display(maindisplayF);
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

