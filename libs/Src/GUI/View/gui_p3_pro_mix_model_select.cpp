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

void gui_p3_pro_mix_model_select(void)
{
  if (gui::is_refresh())
  {
    display_picture(171);
  }

  if (touchxy(74, 100, 162, 170)) //渐变模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.mix_print_type = MIX_PRINT_TYPE_GRADIENT_COLOR;
    gui::set_current_display(printconfirmF);
    return ;
  }

  if (touchxy(192, 100, 280, 170)) //固定模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.mix_print_type = MIX_PRINT_TYPE_FIX_PROPORTION;
    gui::set_current_display(printconfirmF);
    return ;
  }

  if (touchxy(310, 100, 400, 170)) //随机模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.mix_print_type = MIX_PRINT_TYPE_RANDOM;
    gui::set_current_display(printconfirmF);
    return ;
  }
}


#ifdef __cplusplus
} //extern "C" {
#endif


