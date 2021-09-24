#ifndef RESPONDGUI_H
#define RESPONDGUI_H

#ifdef __cplusplus
extern "C" {
#endif

#define MinTempWarning 0
#define MaxTempWarning 1
#define MaxTempBedWarning 2
#define HeatFailWarning 3
#define XMinLimitWarning 4
#define YMinLimitWarning 5
#define ZMinLimitWarning 6
#define XMaxLimitWarning 7
#define YMaxLimitWarning 8
#define ZMaxLimitWarning 9
#define IWDGResetWarning 10
#define FatfsWarning 11
#define ThermistorFallsWarning 12

void PopWarningInfo(uint8_t WarningSelectValue);
void ManagWarningInfo(void);
//void PopPromptInfo(uint8_t PromptInfoSelectValue , const char *TextInfo);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //RESPONDGUI_H

