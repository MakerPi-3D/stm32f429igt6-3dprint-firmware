#ifndef CONFIG_MODEL_TABLES_H
#define CONFIG_MODEL_TABLES_H

#include "threed_engine.h"
#ifdef __cplusplus
extern "C" {
#endif

// 機型表
extern const short model_id_table[];
extern const int   model_size_table[][XYZ_NUM_AXIS];
extern const char *model_name_table[];

// 機型ID
#define M14          ((short)0)
#define M2030        ((short)1)
#define M2041        ((short)2)
#define M2048        ((short)3)
#define M3145        ((short)4)
#define M4141        ((short)5)
#define M4040        ((short)6)
#define M4141S       ((short)7)
#define AMP410W      ((short)8)
#define M14R03       ((short)9)
#define M2030HY      ((short)10)
#define M14S         ((short)11)
#define M3145S       ((short)12)
#define M15          ((short)13)
#define M3036        ((short)14)
#define M4141S_NEW   ((short)15)
#define M41G         ((short)16)
#define M3145T       ((short)17)
#define M3145K       ((short)18)
#define K5           ((short)19)
#define F400TP       ((short)20)
#define F1000TP      ((short)21)
#define P2_Pro       ((short)22)
#define P3_Pro       ((short)23)

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CONFIG_MODEL_TABLES_H

