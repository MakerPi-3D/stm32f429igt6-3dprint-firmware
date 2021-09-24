#ifndef JSHDISPLAY_H
#define JSHDISPLAY_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// ͼƬ���ļ�ͷ�ṹ
typedef struct SMaseFileHeader
{
  uint32_t  uFileFlag;          // ���ļ�ͷ���: 'MASE'
  uint32_t  uFileCount;         // �����ļ�����
  uint32_t  uFileListOfs;       // �ļ��б�ƫ��
  uint32_t  uMaxFileCount;      // ������ļ�����
  uint32_t  uFileSize;          // ���ļ��Ĵ�С
} MaseHeader;

//�����ļ���Ϣ�ṹ
typedef struct SFilesMessage
{
  uint32_t  uFileOfs;          // ���ļ��ڰ��ڵ�ƫ��
  uint32_t  uFileSize;         // ���ļ��Ĵ�С
  char  szFileName[60];   // ���ļ����ļ���������·��
} FilesMsg;

void display_picture(int PictureName);
void DisplayLogoPicture(int PictureName);

#define BMP_PATH "0:/file2.bmp"//��ȡsgcode�е�bmp�ļ���Ĵ��·��20170920
//��������
#define BarWidth 382  //����������
#define BarHeight 21  //���������
#define X_BEGIN 75    //������x����ʼλ��
#define Y_BEGIN 289   //������y����ʼλ��
uint32_t Draw_progressBar_new(uint32_t Printfilesize, uint32_t Filesize, int x, int y, int x_max, int y_max);
uint32_t Draw_progressBar(uint32_t Printfilesize, uint32_t Filesize);
void diplayBMP(unsigned int y_offset);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
