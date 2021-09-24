#ifndef  COMMON_H
#define  COMMON_H

#include <ctype.h>
#include "touch.h"
#include "user_common.h"
#include "jshdisplay.h"
#include  "ff.h"
#include "flashconfig.h"
//#include "threed_engine.h"
#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////
#define MaxDisplayTextLength 23  //显示文件名时能显示的最大长度

/////////////////////////////////////////////////
typedef unsigned          char   u8;
typedef unsigned short     int   u16;
typedef unsigned           int   u32;
///////////////////////////////////////////////////
struct _textrange
{
  int x;
  int y;
  int rangex;
  int rangey;
};
//////////////////////////////////////////////////
struct _touch
{
  volatile  int touch_flag;
  volatile  int down_flag;
  volatile  int up_flag;
  volatile  int pixel_x;
  volatile  int pixel_y;
  volatile  int touch_x;
  volatile  int touch_y;
};

extern struct _touch touch ;
extern void picturedisplay(char *p);
////////////////////////////////////////////////////////
void SetTextDisplayRange(int x, int y, int rangex, int rangey, struct _textrange *p);
void SetTextDisplayRangeNormal(int x, int y, int rangex, int rangey, struct _textrange *p);
void ReadTextDisplayRangeInfo(struct _textrange textrange, u16 *p);
void CopyTextDisplayRangeInfo(struct _textrange textrange, const u16 *psource, u16 *p);
////////////////////////////////////////////////////////////////////
void  DisplayTextInRange(unsigned char *pstr, struct _textrange textrange, u16 *p, int size, u16 colour);
void  DisplayText(const unsigned char *pstr, int x, int y, int size, u16 colour);
void DisplayTextNormal(const unsigned char *pstr, int x, int y, int size, u16 colour);
////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
typedef void (*menufunc_t)(void);

/////////////////////////////////////////////////////////
void  maindisplayF(void);
///////////////////////////////////////////////////////
int IstouchxyDown(int touchx, int touchy, int rangex, int rangey); //add by zouxb
int IstouchxyUp(void);  //add by zouxb
int touchxy(int touchx, int touchy, int rangex, int rangey);
int TouchXY_NoBeep(int touchx, int touchy, int rangex, int rangey);
int touchrange(int touchx, int touchy, int rangex, int rangey);
/////////////////////////////////////////////////////////////////////////////
void guitouchscan(void);
//////////////////////////////////////////////////

//////////////////////////////////////////////////////
extern volatile int print_flage;
extern volatile int pauseprint;
//  extern int keysound;
//  extern int alarmsound;
extern u16 testcolor;

///////////////////////////////////////////////////////////

extern char printname[_MAX_LFN];
extern int page;
//////////////////////////////////////////////////////////


#ifdef __cplusplus
} //extern "C" {
#endif


namespace gui
{
  extern volatile menufunc_t current_display;
  extern volatile int rtc_flag ;

  extern void need_refresh(void);
  extern bool is_refresh(void);
  extern bool is_refresh_rtc(void);
  extern void set_current_display(const volatile menufunc_t display);
}

/////////////////////////////////////////
#endif

