#include "dir.h"
#include "stdint.h"
#include "inode.h"
#include "file.h"
#include "fs.h"
#include "stdio.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "string.h"
#include "interrupt.h"
#include "super_block.h"

struct dir root_dir;             /*根目录*/

/*打开根目录*/
void open_root_dir(struct partition* part) 
{
    root_dir.inode = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0;
}

/*在分区part上打开i结点为inode_no的目录并返回目录指针*/
struct dir* dir_open(struct partition* part, uint32_t inode_no) 
{
    struct dir* pdir = (struct dir*)sys_malloc(sizeof(struct dir));
    pdir->inode = inode_open(part, inode_no);
    pdir->dir_pos = 0;
    return pdir;
}

/*在part分区内的pdir目录内寻找名为name的文件或目录*/
bool search_dir_entry(struct partition* part, struct dir* pdir, const char* name, struct dir_entry* dir_e) 
{
    /*目录项中的数据可能占据140个扇区*/
    uint32_t block_cnt = 140;	 

    /*12个直接块大小+128个间接块,共560字节*/
    uint32_t* all_blocks = (uint32_t*)sys_malloc(48 + 512);
    if (all_blocks == NULL) 
    {
        printk("search_dir_entry: sys_malloc for all_blocks failed");
        return false;
    }

    uint32_t block_idx = 0;
    while (block_idx < 12) 
    {
        all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
        block_idx++;
    }
    block_idx = 0;

    if (pdir->inode->i_sectors[12] != 0) 
        ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);

    /*目录项不跨扇区*/
    uint8_t* buf = (uint8_t*)sys_malloc(SECTOR_SIZE);
    struct dir_entry* p_de = (struct dir_entry*)buf;	    
    /*目录项的大小*/
    uint32_t dir_entry_size = part->sb->dir_entry_size;
    /*一个扇区中可以容纳的目录项数量*/
    uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;  

    /*开始在所有块中查找目录项*/
    while (block_idx < block_cnt) 
    {		  
        /*块地址为0时表示该块中无数据,继续在其它块中找*/
        if (all_blocks[block_idx] == 0) 
        {
	        block_idx++;
	        continue;
        }
        /*读取一个扇区的数据*/
        ide_read(part->my_disk, all_blocks[block_idx], buf, 1);

        uint32_t dir_entry_idx = 0;
        /*遍历扇区中所有目录项*/
        while (dir_entry_idx < dir_entry_cnt) 
        {
	        /*若找到了,就直接复制整个目录项 */
	        if (!strcmp(p_de->filename, name)) 
            {
	            memcpy(dir_e, p_de, dir_entry_size);
	            sys_free(buf);
	            sys_free(all_blocks);
	            return true;
	        }
	        dir_entry_idx++;
	        p_de++;
        }
        block_idx++;
        /*重新从磁盘中读取扇区*/
        p_de = (struct dir_entry*)buf;  
        memset(buf, 0, SECTOR_SIZE);	  
    }
    
    sys_free(buf);
    sys_free(all_blocks);
    return false;
}

/* 关闭目录 */
void dir_close(struct dir* dir) 
{
    /*根目录不能关闭*/
    if (dir == &root_dir) 
        return;
    inode_close(dir->inode);
    sys_free(dir);
}

/* 在内存中初始化目录项p_de */
void create_dir_entry(char* filename, uint32_t inode_no, uint8_t file_type, struct dir_entry* p_de) 
{
    ASSERT(strlen(filename) <=  MAX_FILE_NAME_LEN);

    /*初始化目录项*/
    memcpy(p_de->filename, filename, strlen(filename));
    p_de->i_no = inode_no;
    p_de->f_type = file_type;
}

