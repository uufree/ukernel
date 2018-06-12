#include "shell.h"
#include "stdint.h"
#include "file.h"
#include "syscall.h"
#include "stdio.h"
#include "global.h"
#include "assert.h"
#include "string.h"
#include "buildin_cmd.h"
#include "pipe.h"

#define MAX_ARG_NR 16	   /*加上命令名外,最多支持15个参数*/

/*存储输入的命令*/
static char cmd_line[MAX_PATH_LEN] = {0};
char final_path[MAX_PATH_LEN] = {0};      /*路径的缓冲*/

/*用来记录当前目录,是当前目录的缓存,每次执行cd命令时会更新此内容*/
char cwd_cache[MAX_PATH_LEN] = {0};

/*输出提示符*/
void print_prompt(void) 
{
   printf("[uuchen@localhost %s]$ ", cwd_cache);
}

/*从键盘缓冲区中最多读入count个字节到buf*/
static void readline(char* buf, int32_t count) 
{
    assert(buf != NULL && count > 0);
    char* pos = buf;

    while (read(stdin_no, pos, 1) != -1 && (pos - buf) < count) 
    { 
        /*在不出错情况下,直到找到回车符才返回*/
        switch (*pos) 
        {
            /*找到回车或换行符后认为键入的命令结束,直接返回*/
	        case '\n':
	        case '\r':
	            *pos = 0;	   /*添加cmd_line的终止字符0*/
	            putchar('\n');
	            return;

	        case '\b':
	            if (cmd_line[0] != '\b') 
                {		
                    /*删除缓冲区中的一个字符并且在屏幕上显示回退的效果*/
	                --pos;	   
	                putchar('\b');
	            }
	            break;
            
	        case 'l' - 'a': 
                /*crtl+l，清零*/
	            *pos = 0;
	            clear();
	            print_prompt();
	            printf("%s", buf);
	            break;

	        case 'u' - 'a':
                /*ctrl+u，清除输入*/
	            while (buf != pos) 
                {
	                putchar('\b');
	                *(pos--) = 0;
	            }
	            break;

	        default:
                /*非控制键输出字符*/
	            putchar(*pos);
	            pos++;
        }
    }
    printf("readline: read max char is 128!");
}

/*分析字符串cmd_str中以空格为分隔符的单词,将各单词的指针存入argv数组*/
static int32_t cmd_parse(char* cmd_str, char** argv, char token) 
{
    assert(cmd_str != NULL);
    int32_t arg_idx = 0;
    while(arg_idx < MAX_ARG_NR) 
    {
        argv[arg_idx] = NULL;
        arg_idx++;
    }
    char* next = cmd_str;
    int32_t argc = 0;
    while(*next) 
    {
        /*去除命令字或参数之间的空格*/
        while(*next == token) 
	        next++;

        /*处理最后一个参数后接空格的情况*/
        if (*next == 0) 
	        break; 
        argv[argc] = next;

        /*在字符串结束前找单词分隔符*/
        while (*next && *next != token) 
	        next++;

        /*如果未结束(是token字符),使tocken变成0*/
        if (*next) 
	        *next++ = 0;	
   
        /*避免argv数组访问越界,参数过多则返回0*/
        if (argc > MAX_ARG_NR) 
	        return -1;
        argc++;
    }
    return argc;
}

