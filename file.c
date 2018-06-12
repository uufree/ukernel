#include "file.h"
#include "fs.h"
#include "super_block.h"
#include "inode.h"
#include "stdio.h"
#include "memory.h"
#include "debug.h"
#include "interrupt.h"
#include "string.h"
#include "thread.h"
#include "global.h"

#define DEFAULT_SECS 1

/* 文件表 */
struct file file_table[MAX_FILE_OPEN];

/*从文件表file_table中获取一个空闲位,成功返回下标,失败返回-1*/
int32_t get_free_slot_in_global(void) 
{
    uint32_t fd_idx = 3;
    while (fd_idx < MAX_FILE_OPEN) 
    {
        if (file_table[fd_idx].fd_inode == NULL)
	        break;
        fd_idx++;
    }
    
    if (fd_idx == MAX_FILE_OPEN) 
    {
        printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/* 将全局描述符下标安装到进程或线程自己的文件描述符数组fd_table中*/
int32_t pcb_fd_install(int32_t globa_fd_idx) 
{
    struct task_struct* cur = running_thread();
    uint8_t local_fd_idx = 3; // 跨过stdin,stdout,stderr
    while (local_fd_idx < MAX_FILES_OPEN_PER_PROC) 
    {
        if (cur->fd_table[local_fd_idx] == -1) 
        {	// -1表示free_slot,可用
	        cur->fd_table[local_fd_idx] = globa_fd_idx;
	        break;
        }
        local_fd_idx++;
    }
    
    if (local_fd_idx == MAX_FILES_OPEN_PER_PROC) 
    {
        printk("exceed max open files_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/* 分配一个i结点,返回i结点号 */
int32_t inode_bitmap_alloc(struct partition* part) 
{
    int32_t bit_idx = bitmap_scan(&part->inode_bitmap, 1);
    if (bit_idx == -1) 
        return -1;
    bitmap_set(&part->inode_bitmap, bit_idx, 1);
    return bit_idx;
}
   
/* 分配1个扇区,返回其扇区地址 */
int32_t block_bitmap_alloc(struct partition* part) 
{
    int32_t bit_idx = bitmap_scan(&part->block_bitmap, 1);
    if (bit_idx == -1) 
        return -1;
    bitmap_set(&part->block_bitmap, bit_idx, 1);
    return (part->sb->data_start_lba + bit_idx);
} 

/* 将内存中bitmap第bit_idx位所在的512字节同步到硬盘 */
void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp_type) 
{
    uint32_t off_sec = bit_idx / 4096;  
    uint32_t off_size = off_sec * BLOCK_SIZE;  
    uint32_t sec_lba;
    uint8_t* bitmap_off;

    /*需要被同步到硬盘的位图只有inode_bitmap和block_bitmap */
    switch (btmp_type) 
    {
        case INODE_BITMAP:
	        sec_lba = part->sb->inode_bitmap_lba + off_sec;
	        bitmap_off = part->inode_bitmap.bits + off_size;
	        break;
        
        case BLOCK_BITMAP: 
	        sec_lba = part->sb->block_bitmap_lba + off_sec;
	        bitmap_off = part->block_bitmap.bits + off_size;
	        break;
    }

    ide_write(part->my_disk, sec_lba, bitmap_off, 1);
}

/* 创建文件,若成功则返回文件描述符,否则返回-1 */
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag) 
{
    /*后续操作的公共缓冲区*/
    void* io_buf = sys_malloc(1024);
    if (io_buf == NULL) 
    {
        printk("in file_creat: sys_malloc for io_buf failed\n");
        return -1;
    }
    
    /*操作失败时，回滚资源*/
    uint8_t rollback_step = 0;	       

   /* 为新文件分配inode */
    int32_t inode_no = inode_bitmap_alloc(cur_part); 
    if (inode_no == -1) 
    {
        printk("in file_creat: allocate inode failed\n");
        return -1;
    }
    
    /*为inode分配内存*/
    struct inode* new_file_inode = (struct inode*)sys_malloc(sizeof(struct inode)); 
    if (new_file_inode == NULL) 
    {
        printk("file_create: sys_malloc for inode failded\n");
        rollback_step = 1;
        goto rollback;
    }
    inode_init(inode_no, new_file_inode);	    // 初始化i结点

   /* 返回的是file_table数组的下标 */
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1) 
    {
        printk("exceed max open files\n");
        rollback_step = 2;
        goto rollback;
    }
    
    /*在全局中初始化这个struct file*/
    file_table[fd_idx].fd_inode = new_file_inode;
    file_table[fd_idx].fd_pos = 0;
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    struct dir_entry new_dir_entry;
    memset(&new_dir_entry, 0, sizeof(struct dir_entry));
    
    /*为新创建的文件创建一个目录项*/
    create_dir_entry(filename, inode_no, FT_REGULAR, &new_dir_entry);
    
    /*安装这个新创建的目录项*/
    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf)) 
    {
        printk("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    /*将新创建的目录项同步到硬盘*/
    memset(io_buf, 0, 1024);
    inode_sync(cur_part, parent_dir->inode, io_buf);
    
    /*将新创建的inode同步到磁盘*/
    memset(io_buf, 0, 1024);
    inode_sync(cur_part, new_file_inode, io_buf);
    
    /*将新的inode map同步到磁盘*/
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);
    
    /*将新创建的iNode加入打开的inode列表*/
    list_push(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;
    
    /*在用户进程中安装fd*/
    sys_free(io_buf);
    return pcb_fd_install(fd_idx);

/*创建文件需要创建相关的多个资源,若某步失败则会执行到下面的回滚步骤 */
rollback:
    switch (rollback_step) 
    {
        case 3:
	        memset(&file_table[fd_idx], 0, sizeof(struct file)); 
        case 2:
	        sys_free(new_file_inode);
        case 1:
	        bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
	    break;
    }
    sys_free(io_buf);
    return -1;
}

/* 打开编号为inode_no的inode对应的文件,若成功则返回文件描述符,否则返回-1 */
int32_t file_open(uint32_t inode_no, uint8_t flag) 
{
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1) 
    {
        printk("exceed max open files\n");
        return -1;
    }
    file_table[fd_idx].fd_inode = inode_open(cur_part, inode_no);
    file_table[fd_idx].fd_pos = 0;	     
    file_table[fd_idx].fd_flag = flag;
    bool* write_deny = &file_table[fd_idx].fd_inode->write_deny; 
    
    /*判断此时是否有其他的进程在写此文件*/
    if (flag == O_WRONLY || flag == O_RDWR) 
    {	
        enum intr_status old_status = intr_disable();
        if (!(*write_deny)) 
        {    
            /*此时没人写文件，可以使用*/
	        *write_deny = true; 
	        intr_set_status(old_status);	
        } 
        else 
        {	
            /*有人在写文件，直接返回-1*/
	        intr_set_status(old_status);
	        printk("file can`t be write now, try again later\n");
	        return -1;
        }
    }  // 若是读文件或创建文件,不用理会write_deny,保持默认
    return pcb_fd_install(fd_idx);
}

/* 关闭文件 */
int32_t file_close(struct file* file) 
{
    if (file == NULL)
        return -1;

    file->fd_inode->write_deny = false;
    inode_close(file->fd_inode);
    file->fd_inode = NULL;   
    return 0;
}

/* 把buf中的count个字节写入file,成功则返回写入的字节数,失败则返回-1 */
int32_t file_write(struct file* file, const void* buf, uint32_t count) 
{
    if ((file->fd_inode->i_size + count) > (BLOCK_SIZE * 140))	
    {
        printk("exceed max file_size 71680 bytes, write file failed\n");
        return -1;
    }
    
    /*分配一块公共缓冲区供后续使用*/
    uint8_t* io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL) 
    {
        printk("file_write: sys_malloc for io_buf failed\n");
        return -1;
    }
    
    /*用来记录文件所有的块地址*/
    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE + 48);
    if (all_blocks == NULL) 
    {
        printk("file_write: sys_malloc for all_blocks failed\n");
        return -1;
    }

    const uint8_t* src = buf;       /*待写入数据地址*/ 
    uint32_t bytes_written = 0;	    /*已写入数据量*/ 
    uint32_t size_left = count;	    /*未写入数据量*/
    int32_t block_lba = -1;	        /*块地址*/
    uint32_t block_bitmap_idx = 0;  /*记录在未使用位图中的索引*/
    uint32_t sec_idx;	            /*用来索引扇区*/
    uint32_t sec_lba;	            /*扇区地址*/
    uint32_t sec_off_bytes;         /*扇区内字节偏移量*/
    uint32_t sec_left_bytes;        /*扇区内剩余字节量*/
    uint32_t chunk_size;	        /*每次写入硬盘的数据块大小*/
    int32_t indirect_block_table;   /*用来获取一级间接表地址*/
    uint32_t block_idx;		        /*块索引*/

    /* 判断文件是否是第一次写,如果是,先为其分配一个块 */
    if (file->fd_inode->i_sectors[0] == 0) 
    {
        block_lba = block_bitmap_alloc(cur_part);
        if (block_lba == -1) 
        {
	        printk("file_write: block_bitmap_alloc failed\n");
	        return -1;
        }
        file->fd_inode->i_sectors[0] = block_lba;

        /*将这个新分配的块同步到位图中*/
        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx != 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }

    /*写入count个字节前,该文件已经占用的块数*/
    uint32_t file_has_used_blocks = file->fd_inode->i_size / BLOCK_SIZE + 1;

    /*存储count字节后该文件将占用的块数*/
    uint32_t file_will_use_blocks = (file->fd_inode->i_size + count) / BLOCK_SIZE + 1;
    ASSERT(file_will_use_blocks <= 140);

    /*通过此增量判断是否需要分配扇区,如增量为0,表示原扇区够用*/
    uint32_t add_blocks = file_will_use_blocks - file_has_used_blocks;

    /*将写文件所用到的块地址收集到all_blocks,(系统中块大小等于扇区大小)*/
    if (add_blocks == 0) 
    { 
        /*在同一扇区内写入数据,不涉及到分配新扇区 */
        if (file_has_used_blocks <= 12 ) 
        {	
            /*文件的数据量在12个直接块之内*/
	        block_idx = file_has_used_blocks - 1;
	        all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        } 
        else 
        { 
            /*未写入新数据之前已经占用了间接块,需要将间接块地址读进来*/
	        ASSERT(file->fd_inode->i_sectors[12] != 0);
            indirect_block_table = file->fd_inode->i_sectors[12];
	        ide_read(cur_part->my_disk,indirect_block_table,all_blocks + 12, 1);
        }
    } 
    else 
    {
        /*12个直接块够用，无需分配间接表*/
        if (file_will_use_blocks <= 12 ) 
        {
            /*先将有剩余空间的可继续用的扇区地址写入all_blocks*/
	        block_idx = file_has_used_blocks - 1;
	        ASSERT(file->fd_inode->i_sectors[block_idx] != 0);
	        all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            /*将未来要用的扇区分配好后写入all_blocks */
	        block_idx = file_has_used_blocks;   
	        while (block_idx < file_will_use_blocks) 
            {
	            block_lba = block_bitmap_alloc(cur_part);
	            if (block_lba == -1) 
                {
	                printk("file_write: block_bitmap_alloc for situation 1 failed\n");
	                return -1;
	            }

	            ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
                /*在文件表项中填充新分配的块地址索引*/
	            file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
	            /*将新分配的块同步到磁盘上*/
	            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
	            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

	            block_idx++;   
	        }
        } 
        else if (file_has_used_blocks <= 12 && file_will_use_blocks > 12) 
        { 
	        /*旧数据在12个直接块内,新数据将使用间接块*/
            
            /*先收集直接块内的最后一个直接块*/
	        block_idx = file_has_used_blocks - 1;   
	        all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

	        /*创建一级间接块表*/
	        block_lba = block_bitmap_alloc(cur_part);
	        if (block_lba == -1) 
            {
	            printk("file_write: block_bitmap_alloc for situation 2 failed\n");
	            return -1;
	        }

	        ASSERT(file->fd_inode->i_sectors[12] == 0);  
	        /*分配一级间接块索引表 */
	        indirect_block_table = file->fd_inode->i_sectors[12] = block_lba;
	        block_idx = file_has_used_blocks;	
	        while (block_idx < file_will_use_blocks) 
            {
                /*分配一个间接块中的数据块*/
	            block_lba = block_bitmap_alloc(cur_part);
	            if (block_lba == -1) 
                {
	                printk("file_write: block_bitmap_alloc for situation 2 failed\n");
	                return -1;
	            }

	            if (block_idx < 12) 
                {      
                    /*新分配的直接块直接填入文件数组中，暂时不同步*/
	                ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
	                file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
	            } 
                else 
                {     
                    /*间接块的同步在稍后分配完成后统一同步*/
	                all_blocks[block_idx] = block_lba;
	            }

	            /*但是新分配的间接块指向的数据块的位图需要及时同步*/
	            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
	            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

	            block_idx++;
	        }
	        
            /*将间接块中的数据块的地址同步到磁盘中*/
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);      
        } 
        else if (file_has_used_blocks > 12) 
        {
	        /*新数据占据间接块*/
	        ASSERT(file->fd_inode->i_sectors[12] != 0);
	        /*获取间接块的地址*/
            indirect_block_table = file->fd_inode->i_sectors[12];

	        /*将间接块中的数据内容读取到内存中，即获取间接块中的数据块地址*/
	        ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1); 
	        block_idx = file_has_used_blocks;/*第一个未使用的间接块*/
	        while (block_idx < file_will_use_blocks) 
            {
                /*为即将存储数据的数据块分配扇区*/
	            block_lba = block_bitmap_alloc(cur_part);
	            if (block_lba == -1) 
                {
	                printk("file_write: block_bitmap_alloc for situation 3 failed\n");
	                return -1;
	            }
	            all_blocks[block_idx++] = block_lba;

	            /*每分配一个数据块扇区立即同步位图*/
	            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
	            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
	        }
            /*将间接块中的修改同步到间接块*/
	        ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);   
        } 
    }

   /*用到的块地址已经收集到all_blocks中,下面开始写数据*/
    bool first_write_block = true;      
    file->fd_pos = file->fd_inode->i_size - 1;   /*即时更新文件位置*/
    while (bytes_written < count) 
    {   
        memset(io_buf, 0, BLOCK_SIZE);
        sec_idx = file->fd_inode->i_size / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        /*若之前的数据区不干净，需要先读出数据然后再写*/
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
        if (first_write_block) 
        {
	        ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
	        first_write_block = false;
        }
        memcpy(io_buf + sec_off_bytes, src, chunk_size);
        ide_write(cur_part->my_disk, sec_lba, io_buf, 1);

        src += chunk_size;   /*调整文件偏移*/
        file->fd_inode->i_size += chunk_size;  /*更新文件大小*/
        file->fd_pos += chunk_size;   
        bytes_written += chunk_size;
        size_left -= chunk_size;
    }
    /*同步inode中的信息到磁盘上*/
    inode_sync(cur_part, file->fd_inode, io_buf);
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_written;
}