/*将目录项p_de写入父目录parent_dir中,io_buf由主调函数提供 */
bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de, void* io_buf) 
{
    struct inode* dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    ASSERT(dir_size % dir_entry_size == 0);	 
    
    /*计算每个扇区的目录项数量*/
    uint32_t dir_entrys_per_sec = (512 / dir_entry_size);       
    int32_t block_lba = -1;

    /* 将该目录的所有扇区地址(12个直接块+ 128个间接块)存入all_blocks*/
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0};	  

    while (block_idx < 12) 
    {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }

    struct dir_entry* dir_e = (struct dir_entry*)io_buf;
    int32_t block_bitmap_idx = -1;
    
    block_idx = 0;
    while (block_idx < 140) 
    {  
        /*文件(包括目录)最大支持12个直接块+128个间接块＝140个块*/
        block_bitmap_idx = -1;
        if (all_blocks[block_idx] == 0) 
        {   
            /*目录项为空，分配存储目录项的数据块*/
	        block_lba = block_bitmap_alloc(cur_part);
	        if (block_lba == -1) 
            {
	            printk("alloc block bitmap for sync_dir_entry failed\n");
	            return false;
	        }
        
            /*在分配数据块之后立刻进行同步操作*/
	        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
	        ASSERT(block_bitmap_idx != -1);
	        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

	        block_bitmap_idx = -1;
            /*分配数据块之后在文件中更新扇区地址*/
	        if (block_idx < 12) 
	            dir_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
            else if (block_idx == 12) 
            {	  
                /*若需要用到间接地址，将刚才分配的数据块当做间接块使用*/
	            dir_inode->i_sectors[12] = block_lba;   
	            block_lba = -1;
                /*重新分配一个扇区用作第一个间接数据块*/
	            block_lba = block_bitmap_alloc(cur_part);	
	            if (block_lba == -1) 
                {
	                block_bitmap_idx = dir_inode->i_sectors[12] - cur_part->sb->data_start_lba;
	                bitmap_set(&cur_part->block_bitmap, block_bitmap_idx, 0);
	                dir_inode->i_sectors[12] = 0;
	                printk("alloc block bitmap for sync_dir_entry failed\n");
	                return false;
	            }

	            /*同步刚刚分配的间接数据块*/
	            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
	            ASSERT(block_bitmap_idx != -1);
	            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                
                /*将新分配的数据地址写入第一级间接块*/
	            all_blocks[12] = block_lba;
	            ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
	        } 
            else 
            {	   
                /*间接块已分配，将新分配的间接块的地址写入间接表*/
	            all_blocks[block_idx] = block_lba;
	            ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
	        }

            /*因为是新分配的间接块，说明以前的数据块中不存在目录项*/
	        /*将目录项写入即可*/
	        memset(io_buf, 0, 512);
	        memcpy(io_buf, p_de, dir_entry_size);
	        ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
	        dir_inode->i_size += dir_entry_size;
	        return true;
        }

        /*若第block_idx块已存在,将其读进内存,然后在该块中查找空目录项*/
        ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1); 
        uint8_t dir_entry_idx = 0;
        while (dir_entry_idx < dir_entrys_per_sec) 
        {
	        if ((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN) 
            {
	            memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);    
	            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);

	            dir_inode->i_size += dir_entry_size;
	            return true;
	        }
	        dir_entry_idx++;
        }
        block_idx++;
    }

    printk("directory is full!\n");
    return false;
}

