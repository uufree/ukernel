/*************************************************************************
	> File Name: stringTest.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月07日 星期四 16时04分10秒
 ************************************************************************/

#include"string.h"
#include<stdio.h>

int main(int argc,char** argv)
{
    const char* str = "uuchen";
    const char* str1 = "chenuu";
    char name[10];
    
    uint32_t len = strlen(str);

    printf("%d\n",len);    

    return 0;
}


