#include "inode.h"
#include "fs.h"
#include "file.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "interrupt.h"
#include "list.h"
#include "stdio.h"
#include "string.h"
#include "super_block.h"

/*描述inode在磁盘中的存储位置*/
struct inode_position 
{
    bool	 two_sec;	/*inode是否跨扇区*/
    uint32_t sec_lba;	/*inode所在的扇区号*/
    uint32_t off_size;	/*inode在扇区内的字节偏移量*/
};

/*获取inode所在的扇区和扇区内的偏移量*/
static void inode_locate(struct partition* part, uint32_t inode_no, struct inode_position* inode_pos) 
{
    ASSERT(inode_no < 4096);
    /*从超级块中获取inode表的起始位置*/
    uint32_t inode_table_lba = part->sb->inode_table_lba;

    uint32_t inode_size = sizeof(struct inode);
    uint32_t off_size = inode_no * inode_size;/*获取inode的偏移量*/
    uint32_t off_sec  = off_size / 512;/*相对于inode表的扇区偏移量*/
    uint32_t off_size_in_sec = off_size % 512;/*在扇区中的偏移地址*/

    /*判断此i结点是否跨越2个扇区*/
    uint32_t left_in_sec = 512 - off_size_in_sec;
    if (left_in_sec < inode_size ) 	
        inode_pos->two_sec = true;
    else 				
        inode_pos->two_sec = false;
    
    /*正式的计算出inode在扇区中的位置*/
    inode_pos->sec_lba = inode_table_lba + off_sec;
    inode_pos->off_size = off_size_in_sec;
}

/* 将inode写入到分区part */
void inode_sync(struct partition* part, struct inode* inode, void* io_buf) 
{	 
    uint8_t inode_no = inode->i_no;
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);/*获取inode的inode表中的位置*/
    ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
   
    /*inode在内存中表示的信息与在磁盘中表示的信息略有不同*/
    struct inode pure_inode;
    memcpy(&pure_inode, inode, sizeof(struct inode));

   /*以下inode的三个成员只存在于内存中,现在将inode同步到硬盘,清掉这三项即可*/
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;	
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;

    char* inode_buf = (char*)io_buf;
    if (inode_pos.two_sec) 
    {
        /*读取两个扇区，将inode的信息填充到扇区中*/
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
        memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } 
    else 
    {			    
        /*读取一个扇区，同步inode信息*/
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

/* 根据i结点号返回相应的i结点 */
struct inode* inode_open(struct partition* part, uint32_t inode_no) 
{
   /* 先在已打开inode链表中找inode,此链表是为提速创建的缓冲区 */
    struct list_elem* elem = part->open_inodes.head.next;
    struct inode* inode_found;
    while (elem != &part->open_inodes.tail) 
    {
        inode_found = elem2entry(struct inode, inode_tag, elem);
        if (inode_found->i_no == inode_no) 
        {
	        inode_found->i_open_cnts++;
	        return inode_found;
        }
        elem = elem->next;
    }
    
    /*在已打开的inode列表上没有找到，需要从硬盘上重新加载*/
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);
    
    /*将thread的页目录表置空时，将从内核中分配内存*/
    struct task_struct* cur = running_thread();
    uint32_t* cur_pagedir_bak = cur->pgdir;
    cur->pgdir = NULL;
    /*分配的内存位于内核中*/
    inode_found = (struct inode*)sys_malloc(sizeof(struct inode));
    cur->pgdir = cur_pagedir_bak;/*重置页目录表*/

    char* inode_buf;
    if (inode_pos.two_sec) 
    {	
        /*inode跨扇区*/
        inode_buf = (char*)sys_malloc(1024);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } 
    else 
    {	
        /*inode不跨扇区*/
        inode_buf = (char*)sys_malloc(512);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }

    /*将扇区中合适位置的inode拷贝到inode中*/
    memcpy(inode_found, inode_buf + inode_pos.off_size, sizeof(struct inode));
    
    /*将inode添加到已打开的inode链表中*/
    list_push(&part->open_inodes, &inode_found->inode_tag);
    inode_found->i_open_cnts = 1;

    sys_free(inode_buf);
    return inode_found;
}

/* 关闭inode或减少inode的打开数 */
void inode_close(struct inode* inode) 
{
    enum intr_status old_status = intr_disable();
    /*如果已经没有用户进程使用这个inode，将这个inode从链表中移除*/
    if (--inode->i_open_cnts == 0) 
    {
        list_remove(&inode->inode_tag);	  
        /*确定释放的内存来自于内核内存*/
        struct task_struct* cur = running_thread();
        uint32_t* cur_pagedir_bak = cur->pgdir;
        cur->pgdir = NULL;
        sys_free(inode);
        cur->pgdir = cur_pagedir_bak;
    }
    intr_set_status(old_status);
}

/* 将硬盘分区part上的inode清空 */
void inode_delete(struct partition* part, uint32_t inode_no, void* io_buf) 
{
    ASSERT(inode_no < 4096);
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);/*获取待操作的inode在硬盘上的位置*/
    ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
   
    char* inode_buf = (char*)io_buf;
    if (inode_pos.two_sec) 
    {   
        /*跨扇区，读取，清空，写回*/
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
        memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } 
    else 
    {   
        /*不跨扇区，读取，清空，写回*/
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

/* 回收inode的数据块和inode本身 */
void inode_release(struct partition* part, uint32_t inode_no) 
{
    struct inode* inode_to_del = inode_open(part, inode_no);
    ASSERT(inode_to_del->i_no == inode_no);

    /*回收inode占用的所有块*/
    uint8_t block_idx = 0, block_cnt = 12;
    uint32_t block_bitmap_idx;
    uint32_t all_blocks[140] = {0};	  /*12个直接块+128个间接块*/

    /*先将前12个直接块存入all_blocks*/
    while (block_idx < 12) 
    {
        all_blocks[block_idx] = inode_to_del->i_sectors[block_idx];
        block_idx++;
    }

    /*释放一级间接块占用的空间并将间接块中的间接地址加入all_block*/
    if (inode_to_del->i_sectors[12] != 0) 
    {
        ide_read(part->my_disk, inode_to_del->i_sectors[12], all_blocks + 12, 1);
        block_cnt = 140;

        /*回收一级间接块表占用的扇区*/
        block_bitmap_idx = inode_to_del->i_sectors[12] - part->sb->data_start_lba;/*获取间接块的位图索引*/
        ASSERT(block_bitmap_idx > 0);
        /*同步分区上的位图信息*/
        bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }
   
    /*逐个回收inode占用的内存*/
    block_idx = 0;
    while (block_idx < block_cnt) 
    {
        if (all_blocks[block_idx] != 0) 
        {
	        block_bitmap_idx = 0;
	        block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
	        ASSERT(block_bitmap_idx > 0);
	        bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
        }
        block_idx++; 
    }

    /*回收该inode所占用的inode */
    bitmap_set(&part->inode_bitmap, inode_no, 0);  
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);
    
    /*实际上不需要清零磁盘上inode占用的空间*/
    void* io_buf = sys_malloc(1024);
    inode_delete(part, inode_no, io_buf);
    sys_free(io_buf);
    
    /*在内存链表上关闭inode链表*/
    inode_close(inode_to_del);
}

/* 初始化new_inode */
void inode_init(uint32_t inode_no, struct inode* new_inode) 
{
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = false;

   /* 初始化块索引数组i_sector */
    uint8_t sec_idx = 0;
    while (sec_idx < 13) 
    {
        /*i_sectors[12]为一级间接块地址*/
        new_inode->i_sectors[sec_idx] = 0;
        sec_idx++;
    }
}
