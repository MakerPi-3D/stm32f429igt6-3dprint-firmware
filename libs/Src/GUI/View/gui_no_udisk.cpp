#include "common.h"
#include "commonf.h"

#ifdef __cplusplus
extern "C" {
#endif

void NoUdiskF(void)
{
  if (gui::is_refresh())
  {
    display_picture(43);
  }

  if (touchxy(160, 190, 330, 285))
  {
    gui::set_current_display(maindisplayF);
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

