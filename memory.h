/*************************************************************************
	> File Name: memory.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时18分28秒
 ************************************************************************/

#ifndef _MEMORY_H
#define _MEMORY_H

#include"AddressPool.h"
#include"stdint.h"
#include"print.h"
#include"bitmap.h"

namespace memory
{
    static const uint32_t PG_SIZE = 4096;
    static const uint32_t BITMAP_BASE = 0xc0009a00;
    static const uint32_t K_VIR_MEMORY_BASE = 0xc0100000;
    
    #define PG_P_1 1
    #define PG_P_0 0
    #define PG_RW_R 0
    #define PG_RW_W 2
    #define PG_US_S 0 
    #define PG_US_U 4
    
    #define PDE_IDX(addr) ((addr & 0xffc00000) >> 22) 
    #define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

    enum PoolFlags
    {
        PF_KERNEL = 1,
        PF_USER = 2
    };

    struct MemoryMessage
    {
        MemoryMessage(){init();};
        MemoryMessage(const MemoryMessage& mm) = delete;
        MemoryMessage& operator=(const MemoryMessage& mm) = delete;
        ~MemoryMessage(){};

        void init();
        void printMemoryMessage();
            
        uint32_t usedPageTableSize;
        uint32_t usedMemory;
        uint32_t freeMemory;
        uint32_t freePages;

        uint32_t kernelFreePages;
        uint32_t kernelPhyStart;
        uint32_t kernelBitmapBaseAddr;
        uint32_t kernelBitmapLenght;

        uint32_t userFreePages;
        uint32_t userPhyStart;
        uint32_t userBitmapBaseAddr;
        uint32_t userBitmapLenght;
    };

    class KernelMemory final
    {
        public:
            KernelMemory(const MemoryMessage* mm);
            KernelMemory(const KernelMemory& lhs) = delete;
            KernelMemory& operator=(const KernelMemory& lhs) = delete;
            ~KernelMemory(){};
            
        private:
            uint32_t* getVaddrPTE(uint32_t vaddr)
            {
                static uint32_t idx = (0xffc00000 + ((vaddr & 0xfcc00000) >> 10) + PTE_IDX(vaddr) * 4);
                return &idx;
            };
            
            uint32_t* getVaddrPDE();
            {}
        
        private:
            VirtualPool kernelVPool;
            PhysicalPool kernelPPool;
    };

    class UserMemory final
    {
        public:
            UserMemory(const MemoryMessage* mm);
            UserMemory(const UserMemory& lhs) = delete;
            UserMemory& operator=(const UserMemory& lhs) = delete;
            ~UserMemory(){}; 
        
        private:
            PhysicalPool userPPool;
    };
}

#endif
