#ifndef MIDWAYCHANGEMATERIAL_H
#define MIDWAYCHANGEMATERIAL_H

#include "threed_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

int IsMidWayChangeMat(void);
void setMidWayChangeMat(bool value);

void ConfirmLoadFilament(void);
void IsCompleteHeat(void);
void ChangeFilament(void);
void RefChangeFilamentStatus(void);

#ifdef __cplusplus
} // extern "C" {
#endif


#endif // MIDWAYCHANGEMATERIAL_H