/* 把分区part目录pdir中编号为inode_no的目录项删除 */
bool delete_dir_entry(struct partition* part, struct dir* pdir, uint32_t inode_no, void* io_buf) 
{
    struct inode* dir_inode = pdir->inode;
    uint32_t block_idx = 0, all_blocks[140] = {0};
    while (block_idx < 12) 
    {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }
    
    /*如果间接块存在，将间接块中的地址全部存入*/
    if (dir_inode->i_sectors[12])
        ide_read(part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);

    /*目录项在存储时保证不会跨扇区*/
    uint32_t dir_entry_size = part->sb->dir_entry_size;
    /*每个扇区中可以存储的最大的目录项个数*/
    uint32_t dir_entrys_per_sec = (SECTOR_SIZE / dir_entry_size);   
    struct dir_entry* dir_e = (struct dir_entry*)io_buf;   
    struct dir_entry* dir_entry_found = NULL;
    uint8_t dir_entry_idx, dir_entry_cnt;
    bool is_dir_first_block = false;      

    /*遍历所有块,寻找目录项*/
    block_idx = 0;
    while (block_idx < 140) 
    {
        /*如果当前的扇区为空，继续查找*/
        is_dir_first_block = false;
        if(all_blocks[block_idx] == 0) 
        {
	        block_idx++;
	        continue;
        }
        dir_entry_idx = dir_entry_cnt = 0;
        memset(io_buf, 0, SECTOR_SIZE);
        /*读取扇区,获得目录项*/
        ide_read(part->my_disk, all_blocks[block_idx], io_buf, 1);

        /*遍历所有的目录项,统计该扇区的目录项数量及是否有待删除的目录项*/
        while (dir_entry_idx < dir_entrys_per_sec) 
        {
	        if ((dir_e + dir_entry_idx)->f_type != FT_UNKNOWN) 
            {
                /*意味着这是一个有效的目录项*/
	            if (!strcmp((dir_e + dir_entry_idx)->filename, ".")) 
	                is_dir_first_block = true; 
                else if (strcmp((dir_e + dir_entry_idx)->filename, ".") && 
	       strcmp((dir_e + dir_entry_idx)->filename, "..")) 
                {
	                dir_entry_cnt++;     // 统计此扇区内的目录项个数,用来判断删除目录项后是否回收该扇区
	                if ((dir_e + dir_entry_idx)->i_no == inode_no) 
                    {	  
                        /*如果找到了待删除的目录项，记录下来*/
		                ASSERT(dir_entry_found == NULL);  
		                dir_entry_found = dir_e + dir_entry_idx;
	                }
	            }
	        }
	        dir_entry_idx++;
        } 

        /*若此扇区未找到该目录项,继续在下个扇区中找*/
        if (dir_entry_found == NULL) 
        {
	        block_idx++;
	        continue;
        }

        /*在此扇区中找到目录项后,清除该目录项并判断是否回收扇区*/
        ASSERT(dir_entry_cnt >= 1);
        /*除目录第1个扇区外,若该扇区上只有该目录项自己,则将整个扇区回收*/
        if (dir_entry_cnt == 1 && !is_dir_first_block) 
        {
	        /*在块位图中回收该块*/
	        uint32_t block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
	        bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

	        /*将块地址从数组i_sectors或索引表中去掉*/
	        if (block_idx < 12) 
	            dir_inode->i_sectors[block_idx] = 0;
            else 
            {    
                /*回收间接地址*/
	            uint32_t indirect_blocks = 0;
	            uint32_t indirect_block_idx = 12;
	            while (indirect_block_idx < 140) 
	                if (all_blocks[indirect_block_idx] != 0) 
		                indirect_blocks++;
	            
                ASSERT(indirect_blocks >= 1);  // 包括当前间接块

	            if (indirect_blocks > 1) 
                {	
	                /*间接索引表中还包含其他块，只擦除当前块*/
                    all_blocks[block_idx] = 0; 
	                ide_write(part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1); 
	            } 
                else 
                {
	                /*间接索引表与间接数据块中都已经没有数据，同时回收*/
	                block_bitmap_idx = dir_inode->i_sectors[12] - part->sb->data_start_lba;
	                bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
	       
	                /*将间接索引表地址清0 */
	                dir_inode->i_sectors[12] = 0;
	            }
	        }
        } 
        else 
        { 
            /*仅将当年前的目录项清空即可*/
	        memset(dir_entry_found, 0, dir_entry_size);
	        ide_write(part->my_disk, all_blocks[block_idx], io_buf, 1);
        }

        /*更新i结点信息并同步到硬盘*/
        ASSERT(dir_inode->i_size >= dir_entry_size);
        dir_inode->i_size -= dir_entry_size;
        memset(io_buf, 0, SECTOR_SIZE * 2);
        inode_sync(part, dir_inode, io_buf);

        return true;
    }
    
    return false;
}

