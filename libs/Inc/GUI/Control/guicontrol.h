#ifndef GUICONTROL_H
#define GUICONTROL_H

#include "cmsis_os.h"
#include "threed_engine.h"
class GUIControl
{
public:
  GUIControl();
  void disableSteppers(void);
  void moveXYZ(int (&xyz_mm)[XYZ_NUM_AXIS]);
  void preHeatABS(void);
  void preHeatPLA(void);
  void preHeatBed(void);
  void printSetForM14(void);
  void printSetForNotM14Left(void);
  void printSetForNotM14Right(void);
  void coolDown(void);
  void refreshGuiInfo(void);
  void printSetForCavity(void);
};

extern GUIControl guiControl;

#ifdef __cplusplus
extern "C" {
#endif

void PauseToResumeTemp(void);

#ifdef __cplusplus
}//extern "C" {
#endif

#endif // GUICONTROL_H
