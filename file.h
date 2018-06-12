#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "stdint.h"
#include "ide.h"
#include "dir.h"
#include "global.h"

/*文件结构*/
struct file 
{
    uint32_t fd_pos;        /*文件当前的偏移量*/
    uint32_t fd_flag;       /*文件打开的标志*/
    struct inode* fd_inode; /*文件依赖的inode*/
};

/*标准输入输出描述符*/
enum std_fd 
{
    stdin_no,   /*标准输入*/
    stdout_no,  /*标准输出*/
    stderr_no   /*标准错误*/
};

/* 位图类型 */
enum bitmap_type 
{
    INODE_BITMAP,     
    BLOCK_BITMAP	    
};

#define MAX_FILE_OPEN 32    /*系统可打开的最大文件数*/

extern struct file file_table[MAX_FILE_OPEN];
int32_t inode_bitmap_alloc(struct partition* part);
int32_t block_bitmap_alloc(struct partition* part);
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag);
void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp);
int32_t get_free_slot_in_global(void);
int32_t pcb_fd_install(int32_t globa_fd_idx);
int32_t file_open(uint32_t inode_no, uint8_t flag);
int32_t file_close(struct file* file);
int32_t file_write(struct file* file, const void* buf, uint32_t count);
int32_t file_read(struct file* file, void* buf, uint32_t count);
#endif
