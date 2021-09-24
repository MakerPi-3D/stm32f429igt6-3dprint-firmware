
#ifndef temperature_h
#define temperature_h

#ifdef __cplusplus
extern "C" {
#endif

#define NoError 0
#define HeatFailError 1
#define MaxTempError 2
#define MinTempError 3
#define MaxTempBedError 4
#define DETECT_PCB_FAULSE 5
#define ThermistorFallsOffError 6

void temperature_init(bool _is_open_cavity);            // ��ʼ������ϵͳ
void temperature_manage_heater(const int ext_fanspeed);   // ���ȿ��ƽӿ�
void temperature_disable_heater(void);  // ֹͣ����
bool temperature_update(void);          // �¶ȸ��£���ʱ��
int temperature_get_error_status(void); // ��ȡ�¶ȴ���״̬
void temperature_set_error_status(char Value);

// �����¶Ⱥ����ӿ�
float temperature_get_extruder_current(int extruder);
float temperature_get_extruder_target(int extruder);
void temperature_set_extruder_target(const float celsius, int extruder);
bool temperature_is_extruder_heating(int extruder);
bool temperature_is_extruder_cooling(int extruder);
int temperature_get_extruder_heater_power(int heater);

// �ȴ��¶Ⱥ����ӿ�
float temperature_get_bed_current(void);
float temperature_get_bed_target(void);
void temperature_set_bed_target(const float celsius);
bool temperature_is_bed_heating(void);
bool temperature_is_bed_cooling(void);
int temperature_get_bed_heater_power(void);

// ǻ���¶Ⱥ����ӿ�
float temperature_get_cavity_target(void);
float temperature_get_cavity_current(void);
void temperature_set_cavity_target(const float celsius);

void temperature_set_heater_maxtemp(int axis, int value);
int temperature_get_heater_maxtemp(int axis);

#ifdef __cplusplus
} //extern "C" {
#endif

namespace grbl_temp
{
  void pid_autotune(float temp, int extruder, int ncycles);
}

#endif

