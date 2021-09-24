#ifndef CONTROLXYZEB_H
#define CONTROLXYZEB_H

#ifdef __cplusplus
extern "C" {
#endif

void control_xyz_init(void);
void z_down_60mm_and_xy_to_zero(void);                                                ///< z�½�60mm��xy����
void xy_to_zero(void);                                                                ///< xy����
void z_down_to_bottom(void);                                                          ///< z�½����ײ�
void z_down_to_bottom_and_xy_to_zero(void);                                           ///< z�½����ײ���xy����
void z_check_and_set_bottom(const bool isUpDownMinMin, const float zBottomValue);     ///< z���ײ�������
void eb_compensate_8mm(bool isColorMix);                                                         ///< eb����8mm
void m84_disable_steppers(void);                                                      ///< m84����xyeb
void g92_set_axis_position(const int axis, const float value);                        ///< g92����xyzebλ��

#ifdef __cplusplus
} //extern "C" {
#endif

class ControlXYZEB
{
public:
  ControlXYZEB();
  void xyMoveToZero(void);                                           ///< xy����
  void zDown60MM(void);                                              ///< z�½�60mm
  void zDownToBottom(void);                                          ///< z�½����ײ�
  void zCheckTheBottom(const bool isUpDownMinMin);                   ///< z���ײ�λ��
  void ebCompensate8mm(bool isColorMix);                                        ///< eb����8mm�����ڶϵ�����ͣ��ӡ
  void m84DisableXYEB(void);                                         ///< m84�������
  void g92SetAxisPosition(const int axis, const float value);        ///< g92���õ�ǰλ��
};

#endif // CONTROLXYZEB_H
