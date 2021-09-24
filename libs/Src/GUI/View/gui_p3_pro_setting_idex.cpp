#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_common.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "gcode.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void gui_p3_pro_setting_ext1_comp(void);
extern void gui_p3_pro_setting_home_pos(void);
extern void gui_p3_pro_setting_ext1_bed(void);
extern volatile uint8_t idex_ext_bed_index;
void gui_p3_pro_setting_idex(void)
{
  if (gui::is_refresh())
  {
    display_picture(165);
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
    gui::set_current_display(gui_p3_pro_setting_ext1_comp);
    return ;
  }

  if (touchxy(342, 128, 430, 186)) //归零点补偿
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




#ifdef __cplusplus
} //extern "C" {
#endif


