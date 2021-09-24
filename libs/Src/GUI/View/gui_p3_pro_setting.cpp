#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "gcode.h"
#include "config_model_tables.h"
#include "user_ccm.h"
#include "ConfigurationStore.h"
#include "Configuration.h"
#include "planner.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void dual_extruder_setting(void);
extern void dual_home_pos_setting(void);

extern void gui_p3_pro_setting_idex(void);

static uint8_t extruder_type_bak;

void gui_p3_pro_setting_cal_bedlevel(void)
{
  if (gui::is_refresh())
  {
    display_picture(162);//按键、警报、触摸校准、断电、校准平台
  }

  if (gui::is_refresh_rtc())
  {
    if (gcode::g29_complete_flag)
    {
      gui::set_current_display(settingF);
    }
  }
}

void gui_p3_pro_setting_model_select(void)
{
  if (gui::is_refresh())
  {
    display_picture(168);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      LCD_Fill(115, 88, 115 + 20, 88 + 12, (u16)testcolor);
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
    {
      LCD_Fill(209, 88, 209 + 20, 88 + 12, (u16)testcolor);
    }
    else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
    {
      LCD_Fill(310, 88, 310 + 20, 88 + 12, (u16)testcolor);
    }
  }

  if (touchxy(54, 84, 140, 154)) //idex模式
  {
    if (flash_param_t.extruder_type != EXTRUDER_TYPE_DUAL)
    {
      flash_param_t.extruder_type = EXTRUDER_TYPE_DUAL;

      if (P3_Pro == t_sys_data_current.model_id)
      {
        t_sys.is_idex_extruder = 1;
        gcode::active_extruder = 0;
        ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = false;
        ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = true;
        planner_settings.axis_steps_per_mm[E_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_IDEX;
        planner_settings.axis_steps_per_mm[B_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_IDEX;
        axis_steps_per_sqr_second[E_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[E_AXIS] * planner_settings.axis_steps_per_mm[E_AXIS]);
        axis_steps_per_sqr_second[B_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[B_AXIS] * planner_settings.axis_steps_per_mm[B_AXIS]);
      }
      else if (F400TP == t_sys_data_current.model_id)
      {
        t_sys.is_idex_extruder = 1;
        gcode::active_extruder = 0;
        ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = false;
        ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = true;
      }
      else
      {
        t_sys.is_idex_extruder = 0;
      }

      gui::set_current_display(gui_p3_pro_setting_model_select);
      return ;
    }
  }

  if (touchxy(148, 84, 236, 154)) //混色模式
  {
    if (flash_param_t.extruder_type != EXTRUDER_TYPE_MIX)
    {
      flash_param_t.extruder_type = EXTRUDER_TYPE_MIX;

      if (P3_Pro == t_sys_data_current.model_id)
      {
        gcode::active_extruder = 0;
        ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
        ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = false;
        planner_settings.axis_steps_per_mm[E_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_MIX;
        planner_settings.axis_steps_per_mm[B_AXIS] = EB_AXIS_STEPS_PER_UNIT_P3_PRO_MIX;
        axis_steps_per_sqr_second[E_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[E_AXIS] * planner_settings.axis_steps_per_mm[E_AXIS]);
        axis_steps_per_sqr_second[B_AXIS] = (unsigned long)(planner_settings.max_acceleration_mm_per_s2[B_AXIS] * planner_settings.axis_steps_per_mm[B_AXIS]);
      }
      else if (F400TP == t_sys_data_current.model_id)
      {
        gcode::active_extruder = 0;
        ccm_param::motion_3d_model.enable_invert_dir[E_AXIS] = true;
        ccm_param::motion_3d_model.enable_invert_dir[B_AXIS] = false;
      }
      else
      {
      }

      gui::set_current_display(gui_p3_pro_setting_model_select);
      return ;
    }
  }

  if (touchxy(246, 84, 334, 154)) //激光模式
  {
    if (flash_param_t.extruder_type != EXTRUDER_TYPE_LASER)
    {
      if (P3_Pro == t_sys_data_current.model_id) // 激光头使用喷头2
      {
        flash_param_t.extruder_type = EXTRUDER_TYPE_LASER;
        gcode::active_extruder = 1;
        gui::set_current_display(gui_p3_pro_setting_model_select);
      }
    }
  }

  if (touchxy(72, 16, 150, 50)) //返回键
  {
    if (extruder_type_bak != flash_param_t.extruder_type)
    {
      flash_param_t.flag = 1;
    }

    gui::set_current_display(settingF);
    return ;
  }
}
extern volatile uint8_t idex_ext_bed_index;
extern void gui_p3_pro_setting_ext1_comp(void);
extern void gui_p3_pro_setting_home_pos(void);
extern void gui_p3_pro_setting_ext1_bed(void);
void gui_p3_pro_setting_mix(void)
{
  if (gui::is_refresh())
  {
    display_picture(172);
  }

  if (touchxy(52, 128, 140, 186)) //喷头1热床
  {
    idex_ext_bed_index = 0;
    gui::set_current_display(gui_p3_pro_setting_ext1_bed);
    return ;
  }

  if (touchxy(148, 128, 236, 186)) //喷头2热床
  {
    gui::set_current_display(gui_p3_pro_setting_home_pos);
    return ;
  }

  if (touchxy(0, 0, 150, 70)) //返回键
  {
    SaveBezzerSound();
    gui::set_current_display(settingF);
    return ;
  }
}

void gui_p3_pro_setting_laser(void)
{
  if (gui::is_refresh())
  {
    display_picture(173);
  }

  if (touchxy(52, 128, 140, 186)) //喷头1热床
  {
    idex_ext_bed_index = 0;
    gui::set_current_display(gui_p3_pro_setting_ext1_bed);
    return ;
  }

  if (touchxy(148, 128, 236, 186)) //喷头2热床
  {
    idex_ext_bed_index = 1;
    gui::set_current_display(gui_p3_pro_setting_ext1_bed);
    return ;
  }

  if (touchxy(246, 128, 332, 186)) //喷头2补偿
  {
    gui::set_current_display(gui_p3_pro_setting_home_pos);
    return ;
  }

  if (touchxy(0, 0, 150, 70)) //返回键
  {
    SaveBezzerSound();
    gui::set_current_display(settingF);
    return ;
  }
}

void gui_p3_pro_setting_model_set(void)
{
  if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
  {
    gui_p3_pro_setting_idex();
    return ;
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_MIX)
  {
    gui_p3_pro_setting_mix();
    return ;
  }
  else if (flash_param_t.extruder_type == EXTRUDER_TYPE_LASER)
  {
    gui_p3_pro_setting_laser();
    return ;
  }
}

void gui_p3_pro_setting(void)
{
  if (gui::is_refresh())
  {
    display_picture(166);//按键、警报、触摸校准、断电、校准平台

    if (BED_LEVEL_PRESSURE_SENSOR != t_sys_data_current.enable_bed_level)
    {
      LCD_Fill(148, 182, 240, 256, BACKBLUE);
    }

    if (t_sys.key_sound)
    {
      LCD_Fill(115, 96, 115 + 20, 96 + 12, (u16)testcolor);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill(209, 96, 209 + 20, 96 + 12, (u16)testcolor);
    }
  }

  if (touchxy(22, 113, 118, 186)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill(115, 96, 115 + 20, 96 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(135, 113, 230, 186)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui::set_current_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill(209, 96, 209 + 20, 96 + 12, (u16)testcolor);
      (void)OS_DELAY(500);
    }
  }

  if (touchxy(248, 113, 345, 186)) //触摸校正键
  {
    tp_dev.adjust();
    gui::set_current_display(settingF);
    return ;
  }

  if (touchxy(365, 113, 461, 186)) //模式选择
  {
    extruder_type_bak = flash_param_t.extruder_type;
    gui::set_current_display(gui_p3_pro_setting_model_select);
    return ;
  }

  if (touchxy(58, 184, 140, 252)) //模式设置
  {
    gui::set_current_display(gui_p3_pro_setting_model_set);
    return ;
  }

  if (BED_LEVEL_PRESSURE_SENSOR == t_sys_data_current.enable_bed_level)
  {
    if (touchxy(135, 219, 230, 294)) //堵料开关
    {
      gcode::g29_complete_flag = false;
      gui::set_current_display(gui_p3_pro_setting_cal_bedlevel);
      respond_gui_send_sem(StartCalBedLevel);
      return ;
    }
  }

  /*
    if (touchxy(365, 113, 461, 186)) //Z行程测量键
    {
      gui::set_current_display(dual_extruder_setting);
      return ;
    }

    if (touchxy(22, 219, 118, 294)) //计算平台
    {
      gcode::g29_complete_flag = false;
      gui::set_current_display(p3_pro_cal_bedlevel);
      respond_gui_send_sem(StartCalBedLevel);
      return ;
    }

    if (touchxy(135, 219, 230, 294)) //堵料开关
    {
      gui::set_current_display(dual_home_pos_setting);
      return ;
    }
  */
}

#ifdef __cplusplus
} //extern "C" {
#endif


