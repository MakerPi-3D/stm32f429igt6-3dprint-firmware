#include "globalvariables.h"
#include "gcode.h"
#include "user_ccm.h"
#include "user_common.h"
#include "usbd_cdc_if.h"
#include "interface.h"
#include "PrintControl.h"
#include "process_command.h"
#include "temperature.h"
#include "sysconfig_data.h"
#include "ref_data_task.h"
#ifdef __cplusplus
extern "C" {
#endif
unsigned long TransFileTimeOut; //传输是否超时
uint32_t byteswritten;
FIL MyFile;

//////////////////////////////上传文件--OS任务中的处理//////////////////////////////////

static volatile bool IsUSBPrintFinished = false; //联机打印是否完成
static bool IsUSBPrintStop = false; //打印中途是否停止了打印
static bool IsUSBPrintPause = false; //打印中途是否暂停了打印
void SetIsUSBPrintFinished(bool Value)
{
  IsUSBPrintFinished = Value;
}

void SetIsUSBPrintStop(bool Value)
{
  IsUSBPrintStop = Value;
}

void SetIsUSBPrintPause(bool Value)
{
  IsUSBPrintPause = Value;
}

void TransFileReady(void);

void process_cmd(void)
{
  if (transFileStatus.IsM35Order) //M35命令，获取状态命令
  {
    transFileStatus.IsM35Order = 0; //重置标志

    if (IsWarning) user_usb_device_transmit("SysErr"); //液晶屏正在显示警告界面或错误界面
    else if (IsPausePrint()) user_usb_device_transmit("Pausing"); //正在暂停状态
    else if (IsPrint()) user_usb_device_transmit("Printing"); //正在打印状态
    else user_usb_device_transmit("NotPrinting"); //没有在打印
  }
  else if (transFileStatus.IsM34Order) //M34命令，机型确认命令
  {
    transFileStatus.IsM34Order = 0; //重置标志
    //对比机型

    if (0 == strcmp(t_sys.model_str, (char *)transFileStatus.ModelStr)) user_usb_device_transmit("Err:ModelNotRight"); //机型不正确，不上传文件, 返回Err给电脑端
    else user_usb_device_transmit("ok"); //返回ok给电脑端
  }
  else if (transFileStatus.IsM28Order) //M28命令，开始文件传输
  {
    transFileStatus.IsM28Order = 0; //重置标志

    //准备去传输文件
    if (IsPrint()) //正在打印，不上传文件
    {
      user_usb_device_transmit("no"); //返回no给电脑端
    }
    else
    {
      IsTransFile = 1; //置位GUI上传文件标志
      TransFileReady();  //传输相关的一些设置
      (void)OS_DELAY(200);  //延时以让GUI任务切换相应界面

      if (transFileStatus.IsTaskCritical)
      {
        taskENTER_CRITICAL();
      }

      f_unlink((char *)transFileStatus.FileNameStr);

      if (f_open(&MyFile, (char *)transFileStatus.FileNameStr, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) //创建文件
      {
        f_close(&MyFile);
      }

      if (transFileStatus.IsTaskCritical)
      {
        taskEXIT_CRITICAL();
      }

      user_usb_device_transmit("ok"); //返回ok给电脑端
    }
  }
  else if (transFileStatus.IsM105Order) //M105命令，返回打印数据
  {
    transFileStatus.IsM105Order = 0; //重置标志
    //返回温度数据
    int NozzleTemp;
    int HotbedTemp;
    int Percent;
    char BackDataBuf[64];
    NozzleTemp = (int)temperature_get_extruder_current(0); //读取喷嘴温度
    HotbedTemp = (int)temperature_get_bed_current(); //读取热床温度
    Percent = t_gui.print_percent; //读取打印进度

    if (IsPrint())
    {
      (void)snprintf(BackDataBuf, sizeof(BackDataBuf), "S:Printing T:%d B:%d P:%d", NozzleTemp, HotbedTemp, Percent); //合成字符串
      (void)CDC_Transmit_HS((uint8_t *)BackDataBuf, strlen(BackDataBuf)); //返回温度数据
    }
    else
    {
      (void)snprintf(BackDataBuf, sizeof(BackDataBuf), "S:NotPrinting T:%d B:%d", NozzleTemp, HotbedTemp); //合成字符串
      (void)CDC_Transmit_HS((uint8_t *)BackDataBuf, strlen(BackDataBuf)); //返回温度数据
    }
  }
  else if (transFileStatus.IsG1Order) //G1控制电机命令，控制电机移动
  {
    transFileStatus.IsG1Order = 0; //重置标志
    //将接受到的G1命令传给系统处理

    if (IsPrint())
    {
      user_usb_device_transmit("Printing"); //正在打印，不执行操作，返回对应信息
    }
    else
    {
      user_usb_device_transmit("ok"); //没有打印，去执行命令

      if (!gcode::g28_complete_flag)
        user_send_internal_cmd((char *)"G28"); //没有归零过，先归零

      user_send_internal_cmd((char *)transFileStatus.G1OrderBuf); //用消息队列将Gcode命令传输到Gcode处理任务去处理
    }
  }
  else if (transFileStatus.IsG28Order) //G28归零命令
  {
    transFileStatus.IsG28Order = 0; //重置标志
    //将接受到的G28命令传给系统处理

    if (IsPrint())
    {
      user_usb_device_transmit("Printing"); //正在打印，不执行操作，返回对应信息
    }
    else
    {
      user_usb_device_transmit("ok"); //没有打印，去执行命令
      user_send_internal_cmd((char *)"G28"); //用消息队列将Gcode命令传输到Gcode处理任务去处理
    }
  }
  else if (transFileStatus.IsM25Order) //M25暂停打印命令
  {
    transFileStatus.IsM25Order = 0; //重置标志
    //将接受到的M25命令传给系统处理

    if (IsPrint())
    {
      user_usb_device_transmit("ok"); //正在打印，去执行命令
      //暂停打印命令相当于在GUI界面点击了暂停打印按钮
      IsComputerControlToPausePrint = 1;
    }
    else
    {
      user_usb_device_transmit("NotPrinting"); //没有在打印状态，不执行操作，返回对应信息
    }
  }
  else if (transFileStatus.IsM24Order) //M24继续打印命令
  {
    transFileStatus.IsM24Order = 0; //重置标志
    //将接受到的M24命令传给系统处理

    if (IsPausePrint())
    {
      user_usb_device_transmit("ok"); //正在暂停状态，去执行命令
      //继续打印命令相当于在GUI界面点击了继续打印按钮
      IsComputerControlToResumePrint = 1;
    }
    else
    {
      user_usb_device_transmit("NotPausing"); //不是在暂停状态，不执行操作，返回对应信息
    }
  }
  else if (transFileStatus.IsM33Order) //M33停止打印命令
  {
    transFileStatus.IsM33Order = 0; //重置标志
    //将接受到的M33命令传给系统处理

    if (IsPrint() || IsPausePrint()) //在打印状态或在暂停状态
    {
      user_usb_device_transmit("ok"); //正在打印或在暂停状态，去执行命令
      //停止打印命令相当于在GUI界面点击了停止打印按钮
      IsComputerControlToStopPrint = 1;
    }
    else
    {
      user_usb_device_transmit("NotPrinting"); //没在打印或在暂停状态，不执行操作，返回对应信息
    }
  }
  else if (transFileStatus.IsM700Order) //M33停止打印命令
  {
    transFileStatus.IsM700Order = 0; //重置标志

    //将接受到的M700命令传给系统处理
    if (IsUSBPrintFinished) user_usb_device_transmit("PrintFinished"); //已完成打印
    else if (transFilePrintStatus.isFilamentCheck) user_usb_device_transmit("NoMaterialToPrint"); //暂停了打印
    else if (IsUSBPrintPause) user_usb_device_transmit("PrintPause"); //暂停了打印
    else if (IsUSBPrintStop) user_usb_device_transmit("PrintStop"); //停止了打印
    else user_usb_device_transmit("NotPrintFinished"); //正在打印中
  }
}

void SaveUSBFile(void)
{
  TransFileTimeOut = xTaskGetTickCount() + 5000; //传输超时时间设置

  if (1 == transFileStatus.IsEndTrans) //传输是否结束
  {
    OS_DELAY(300);
    user_usb_device_write_file_buf();
    USER_EchoLog("Virtual serial port ==>> Tran file done!\n");
    IsTransFile = 0;
    //让GUI跳转界面并开始打印
    (void)strcpy(SettingInfoToSYS.PrintFileName, (char *)&transFileStatus.FileNameStr[3]);
    (void)strcpy(SDFileName, (char *)&transFileStatus.FileNameStr[3]); //复制文件名给GUI
    printFileControl.setSDMedium();//设置从SD卡读取文件
    transFileStatus.IsEndTrans = 0;
    OS_DELAY(50);
    user_usb_device_receive_prepare_cmd();
  }
  else if (1 == IsTransFile) //写文件内容到SD
  {
    OS_DELAY(300);
    user_usb_device_write_file_buf(); //将接收到的文件内容保存到SD卡中
    OS_DELAY(50);
    user_usb_device_receive_prepare_file();
  }
  else if (1 == transFileStatus.IsCmd)
  {
    process_cmd();
    transFileStatus.IsCmd = 0;
    OS_DELAY(50);
    user_usb_device_receive_prepare_cmd();
  }
}



#ifdef __cplusplus
} // extern "C" {
#endif

