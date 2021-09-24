#ifndef COMMONF_H
#define COMMONF_H

#ifdef __cplusplus
extern "C" {
#endif

extern void maindisplayF(void);

extern void ChangeFilamentStatus(void);
extern void ChangeFilamentConfirm(void);
extern void NotHaveMatToChangeFilament(void);

extern void DoorOpenWarning_StartPrint(void);
extern void DoorOpenWarningInfo_NotPrinting(void);
extern void DoorOpenWarningInfo_Printing(void);

extern void filescanF(void);

extern void NotHaveMatControlInterface2(void);
extern void NotHaveMatControlInterface1(void);
extern void NoHaveMatWaringInterface(void);

extern void CalculatingZMaxLimit(void);

extern void MoveXYZ(void);

extern void NoUdiskF(void);

extern void PausePrintF(void);
extern void ResumePrintF(void);


extern void PowerOffRecover(void);
extern void PowerOffRecoverReady(void);

extern void prepareF(void);

extern void printconfirmF(void);

extern void printfinishF(void);

extern void PrintSet_M14(void);
extern void PrintSet_NotM14_Left(void);
extern void PrintSet_NotM14_Right(void);
extern void PrintSet_Cavity(void);
extern void SetLoadFilamentNozzleTemp(void);
extern void SetUnLoadFilamentNozzleTemp(void);
extern void SetPowerOffRecoverNozzleTemp(void);
extern void SetPowerOffRecoverHotBedTemp(void);

extern void PrintSet_NotM14_Left_dual(void);
extern void SetLoadFilamentNozzleTemp_dual(void);
extern void SetUnLoadFilamentNozzleTemp_dual(void);
extern void SetPowerOffRecoverNozzleTemp_dual(void);

extern void MachineSetting(void);

extern void settingF(void);

extern void statusF(void);

extern void stopprintF(void);

extern void WarningInfoF(void);

extern void loadfilament0F_dual(void);
extern void loadfilament0F(void);
extern void loadfilament1F(void);
extern void loadfilament2F(void);

extern void unloadfilament0F_dual(void);
extern void unloadfilament0F(void);
extern void unloadfilament1F(void);
extern void unloadfilament2F(void);

extern void page_homing(void); //归零页面
extern void goto_page_homing(void); //跳转归零页面
extern void gui_p3_pro_setting(void);
extern void settingF_p2_pro(void);

extern void waiting_for_stopping(void);
extern void waiting_for_resuming(void);
extern void waiting_for_pausing(void);
extern void waiting_for_xyz_moving(void);

extern void gui_p3_pro_idex_model_select(void);
extern void gui_p3_pro_setting_model_set(void);

extern void gui_p3_pro_mix_model_select(void);
#ifdef __cplusplus
} //extern "C" {
#endif

#endif


