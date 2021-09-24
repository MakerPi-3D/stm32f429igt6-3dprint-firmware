#include <ctype.h>
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "globalvariables.h"
#include "common.h"
#include "interface.h"


static char Path[_MAX_LFN];
static int GUIpage;
static int PageItemNum;
static int CurrentPos;
static int IsCurrentPageFileScanFinished;
static FILINFO fileinfo;
static FILINFO sdfileinfo;
int CheckIsGcodeFile(const char *p)
{
  unsigned int i;
  char a[10];
  i = strlen(p);

  while (p[i] != '.')
  {
    --i;
  }

  (void)strcpy(a, &p[i + 1]);

  // 文件后缀转为小写
  for (i = 0; i < strlen(a); i++)
  {
    a[i] = tolower(a[i]);
  }

  if (0 == strcmp(a, "gcode")  || 0 == strcmp(a, "sgcode") ||
      0 == strcmp(a, "sgc") || 0 == strcmp(a, "gco")) // 长文件太长，文件名会变为8.3的方式
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

extern char NAND_Path[4];  //xiugai
void DelectSDFiles(void)
{
  if (t_power_off.is_file_from_sd)  return; //断电续打，但文件在U盘，删除SD卡里的文件

  char lfnamebuffer[_MAX_LFN];
  char PathAndFilename[_MAX_LFN];
  char *sdfname;
  DIR dirs;
  sdfileinfo.fsize = _MAX_LFN;
  memcpy(&sdfileinfo.fname[0], &lfnamebuffer[0], _MAX_LFN);
  //    sdfileinfo.fname = lfnamebuffer;
  taskENTER_CRITICAL();

  if (f_opendir(&dirs, NAND_Path) == FR_OK)
  {
    while ((f_readdir(&dirs, &sdfileinfo) == FR_OK) && sdfileinfo.fname[0])    //依次读取文件名
    {
      if ((sdfileinfo.fattrib & AM_DIR) ==  0)   //不是目录
      {
        #if _USE_LFN
        sdfname = *sdfileinfo.fname ? sdfileinfo.fname : sdfileinfo.altname;
        #else
        sdfname = fileinfo.fname;
        #endif

        if (CheckIsGcodeFile(sdfname))
        {
          (void)snprintf(PathAndFilename, sizeof(PathAndFilename), "%s", NAND_Path);
          (void)strcat(PathAndFilename, sdfname);
          (void)f_unlink(PathAndFilename);
        }
      }
    }

    (void)f_closedir(&dirs);
  }

  taskEXIT_CRITICAL();
}

void FindFolder(void)
{
  DIR dirs_files;
  int dirSkip;
  char lfnamebuf[_MAX_LFN];
  char *fname;
  fileinfo.fsize = _MAX_LFN;
  //    fileinfo.fname = lfnamebuf;
  memcpy(&fileinfo.fname[0], &lfnamebuf[0], _MAX_LFN);

  if (IsCurrentPageFileScanFinished) //若当前页已扫描完，不再扫描，直接返回
  {
    return;
  }

  user_mount_Udisk();
  OS_DELAY(50);

  if (f_opendir(&dirs_files, Path) == FR_OK)
  {
    USER_EchoLog("f_opendir DIR ok!\r\n");

    while ((f_readdir(&dirs_files, &fileinfo) == FR_OK) && fileinfo.fname[0]) //依次读取文件名
    {
      if (fileinfo.fattrib == AM_DIR)  //文件夹
      {
        #if _USE_LFN
        fname = *fileinfo.fname ? fileinfo.fname : fileinfo.altname;
        #else
        fname = fileinfo.fname;
        #endif
        dirSkip = strcmp("System Volume Information", fname);
        dirSkip = (dirSkip && strcmp("$RECYCLE.BIN", fname));
        dirSkip = (dirSkip && strcmp("LOST.DIR", fname));

        if (dirSkip) //过滤掉系统文件夹System Volume Information和$RECYCLE.BIN，以防由此引发的死机
        {
          if (CurrentPos >= (OnePageNum * GUIpage))
          {
            IsCurrentPageFileScanFinished = 1;
            IsHaveNextPage = 1;
            GUIpage++;
            (void)f_closedir(&dirs_files);
            return;
          }

          if (CurrentPos >= (OnePageNum * (GUIpage - 1)))
          {
            (void)strcpy(DisplayFileName[PageItemNum], fname);
            IsHaveFile[PageItemNum] = 1;
            IsDir[PageItemNum++] = 1;
          }

          CurrentPos++;
        }
      }
    }

    (void)f_closedir(&dirs_files);
  }
  else
  {
    USER_ErrLog("f_opendir DIR not ok!\r\n");
  }
}

void FindGcodeFile(void)
{
  DIR dirs_files;
  char lfnamebuf[_MAX_LFN];
  char *fname;
  fileinfo.fsize = _MAX_LFN;
  //    fileinfo.fname = lfnamebuf;
  memcpy(&fileinfo.fname[0], &lfnamebuf[0], _MAX_LFN);

  if (IsCurrentPageFileScanFinished) //若当前页已扫描完，不再扫描，直接返回
  {
    return;
  }

  if (f_opendir(&dirs_files, Path) == FR_OK)
  {
    while ((f_readdir(&dirs_files, &fileinfo) == FR_OK) && fileinfo.fname[0]) //依次读取文件名
    {
      if (fileinfo.fattrib == AM_ARC)  //文件
      {
        #if _USE_LFN
        fname = *fileinfo.fname ? fileinfo.fname : fileinfo.altname;
        #else
        fname = fileinfo.fname;
        #endif

        if (CheckIsGcodeFile(fname))
        {
          if (CurrentPos >= (OnePageNum * GUIpage))
          {
            IsCurrentPageFileScanFinished = 1;
            IsHaveNextPage = 1;
            GUIpage++;
            (void)f_closedir(&dirs_files);
            return;
          }

          if (CurrentPos >= (OnePageNum * (GUIpage - 1)))
          {
            (void)strcpy(DisplayFileName[PageItemNum], fname);
            IsHaveFile[PageItemNum] = 1;
            IsDir[PageItemNum++] = 0;
          }

          CurrentPos++;
        }
      }
    }

    (void)f_closedir(&dirs_files);
  }
}


void FileScan(void)  //在GUI界面显示的时候，文件夹显示在前面，Gcode文件显示在后面
{
  PageItemNum = 0;
  CurrentPos = 0;
  FindFolder();//寻找文件夹
  FindGcodeFile();//寻找Gcode文件
}

void CleanFilNameBuf(void)
{
  int i;

  for (i = 0; i < OnePageNum; i++)
  {
    DisplayFileName[i][0] = 0;
    IsHaveFile[i] = 0;
    IsDir[i] = 0;
  }

  IsHaveNextPage = 0;
  IsCurrentPageFileScanFinished = 0;
}

//打开SD卡
void GUIOpenSDCard(void)
{
  CleanFilNameBuf();
  (void)strcpy(Path, "0:/");
  GUIpage = 1;
  (void)strcpy(CurrentPath, Path);
  IsRootDir = 1;
  CurrentPage = GUIpage;
  FileScan();
}

//打开目录
void GUIOpenSDDir(void)
{
  int LastPathLength;
  LastPathLength = (int)strlen(Path);

  if (IsRootDir)
    (void)sprintf(&Path[LastPathLength], "%s", SettingInfoToSYS.DirName);
  else
    (void)sprintf(&Path[LastPathLength], "/%s", SettingInfoToSYS.DirName);

  GUIpage = 1;
  CleanFilNameBuf();
  (void)strcpy(CurrentPath, Path);
  IsRootDir = 0;
  CurrentPage = GUIpage;
  FileScan();
}

//返回上层目录
void GUIBackSDLastDir(void)
{
  CleanFilNameBuf();
  char *LastDirEndPos;
  int LastDirEndPosition;
  char *DiskPath;
  DiskPath = USBHPath;

  if (0 == strcmp(Path, DiskPath))
  {
    (void)strcpy(Path, DiskPath);
    IsRootDir = 1;
  }
  else
  {
    LastDirEndPos = strrchr(Path, '/');
    LastDirEndPosition = LastDirEndPos - Path;
    Path[LastDirEndPosition] = 0;

    if (0 == strcmp(Path, "0:"))
    {
      IsRootDir = 1;
      (void)strcpy(Path, DiskPath);
    }
    else
      IsRootDir = 0;
  }

  GUIpage = 1;
  (void)strcpy(CurrentPath, Path);
  CurrentPage = GUIpage;
  FileScan();
}

//点击下一页
void GUINextPage(void)
{
  CleanFilNameBuf();
  CurrentPage = GUIpage;
  FileScan();
}

//点击上一页
void GUILastPage(void)
{
  if ((IsHaveNextPage) == 1)
    GUIpage = GUIpage - 2;
  else
    GUIpage = GUIpage - 1;

  CleanFilNameBuf();
  CurrentPage = GUIpage;
  FileScan();
}

#ifdef __cplusplus
} // extern "C" {
#endif

