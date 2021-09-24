#include "machinecustom.h"
#include "machine_model.h"
#include "sysconfig.h"
#include "globalvariables.h"
#include "user_common.h"
#include "ConfigurationStore.h"
#include "config_model_tables.h"
#include "temperature.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_ccm.h"

static SysConfigOperation sysConfigOperation;

#ifdef __cplusplus
extern "C" {
#endif

void sysconfig_init(void)
{
  SysConfig sysConfig;
  sysConfig.init();
}

void SaveSelectedModel(void) //保存已选择的机型
{
  sysConfigOperation.ChangeModelInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveSelectedPicture(void) //保存已选择的图片
{
  sysConfigOperation.ChangePictureInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveCustomModelId(void) //保存已选择的图片
{
  sysConfigOperation.ChangeCustomModelId();
  sysConfigOperation.saveInfo(true);
}

void SaveSelectedFunction(void) //保存已选择的功能
{
  sysConfigOperation.ChangeFunctionInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveCalculateZMaxPos(float ZMaxPosValue) //保存测量的Z轴行程
{
  if (t_sys_data_current.enable_powerOff_recovery)
  {
    // 判断校准值，如果大于等于最大Z位置减去CAL_Z_MAX_POS_OFFSET，更新校准值
    // 这里主要更新Z最大点和移动最大点
    // 自动调平高度可能小于默认最大高度，忽略修改
    if ((ccm_param::motion_3d_model.z_max_pos_origin - CAL_Z_MAX_POS_OFFSET) <= ZMaxPosValue //2017.10.8除去+8;+8是防止校准值小于默认值太多，引起最大行程警报，2017.7.14john
        || 1 == t_sys_data_current.enable_bed_level)
    {
      ccm_param::motion_3d_model.xyz_max_pos[2] = ZMaxPosValue;
      ccm_param::motion_3d_model.xyz_move_max_pos[2] = ZMaxPosValue;
    }
    else
    {
      ZMaxPosValue = ccm_param::motion_3d_model.xyz_max_pos[2];
    }

    t_sys_data_current.poweroff_rec_z_max_value = ZMaxPosValue;
    sysConfigOperation.ChangeZMaxPosValueInfo();
    sysConfigOperation.saveInfo(false);
  }
}

void SaveMatCheckAvgVol(float MatCheckAvgVolValue) //保存测量的断料续打模块空载时的平均电压值
{
  t_sys_data_current.material_chk_vol_value = MatCheckAvgVolValue;
  sysConfigOperation.ChangeMatCheckAvgVolValueInfo();
  sysConfigOperation.saveInfo(false);
}

void SaveBedLevelZValue(void)
{
  sysConfigOperation.ChangeBedLevelZAtLFValue();
  sysConfigOperation.ChangeBedLevelZAtRFValue();
  sysConfigOperation.ChangeBedLevelZAtLBValue();
  sysConfigOperation.ChangeBedLevelZAtRBValue();
  sysConfigOperation.ChangeBedLevelZAtMiddleValue();
  sysConfigOperation.saveInfo(false);
}

void SavePidOutputFactorValue(void)
{
  sysConfigOperation.ChangePidOutputFactorValue();
  sysConfigOperation.saveInfo(false);
}

void SaveSelectedlogo(void)
{
  sysConfigOperation.ChangelogoInfo();
  sysConfigOperation.saveInfo(true);
}

void SaveBezzerSound(void) //保存soundValua；
{
  sysConfigOperation.ChangeBezzerSound();
  sysConfigOperation.saveInfo(false);
}

void SaveZOffsetZero(void)
{
  sysConfigOperation.ChangeZOffsetZeroValue();
  sysConfigOperation.saveInfo(false);
}

#ifdef __cplusplus
} //extern "C"
#endif