/* 读取目录,成功返回1个目录项,失败返回NULL */
struct dir_entry* dir_read(struct dir* dir) 
{
    struct dir_entry* dir_e = (struct dir_entry*)dir->dir_buf;
    struct inode* dir_inode = dir->inode; 
    uint32_t all_blocks[140] = {0}, block_cnt = 12;
    uint32_t block_idx = 0, dir_entry_idx = 0;
    
    /*保存页目录中所有扇区的编址*/
    while (block_idx < 12) 
    {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }
    
    if (dir_inode->i_sectors[12] != 0) 
    {	     // 若含有一级间接块表
        ide_read(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
        block_cnt = 140;
    }
    
    block_idx = 0;
    /*记录当前目录项的偏移*/
    uint32_t cur_dir_entry_pos = 0;	
    /*每个目录项的大小*/
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
    /*每个扇区中目录项的个数*/
    uint32_t dir_entrys_per_sec = SECTOR_SIZE / dir_entry_size;	 

    /*因为此目录内可能删除了某些文件或子目录,所以要遍历所有块 */
    while (block_idx < block_cnt) 
    {
        if (dir->dir_pos >= dir_inode->i_size) 
	        return NULL;
        
        if (all_blocks[block_idx] == 0) 
        {   
            /*如果这个目录项数据块为空*/
	        block_idx++;
	        continue;
        }

        /*从目录项数据块的地址中读取一个扇区的数据*/
        memset(dir_e, 0, SECTOR_SIZE);
        ide_read(cur_part->my_disk, all_blocks[block_idx], dir_e, 1);
        dir_entry_idx = 0;

        /*遍历扇区内所有目录项*/
        while (dir_entry_idx < dir_entrys_per_sec) 
        {
	        if ((dir_e + dir_entry_idx)->f_type) 
            {	 
	            /*该目录项的位置已知，file或者dir*/    
	            if (cur_dir_entry_pos < dir->dir_pos) 
                {
	                cur_dir_entry_pos += dir_entry_size;
	                dir_entry_idx++;
	                continue;
	            }
	            ASSERT(cur_dir_entry_pos == dir->dir_pos);
                /*更新偏移位置并返回目录项地址*/
	            dir->dir_pos += dir_entry_size;	      
	            return dir_e + dir_entry_idx; 
	        }
	        dir_entry_idx++;
        }
        block_idx++;
    }
    return NULL;
}

/* 判断目录是否为空 */
bool dir_is_empty(struct dir* dir) 
{
    struct inode* dir_inode = dir->inode;
    return (dir_inode->i_size == cur_part->sb->dir_entry_size * 2);
}

/* 在父目录parent_dir中删除child_dir */
int32_t dir_remove(struct dir* parent_dir, struct dir* child_dir) 
{
    struct inode* child_dir_inode  = child_dir->inode;
    int32_t block_idx = 1;
    while (block_idx < 13) 
    {
        ASSERT(child_dir_inode->i_sectors[block_idx] == 0);
        block_idx++;
    }
    
    void* io_buf = sys_malloc(SECTOR_SIZE * 2);
    if (io_buf == NULL) 
    {
        printk("dir_remove: malloc for io_buf failed\n");
        return -1;
    }

    /*在父目录parent_dir中删除子目录child_dir对应的目录项*/
    delete_dir_entry(cur_part, parent_dir, child_dir_inode->i_no, io_buf);

    /*回收inode中i_secotrs中所占用的扇区,并同步inode_bitmap和block_bitmap*/
    inode_release(cur_part, child_dir_inode->i_no);
    sys_free(io_buf);
    return 0;
}