/* 从文件file中读取count个字节写入buf, 返回读出的字节数,若到文件尾则返回-1 */
int32_t file_read(struct file* file, void* buf, uint32_t count) 
{
    uint8_t* buf_dst = (uint8_t*)buf;
    uint32_t size = count, size_left = size;
    
    /*根据文件的偏移量判断文件的可读数据量*/
    if ((file->fd_pos + count) > file->fd_inode->i_size)	
    {
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if (size == 0) 	
	        return -1;
    }

    uint8_t* io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL) 
        printk("file_read: sys_malloc for io_buf failed\n");
   
    /*记录所有文件需要操作的扇区地址*/
    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE + 48);	
    if (all_blocks == NULL) 
    {
        printk("file_read: sys_malloc for all_blocks failed\n");
        return -1;
    }

    /*可读数据块的起始索引*/
    uint32_t block_read_start_idx = file->fd_pos / BLOCK_SIZE;
    /*可读数据块的终止索引*/
    uint32_t block_read_end_idx = (file->fd_pos + size) / BLOCK_SIZE;	
    /*记录可读数据块的数量*/
    uint32_t read_blocks = block_read_start_idx - block_read_end_idx;	       
    ASSERT(block_read_start_idx < 139 && block_read_end_idx < 139);

    int32_t indirect_block_table;   /*获取一级间接表的索引*/
    uint32_t block_idx;		       

    if (read_blocks == 0) 
    {       
        /*若在同一扇区内读取数据即可*/
        ASSERT(block_read_end_idx == block_read_start_idx);
        if (block_read_end_idx < 12 ) 
        {	   
            /*待读的数据在12个直接块之内*/
	        block_idx = block_read_end_idx;
	        all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        } 
        else 
        {		
            /*若用到了一级间接块表,需要将表中间接块读进来*/
	        indirect_block_table = file->fd_inode->i_sectors[12];
	        ide_read(cur_part->my_disk,indirect_block_table,all_blocks + 12, 1);
        }
    } 
    else 
    {      
        /*起始块和终止块属于直接块*/
        if (block_read_end_idx < 12 ) 
        {	
	        block_idx = block_read_start_idx; 
	        while (block_idx <= block_read_end_idx) 
            {
	            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx]; 
	            block_idx++;
	        }
        } 
        else if (block_read_start_idx < 12 && block_read_end_idx >= 12) 
        {
            /*待读入的数据跨越直接块和间接块两类*/
	        block_idx = block_read_start_idx;
	        while (block_idx < 12) 
            {
	            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
	            block_idx++;
	        }
	        ASSERT(file->fd_inode->i_sectors[12] != 0);	    

            /*将间接块地址写入all_blocks*/
	        indirect_block_table = file->fd_inode->i_sectors[12];
	        ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);	
        } 
        else 
        {	
            /*数据在间接块中*/
	        ASSERT(file->fd_inode->i_sectors[12] != 0);
	        indirect_block_table = file->fd_inode->i_sectors[12];	 
            /*间接块中的数据就是uint32_t类型的数据，可以直接加入数据，无需转换*/
	        ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);	    
        } 
    }

    /*用到的块地址已经收集到all_blocks中,下面开始读数据 */
    uint32_t sec_idx, sec_lba, sec_off_bytes, sec_left_bytes, chunk_size;
    uint32_t bytes_read = 0;
    while (bytes_read < size) 
    {	      
        sec_idx = file->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_pos % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;	    

        memset(io_buf, 0, BLOCK_SIZE);
        ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
        memcpy(buf_dst, io_buf + sec_off_bytes, chunk_size);

        buf_dst += chunk_size;
        file->fd_pos += chunk_size;
        bytes_read += chunk_size;
        size_left -= chunk_size;
    }
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_read;
}
