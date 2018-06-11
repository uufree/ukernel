#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H

#include "global.h"

#define BITMAP_MASK 1

struct bitmap {
   uint32_t btmp_bytes_len; /*位图表示的数据长度*/
   uint8_t* bits;   /*数据区*/
};

/* 将位图btmp初始化 */
void bitmap_init(struct bitmap* btmp);

/* 判断bit_idx位是否为1,若为1则返回true，否则返回false */
bool bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx);

/* 在位图中申请连续cnt个位,返回其起始位下标 */
int bitmap_scan(struct bitmap* btmp, uint32_t cnt);

/* 将位图btmp的bit_idx位设置为value */
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);

#endif
