#ifndef CONTROLFUNCTION_H
#define CONTROLFUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

void control_function_init(void);
void control_function_process(void);

void TIM7_IRQHandler_process(void); ///< ��ʱ��2�����ϼ��ִ��
void TIM3_IRQHandler_process(void); ///< ��ʱ��3���¶ȼ��ִ��
void TIM4_IRQHandler_process(void); ///< ��ʱ��4�����ִ�����
//void TIM7_IRQHandler_process(void); ///< ��ʱ��7�����ڳ�ʼ��ִ��

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CONTROLFUNCTION_H
