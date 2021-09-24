#include "globalvariables.h"
#include "user_common.h"

//SD卡信息结构
uint8_t IsRootDir;                          //当前目录是否是根目录
int CurrentPage;                        //当前GUI要显示的页面-当前目录下的文件分成多个GUI页面
uint8_t IsHaveNextPage;                     //是否有下一页
uint8_t IsHaveFile[OnePageNum];             //是否有需要显示的文件名
uint8_t IsDir[OnePageNum];                  //当前GUI显示的文件是否是目录文件
char CurrentPath[100];                  //当前目录的路径
char DisplayFileName[OnePageNum][100];  //当前GUI要显示的文件名


//状态信息结构
uint8_t IsWarning;  //错误警告
int WarningInfoSelect; //警告信息选择
//int IsGUIReFresh;  //是否GUI刷新
uint8_t IsFinishedPowerOffRecoverReady; //是否完成了断电续打的准备
uint8_t IsFinishedCalculateZPosLimit;  //是否完成了Z轴的行程测量
uint8_t IsComputerControlToPausePrint; //电脑端控制暂停打印
uint8_t IsComputerControlToResumePrint; //电脑端控制继续打印
uint8_t IsComputerControlToStopPrint; //电脑端控制停止打印
uint8_t IsFinishedFilamentHeat;  //是否完成加热-进丝、退丝
uint8_t IsSuccessFilament;  //是否完成进丝、退丝
//int IsPowerOffRecoverFileInSD; //断电续打文件是否在SD卡中

uint8_t IsTransFile;  //是否在上传文件
uint8_t ChangeFilamentHeatStatus; //是否加热完成
uint8_t M600FilamentChangeStatus; //打印中途换料状态
uint8_t IsDoorOpen; //M14R03,M14S检测到门是否打开
uint8_t IsDisplayDoorOpenInfo; //是否显示门打开的提示信息
uint8_t IsFinishMatCheckCalibrate; //是否完成了断料校准
uint8_t IsNotHaveMatInPrint; //在打印的时候是否没料了
char SDFileName[50];  //电脑上传的文件名

unsigned int isOpenBeep = 0;
unsigned int doorStatus = 0;

unsigned int task_schedule_delay_time = 50;

//// 结构体初始化
T_CUSTOM_SERVICES t_custom_services;
T_GUI t_gui;
T_POWER_OFF t_power_off;

#ifdef CAL_Z_ZERO_OFFSET
  bool isCalZero = FALSE;
#endif

uint32_t poweroff_data_size = 0;                   // 斷電續打數據緩存大小


unsigned char is_serial_full = 0;
t_gui_respond_data gui_respond_data;



