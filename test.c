/*************************************************************************
	> File Name: test.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时30分20秒
 ************************************************************************/

#include<stdio.h>
#include"string.h"

int main(int argc,char** argv)
{
    char name[10];
    char name1[10];
    for(int i=0;i<9;++i)
    {
        name[i] = 'u';
        name1[i] = 'c';
    }
    name[9] = '\0';
    
    printf("name: %s\n",name);
    
//    memset((void*)name,'k',10);
//    memcpy((void*)name,(void*)name1,10);
//    uint8_t o1 = memcmp((void*)name,(void*)name1,10);

//    strcpy(name,name1);
//    uint32_t len = strlen(name);
//    uint8_t o2 = strcmp(name,name1);
    strcat(name,name1);
    
//    printf("bool: %d\n",len);
    printf("name: %s\n",name);

    return 0;

}

