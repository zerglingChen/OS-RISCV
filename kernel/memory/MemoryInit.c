#include <Page.h>
#include <Driver.h>
#include <MemoryConfig.h>
#include <Riscv.h>
#include <Platform.h>
#include <Process.h>
#include "Spinlock.h"

PageList freePages;
PhysicalPage pages[PHYSICAL_PAGE_NUM];
extern char kernelStart[];
extern char kernelEnd[];
extern u64 kernelPageDirectory[];

static void initFreePages() {
    u64 i;
    u64 n = PA2PPN(kernelEnd);
    for (i = 0; i < n; i++) {
        pages[i].ref = 1;
    }

    n = PA2PPN(PHYSICAL_MEMORY_TOP);
    LIST_INIT(&freePages);
    for (; i < n; i++) {
        pages[i].ref = 0;
        LIST_INSERT_HEAD(&freePages, &pages[i], link);
    }
}

static void resetRef() {
    u64 i = PA2PPN(kernelEnd), n = PA2PPN(PHYSICAL_MEMORY_TOP);
    for (; i < n; i++) {
        pages[i].ref = 0;
    }
}

static void virtualMemory() {
    u64 va, pa;
    pageInsert(kernelPageDirectory, UART_V, UART, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    va = CLINT_V, pa = CLINT;
    for (u64 i = 0; i < 0x10000; i += PAGE_SIZE) {
        pageInsert(kernelPageDirectory, va + i, pa + i, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    }
    va = PLIC_V; pa = PLIC;
    for (u64 i = 0; i < 0x4000; i += PAGE_SIZE) {
        pageInsert(kernelPageDirectory, va + i, pa + i, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    }
    va = PLIC_V + 0x200000; pa = PLIC + 0x200000;
    for (u64 i = 0; i < 0x4000; i += PAGE_SIZE) {
        pageInsert(kernelPageDirectory, va + i, pa + i, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    }
    pageInsert(kernelPageDirectory, SPI_CTRL_ADDR, SPI_CTRL_ADDR, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    pageInsert(kernelPageDirectory, UART_CTRL_ADDR, UART_CTRL_ADDR, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    extern char textEnd[];
    va = pa = (u64)kernelStart;
    for (u64 i = 0; va + i < (u64)textEnd; i += PAGE_SIZE) {
        pageInsert(kernelPageDirectory, va + i, pa + i, PTE_READ | PTE_EXECUTE | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    }
    va = pa = (u64)textEnd;
    for (u64 i = 0; va + i < PHYSICAL_MEMORY_TOP; i += PAGE_SIZE) {
        pageInsert(kernelPageDirectory, va + i, pa + i, PTE_READ | PTE_WRITE | PTE_ACCESSED | PTE_DIRTY);
    }
    extern char trampoline[];
    pageInsert(kernelPageDirectory, TRAMPOLINE_BASE, (u64)trampoline, 
        PTE_READ | PTE_WRITE | PTE_EXECUTE | PTE_ACCESSED | PTE_DIRTY);
    pageInsert(kernelPageDirectory, TRAMPOLINE_BASE + PAGE_SIZE, (u64)trampoline + PAGE_SIZE, 
        PTE_READ | PTE_WRITE | PTE_EXECUTE | PTE_ACCESSED | PTE_DIRTY);
}

void startPage() {
    w_satp(MAKE_SATP(kernelPageDirectory));
    sfence_vma();
}

void memoryInit() {
    printf("Memory init start...\n");
    initFreePages();
    virtualMemory();
    resetRef();
    startPage();
    printf("Memory init finish!\n");
    printf("Test memory start...\n");
    // testMemory();
    printf("Test memory finish!\n");
}

void bcopy(void *src, void *dst, u32 len) {
    void *finish = src + len;

    while (src < finish) {
        *(u8*)dst = *(u8*)src;
        src++;
        dst++;
    }
}

void bzero(void *start, u32 len) {
    void *finish = start + len;

    if (len <= 7) {
        while (start < finish) {
            *(u8*)start++ = 0;
        }
        return;
    }
    while (((u64) start) & 7) {
        *(u8*)start++ = 0;
    }
    while (start + 7 < finish) {
        *(u64*)start = 0;
        start += 8;
    }
    while (start < finish) {
        *(u8*)start++ = 0;
    }
}