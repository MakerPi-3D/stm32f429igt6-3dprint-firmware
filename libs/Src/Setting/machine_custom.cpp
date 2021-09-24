#include "machine_custom.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void machine_custom_init(void)
{
  if (0 == t_sys_data_current.custom_model_id)
  {
    if (t_sys_data_current.enable_color_mixing)
    {
      if (P2_Pro != t_sys_data_current.model_id && P3_Pro == t_sys_data_current.model_id && F400TP == t_sys_data_current.model_id)
      {
        strcat(t_sys.model_str, "X");
      }
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif


