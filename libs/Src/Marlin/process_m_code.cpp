#include "process_m_code.h"
#include "gcodebufferhandle.h"
#include "stepper.h"
#include "stepper_pin.h"
#include "user_common.h"
#include "Configuration.h"
#include "temperature.h"
#include "config_model_tables.h"
#include  "interface.h"
#include "globalvariables.h"
#include "temperature_pid_temp.h"
#include "planner.h"
#include "functioncustom.h"
#include "sysconfig_data.h"
#include <arm_math.h>
#include "user_fan.h"
#include "commonf.h"
#include "user_ccm.h"
#include "gcode.h"
#include "planner.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "PrintControl.h"

extern uint8_t tmp_extruder;
extern unsigned long previous_xTaskGetTickCount_cmd;
extern void manage_synchronize(void);

extern float add_homeing[XYZ_NUM_AXIS];
extern int feedmultiply;
extern osSemaphoreId ReadUdiskSemHandle;


bool setTargetedHotend(int16_t code)
{
  tmp_extruder = gcode::active_extruder;

  if (parseGcodeBufHandle.codeSeen('T'))
  {
    tmp_extruder = (unsigned char)parseGcodeBufHandle.codeValue();

    if (tmp_extruder >= EXTRUDERS)
    {
      USER_ErrLogStart();
      USER_ErrLogStr("M%d Invalid extruder ", code); // M104 M105 M109 M218 M221
      USER_ErrLogStr("%d\n", tmp_extruder);
      return true;
    }
  }

  return false;
}

void process_m_code(void)
{
  switch ((int)parseGcodeBufHandle.codeValue())
  {
  case 104: // M104 Set Extruder Temperature
    gcode::m104_process();
    break;

  case 140: // M140 set bed temp
    gcode::m140_process();
    break;

  case 105 : // M105 Get Extruder Temperature
    gcode::m105_process();
    break;

  case 109:  // M109 - Wait for extruder heater to reach target.
    gcode::m109_process();
    break;

  case 190: // M190 - Wait for bed heater to reach target.
    gcode::m190_process();
    break;

  case 114://M114  Get Current Position
    gcode::m114_process();
    break;

  //#if defined(FAN_PIN) && FAN_PIN_ON > -1
  case 106: //M106 Fan On
    gcode::m106_process();
    break;

  case 107: //M107 Fan Off
    gcode::m107_process();
    break;

  //#endif //FAN_PIN
  case 82://M82 Set extruder to absolute mode
    planner_settings.axis_relative_modes[E_AXIS] = false;

    if (1 == t_sys_data_current.enable_color_mixing)  planner_settings.axis_relative_modes[B_AXIS] = false;

    break;

  case 83://M83 Set extruder to relative mode
    planner_settings.axis_relative_modes[E_AXIS] = true;

    if (1 == t_sys_data_current.enable_color_mixing) planner_settings.axis_relative_modes[B_AXIS] = true;

    break;

  case 84: // M84 Stop idle hold
    gcode::m84_process();
    break;

  case 92: // M92 Set axis_steps_per_unit
    gcode::m92_process();
    break;

  case 120: // M120
    st_synchronize();//??????????????????????????????
    st_enable_endstops(false) ;
    break;

  case 121: // M121
    st_synchronize();//??????????????????????????????
    st_enable_endstops(true) ;
    break;

  case 200: // M200 D<millimeters> set filament diameter and set E axis units to cubic millimeters (use S0 to set back to millimeters).
    gcode::m200_process();
    break;

  case 201: // M201
    gcode::m201_process();
    break;

  case 203: // M203 max feedrate mm/sec
    gcode::m203_process();
    break;

  case 204: // M204 acclereration S normal moves T filmanent only moves
    gcode::m204_process();
    break;

  case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
    gcode::m205_process();
    break;

  case 206: // M206 additional homing offset
    for (int8_t i = 0; i < 3; i++)
    {
      if (parseGcodeBufHandle.codeSeen(axis_codes[i])) add_homeing[i] = parseGcodeBufHandle.codeValue();
    }

    break;

  case 207: // M207 Calibrate z axis by detecting z max length
    //??????????????????????????????2017.7.6
    USER_EchoLogStr("Start cal_z_max_pos\r\n");
    respond_gui_send_sem(CalculateZMaxPos);
    //    poweroff_start_cal_z_max_pos();
    break;

  case 220: // M220 S<factor in percent>- set speed factor override percentage
    if (parseGcodeBufHandle.codeSeen('S')) feedmultiply = (int)parseGcodeBufHandle.codeValue() ;

    //      USER_EchoLogStr("feedmultiply=%d\r\n",feedmultiply);//0~200;default:100
    break;

  case 221: // M221 S<factor in percent>- set extrude factor override percentage
    gcode::m221_process();
    break;

  case 301: // M301
    gcode::m301_process();
    break;
    #ifdef PREVENT_DANGEROUS_EXTRUDE

  case 302: // allow cold extrudes, or set the minimum extrude temperature
    gcode::m302_process();
    break;
    #endif

  case 303: // M303 PID autotune
    gcode::m303_process();
    break;

  case 400: // M400 finish all moves
    st_synchronize();
    break;

  case 600: //M600//Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
    gcode::m600_process();
    break;

  case 601: //M601//Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
    gcode::m601_process();
    break;

  case 117://M117 start Printing

    //??????????????????????????????2017.7.6
    //    USER_EchoLogStr("M117 Printing!\r\n");
    //    print_flage = 1;
    if (parseGcodeBufHandle.codeSeen('F'))
      t_gui.cura_speed = parseGcodeBufHandle.codeValue();
    else
      t_gui.cura_speed = 40;

    break;

  case 305://M305
    if (parseGcodeBufHandle.codeSeen('S')) setForceVerify(parseGcodeBufHandle.codeValue());

    break;

  case 2000:// M2000 ????????????????????????????????????true
    SetPrintStatus(true);
    break;

  case 2001:// M2001 ??????????????????????????????????????????true
    SetPrintStatus(true);
    break;

  case 2002:// M2002 ?????????U?????????
    (void)osSemaphoreRelease(ReadUdiskSemHandle);
    break;

  case 2003: // M2003 ??????????????????????????????
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      plan_set_process_auto_bed_level_status(parseGcodeBufHandle.codeValueLong() == 1 ? true : false);
    }

  case 2004: // M2004 ??????
    st_synchronize();

    if (parseGcodeBufHandle.codeSeen('S'))
    {
      extern volatile uint8_t is_gui_waiting_done;
      is_gui_waiting_done = parseGcodeBufHandle.codeValueLong();
    }

    break;

  default:
    break;
  }
}

#ifdef __cplusplus
} // extern "C" {
#endif







