#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "gcode.h"

#ifdef __cplusplus
extern "C" {
#endif

void DisableStepInfo(void)
{
  if (gui::is_refresh())
  {
    display_picture(63);
  }

  if (touchxy(160, 200, 320, 300)) //确定键
  {
    gui::set_current_display(prepareF);
    return ;
  }
}

void prepareF(void)
{
  if (gui::is_refresh())
  {
    // 日本定制固件，不显示预热pla、abs，只显示预热热床
    if (5 == t_sys_data_current.custom_model_id && PICTURE_IS_JAPANESE == t_sys_data_current.pic_id)
    {
      display_picture(99);
    }
    else
    {
      if (t_custom_services.disable_abs) //不能打印ABS
      {
        display_picture(44);
      }
      else
      {
        if (7 == t_sys_data_current.custom_model_id && PICTURE_IS_JAPANESE == t_sys_data_current.pic_id)
          display_picture(106);
        else
          display_picture(4);
      }
    }
  }

  if (touchxy(0, 0, 150, 65))
  {
    gui::set_current_display(maindisplayF);
    return ;
  }

  if (touchxy(18, 115, 113, 187))
  {
    gcode::g28_complete_flag = false;
    gui::set_current_display(goto_page_homing);
    respond_gui_send_sem(BackZeroValue);
    return ;
  }

  if (touchxy(133, 115, 229, 187))
  {
    respond_gui_send_sem(FeedFilamentValue);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(loadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(loadfilament0F);
    }

    return ;
  }

  if (touchxy(249, 115, 343, 187))
  {
    respond_gui_send_sem(BackFilamentValue);

    if (flash_param_t.extruder_type == EXTRUDER_TYPE_DUAL)
    {
      gui::set_current_display(unloadfilament0F_dual);
    }
    else
    {
      gui::set_current_display(unloadfilament0F);
    }

    return ;
  }

  if (touchxy(364, 115, 459, 187))
  {
    gui::set_current_display(MoveXYZ);
    return ;
  }

  if (touchxy(18, 218, 113, 290))
  {
    respond_gui_send_sem(DisableStepValue);
    gui::set_current_display(DisableStepInfo);
    return ;
  }

  if (5 == t_sys_data_current.custom_model_id && PICTURE_IS_JAPANESE == t_sys_data_current.pic_id)
  {
    if (touchxy(133, 218, 229, 290))
    {
      respond_gui_send_sem(PreHeatBedValue);
      gui::set_current_display(maindisplayF);
      return ;
    }

    if (touchxy(249, 218, 343, 290))
    {
      respond_gui_send_sem(CoolDownValue);
      gui::set_current_display(maindisplayF);
      return ;
    }
  }
  else
  {
    if (touchxy(133, 218, 229, 290))
    {
      respond_gui_send_sem(PreHeatPLAValue);
      gui::set_current_display(maindisplayF);
      return ;
    }

    if (t_custom_services.disable_abs) //不能打印ABS,无预热ABS按键
    {
      if (touchxy(249, 218, 343, 290))
      {
        respond_gui_send_sem(CoolDownValue);
        gui::set_current_display(maindisplayF);
        return ;
      }
    }
    else
    {
      if (touchxy(249, 218, 343, 290))
      {
        respond_gui_send_sem(PreHeatABSValue);
        gui::set_current_display(maindisplayF);
        return ;
      }

      if (touchxy(364, 218, 459, 290))
      {
        respond_gui_send_sem(CoolDownValue);
        gui::set_current_display(maindisplayF);
        return ;
      }
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

