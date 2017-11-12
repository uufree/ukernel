/*************************************************************************
	> File Name: test.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时30分20秒
 ************************************************************************/

#include<stdio.h>
#include"string.h"
#include"bitmap.h"

int getByte(uint8_t byte,uint8_t pos)
{
    if(pos > 7 || pos < 0)
        return -1;
    
    uint8_t uu;
    switch (pos)
    {
        case 0:
            uu = 0x01;
            break;
        case 1:
            uu = 0x02;
            break;
        case 2:
            uu = 0x04;
            break;
        case 3:
            uu = 0x08;
            break;
        case 4:
            uu = 0x10;
            break;
        case 5:
            uu = 0x20;
            break;
        case 6:
            uu = 0x40;
            break;
        case 7:
            uu = 0x80;
            break;
    }
    
    if(uu & byte)
        return 1;
    
    return 0;
}

int main(int argc,char** argv)
{
    char name[8];
    for(int i=0;i<8;++i)
        name[i] = 'm';
    
    printf("Init Before: %s\n",name);

    struct Bitmap map;
    bitmapInit(&map,name,8,4096);
    bitmapSetPos(&map,0);
    bitmapSetPos(&map,4);
    bitmapSetPos(&map,5);
    
    int num;
    for(int i=0;i<8;++i)
    {
        num = getByte(name[0],i);
        printf("num: %d\n",num);
    }
    
    printf("Init After: %s\n",name);

    return 0;
}