/* 执行命令 */
static void cmd_execute(uint32_t argc, char** argv) 
{
    if (!strcmp("ls", argv[0])) 
        buildin_ls(argc, argv);
    else if (!strcmp("cd", argv[0])) 
    {
        if (buildin_cd(argc, argv) != NULL) 
        {
	        memset(cwd_cache, 0, MAX_PATH_LEN);
	        strcpy(cwd_cache, final_path);
        }
    } 
    else if (!strcmp("pwd", argv[0])) 
        buildin_pwd(argc, argv);
    else if (!strcmp("ps", argv[0])) 
        buildin_ps(argc, argv);
    else if (!strcmp("clear", argv[0])) 
        buildin_clear(argc, argv);
    else if (!strcmp("mkdir", argv[0]))
        buildin_mkdir(argc, argv);
    else if (!strcmp("rmdir", argv[0]))
        buildin_rmdir(argc, argv);
    else if (!strcmp("rm", argv[0])) 
        buildin_rm(argc, argv);
    else if (!strcmp("help", argv[0])) 
        buildin_help(argc, argv);
    else 
    {      
        /*如果是外部命令,需要从磁盘上加载*/
        int32_t pid = fork();
        if (pid) 
        {	   
            /*父进程*/
	        int32_t status;
	        int32_t child_pid = wait(&status);/*父进程在此阻塞*/
	        if (child_pid == -1) 
	            panic("my_shell: no child\n");
	        printf("child_pid %d, it's status: %d\n", child_pid, status);
        } 
        else 
        {	   
            /*子进程*/
	        make_clear_abs_path(argv[0], final_path);
	        argv[0] = final_path;

	        /*先判断下文件是否存在*/
	        struct stat file_stat;
	        memset(&file_stat, 0, sizeof(struct stat));
	        if (stat(argv[0], &file_stat) == -1) 
            {
	            printf("my_shell: cannot access %s: No such file or directory\n", argv[0]);
	            exit(-1);
	        } 
            else 
	            execv(argv[0], argv);
        }
    }
}

/*命令单词数组*/
char* argv[MAX_ARG_NR] = {NULL};
int32_t argc = -1;
/*简单的shell*/
void my_shell(void) 
{
    cwd_cache[0] = '/';
    while (1) 
    {
        print_prompt(); 
        memset(final_path, 0, MAX_PATH_LEN);
        memset(cmd_line, 0, MAX_PATH_LEN);
        readline(cmd_line, MAX_PATH_LEN);
        if (cmd_line[0] == 0) 
	        continue;

        /*针对管道的处理*/
        char* pipe_symbol = strchr(cmd_line, '|');
        if (pipe_symbol) 
        {
	        int32_t fd[2] = {-1};	    /*fd[0]用于输入,fd[1]用于输出*/
	        pipe(fd);
	        fd_redirect(1,fd[1]);/*将标准输出重定向到管道输出端口*/

	        char* each_cmd = cmd_line;
	        pipe_symbol = strchr(each_cmd, '|');
	        *pipe_symbol = 0;

	        /*执行第一个命令,命令的输出会写入环形缓冲区*/
	        argc = -1;
	        argc = cmd_parse(each_cmd, argv, ' ');
	        cmd_execute(argc, argv);

	        each_cmd = pipe_symbol + 1;

	        /*将标准输入重定向到fd[0],使之指向内核环形缓冲区*/
	        fd_redirect(0,fd[0]);
	        while ((pipe_symbol = strchr(each_cmd, '|'))) 
            { 
	            *pipe_symbol = 0;
	            argc = -1;
	            argc = cmd_parse(each_cmd, argv, ' ');
	            cmd_execute(argc, argv);
	            each_cmd = pipe_symbol + 1;
	        }

	        /*将标准输出恢复屏幕*/
            fd_redirect(1,1);

	        /*执行最后一个命令*/
	        argc = -1;
	        argc = cmd_parse(each_cmd, argv, ' ');
	        cmd_execute(argc, argv);

            /*将标准输入恢复为键盘*/
            fd_redirect(0,0);

            /*关闭管道*/
	        close(fd[0]);
	        close(fd[1]);
        } 
        else 
        {		
            /*无管道操作的命令*/
	        argc = -1;
	        argc = cmd_parse(cmd_line, argv, ' ');
	        if (argc == -1) 
            {
	            printf("num of arguments exceed %d\n", MAX_ARG_NR);
	            continue;
	        }
	        cmd_execute(argc, argv);
        }
    }
    panic("my_shell: should not be here");
}
