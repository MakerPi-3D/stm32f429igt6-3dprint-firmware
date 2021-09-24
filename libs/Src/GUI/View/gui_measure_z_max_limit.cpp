#include "common.h"
#include "commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "config_motion_3d.h"
#include "user_ccm.h"
#ifdef __cplusplus
extern "C" {
#endif

void FinishedCalculateZMaxLimit(void)
{
  char TextBuffer[20];

  if (gui::is_refresh())
  {
    display_picture(48);
    //显示Z轴最大位置
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)ccm_param::motion_3d_model.xyz_move_max_pos[Z_AXIS]);
    DisplayText((uint8_t *)TextBuffer, 245, 141, 24, (u16)testcolor);
  }

  if (touchxy(165, 210, 320, 290))
  {
    gui::set_current_display(settingF);
  }
}


void CalculatingZMaxLimit(void)
{
  if (gui::is_refresh())
  {
    display_picture(47);
  }

  if (IsFinishedCalculateZPosLimit)
  {
    IsFinishedCalculateZPosLimit = 0;
    gui::set_current_display(FinishedCalculateZMaxLimit);
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

