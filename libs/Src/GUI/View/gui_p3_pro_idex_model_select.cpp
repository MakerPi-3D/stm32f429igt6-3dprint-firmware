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

void gui_p3_pro_idex_model_select(void)
{
  if (gui::is_refresh())
  {
    display_picture(170);
  }

  if (touchxy(74, 100, 162, 170)) //正常模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.idex_print_type = IDEX_PRINT_TYPE_NORMAL;
    gui::set_current_display(printconfirmF);
    return ;
  }

  if (touchxy(192, 100, 280, 170)) //复制模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.idex_print_type = IDEX_PRINT_TYPE_COPY;
    gui::set_current_display(printconfirmF);
    return ;
  }

  if (touchxy(310, 100, 400, 170)) //镜像模式
  {
    respond_gui_send_sem(ResetEBValue);
    osDelay(50);
    t_sys.idex_print_type = IDEX_PRINT_TYPE_MIRROR;
    gui::set_current_display(printconfirmF);
    return ;
  }
}


#ifdef __cplusplus
} //extern "C" {
#endif


