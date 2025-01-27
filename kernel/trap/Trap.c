#include <Riscv.h>
#include <Driver.h>
#include <Timer.h>
#include <Trap.h>
#include <Process.h>
#include <Page.h>
#include <Syscall.h>
#include <Hart.h>
#include <Sysfile.h>
#include <Debug.h>
#include <Defs.h>
#include <Exec.h>
#include <Thread.h>

void trapInit() {
    printf("Trap init start...\n");
    w_stvec((u64)kernelVector);
    w_sstatus(r_sstatus() | SSTATUS_SIE | SSTATUS_SPIE);
    // setNextTimeout();
    w_sip(0); //todo
    w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);
    printf("Trap init finish!\n");
}

int trapDevice() {
    u64 scause = r_scause();
    #ifdef QEMU
    if ((scause & SCAUSE_INTERRUPT) && 
    ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_SUPERVISOR_EXTERNAL)) {
    #else
    // todo
    if ((scause & SCAUSE_INTERRUPT) && 
    ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_SUPERVISOR_EXTERNAL)) {
    #endif
        int irq = interruptServed();
        if (irq == UART_IRQ) {
            int c = getchar();
            if (c != -1) {
                consoleInterrupt(c);
            }
        } else if (irq == DISK_IRQ) {
            // todo
        } else if (irq) {
            panic("unexpected interrupt irq = %d\n", irq);
        }
        if (irq) {
            interruptCompleted(irq);
        }
        #ifndef QEMU
        // todo
        #endif
        return SOFTWARE_TRAP;
    }
    if ((scause & SCAUSE_INTERRUPT) &&
    ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_SUPERVISOR_TIMER)) {
        timerTick();
        return TIMER_INTERRUPT;
    }
    return UNKNOWN_DEVICE;
}

void kernelTrap() {
    u64 sepc = r_sepc();
    u64 sstatus = r_sstatus();
    u64 scause = r_scause();
    u64 hartId = r_hartid();
    printf("[Kernel Trap] hartId is %lx, status is %lx, spec is %lx, cause is %lx, stval is %lx\n", hartId, sstatus, sepc, scause, r_stval());

#ifdef CJY_DEBUG
    printf("[Kernel Trap] hartId is %lx, status is %lx, spec is %lx, cause is %lx, stval is %lx\n", hartId, sstatus, sepc, scause, r_stval());
#endif

    Trapframe* trapframe = getHartTrapFrame();

    trapframeDump(trapframe);
    if (!(sstatus & SSTATUS_SPP)) {
        panic("kernel trap not from supervisor mode");
    }
    if (intr_get()) {
        panic("kernel trap while interrupts enbled");
    }
    
    int device = trapDevice();
    if (device == UNKNOWN_DEVICE) {
        u64* pte;
        int pa = pageLookup(myProcess()->pgdir, r_stval(), &pte);
        panic("unhandled error %d,  %lx, %lx\n", scause, r_stval(), pa);
        panic("kernel trap");
    }
    if (device == TIMER_INTERRUPT) {
        yield();
    }
    w_sepc(sepc);
    w_sstatus(sstatus);
}

static inline void userProcessCpuTimeEnd() {
    return;
    Process *p = myProcess();
    long currentTime = r_time();
    p->cpuTime.user += currentTime - p->processTime.lastUserTime;
}

void userTrap() {
    u64* pgdir = myProcess()->pgdir;
    // if ((scause & SCAUSE_EXCEPTION_CODE) != SCAUSE_ENVIRONMENT_CALL) {
    // int hartId = r_hartid();
    // printf("[User Trap] hartId is %lx, threadId: %lx, status is %lx, spec is %lx, cause is %d, stval is %lx, a7 is %d\n", 
    //    hartId, myThread()->id, r_sstatus(), r_sepc(), r_scause(), r_stval(), getHartTrapFrame()->a7);
    // // }
#ifdef CJY_DEBUG
    printf("[User Trap] hartId is %lx, status is %lx, spec is %lx, cause is %lx, stval is %lx\n", hartId, sstatus, sepc, scause, r_stval());
#else
    // use((void *)sepc);
#endif
    if (r_sstatus() & SSTATUS_SPP) {
        panic("usertrap: not from user mode\n");
    }
    w_stvec((u64) kernelVector);
    userProcessCpuTimeEnd();
    Trapframe* trapframe = getHartTrapFrame();
    // printf("in tp: %lx\n", trapframe->tp);
    if (r_scause() & SCAUSE_INTERRUPT) {
        trapDevice();
        yield();
    } else {
        kernelProcessCpuTimeBegin();
        u64 *pte = NULL;
        u64 pa = -1;
        // printf("sepc:%lx sstatus:%lx scause:%lx \n", sepc, sstatus, scause);
        switch (r_scause() & SCAUSE_EXCEPTION_CODE) {
        // case SCAUSE_BREAKPOINT:
        //     trapframe->epc += 4;
        //     break;
        case SCAUSE_ENVIRONMENT_CALL:
            trapframe->epc += 4;
            // printf("syscall %d\n", trapframe->a7);
            // if (trapframe->a7 == SYSCALL_WRITE || trapframe->a7 == SYSCALL_WRITE_VECTOR || trapframe->a7 == SYSCALL_SELECT || trapframe->a7 == SYSCALL_GET_TIME) {
            // if (trapframe->a7 != 63)
            //     printf("syscall-trigger %d, sepc: %lx\n", trapframe->a7, trapframe->epc);
            // }
            // if (trapframe->a7 != 72 && trapframe->a7 != 63 && trapframe->a7 != 165 && trapframe->a7 != 113) {
            //     printf("syscall-trigger: %d\n", trapframe->a7);
            // }
            if (!syscallVector[trapframe->a7]) {
                // printf("%lx\n", r_scause());
                panic("unknown-syscall: %d\n", trapframe->a7);
            }
            syscallVector[trapframe->a7]();
            // if (trapframe->a7 != 73 && trapframe->a7 != 64 && trapframe->a7 != 165 && trapframe->a7 != 113) {
            //     printf("syscall-end: %d\n", trapframe->a7);
            // }
            // if ((i64)trapframe->a0 <= -1) {
            // printf("return %d: %d\n", trapframe->a0, trapframe->a7);
            // }
            break;
        case SCAUSE_LOAD_PAGE_FAULT:
            pa = pageLookup(pgdir, r_stval(), &pte);
            if (pa == 0) {
                // printf("%d, spec: %lx\n", __LINE__, r_sepc());
                pageout(pgdir, r_stval());
            } else if (!(*pte & PTE_READ)) {
                processSignalSend(myProcess()->processId, SIGSEGV);
            } else {
                panic("unknown");
            }
            break;
        case SCAUSE_STORE_PAGE_FAULT:
            pa = pageLookup(pgdir, r_stval(), &pte);
            if (pa == 0) {
                // printf("%d, spec: %lx\n", __LINE__, r_sepc());
                pageout(pgdir, r_stval());
            } else if (*pte & PTE_COW) {
                cowHandler(pgdir, r_stval());
            } else if (!(*pte & PTE_WRITE)) {
                processSignalSend(myProcess()->processId, SIGSEGV);
            } else {
                // printf("spec: %lx %lx %lx %lx\n", sepc, pa, *pte, TRAMPOLINE_BASE);
                panic("unknown");
            }
            break;
        default:
            pageout(myProcess()->pgdir, r_stval());
            // trapframeDump(trapframe);
            // pageLookup(pgdir, r_stval(), &pte);
            // panic("unhandled error %d,  %lx, %lx\n", r_scause(), r_stval(), *pte);
            break;
        }
    }
    kernelProcessCpuTimeEnd();
    userTrapReturn();
}

static inline void userProcessCpuTimeBegin() {
    return;
    Process *p = myProcess();
    p->processTime.lastUserTime = r_time();
}

void userTrapReturn() {
    userProcessCpuTimeBegin();
    extern char trampoline[];
    w_stvec(TRAMPOLINE_BASE + ((u64)userVector - (u64)trampoline));

    Trapframe* trapframe = getHartTrapFrame();

    trapframe->kernelSp = getThreadTopSp(myThread());
    trapframe->trapHandler = (u64)userTrap;
    trapframe->kernelHartId = r_tp();

    handleSignal(myThread());

    //bcopy(&(currentProcess->trapframe), trapframe, sizeof(Trapframe));

    u64 sstatus = r_sstatus();
    sstatus &= ~SSTATUS_SPP;
    sstatus |= SSTATUS_SPIE;
    w_sstatus(sstatus);
    u64 satp = MAKE_SATP(myProcess()->pgdir);
    u64 fn = TRAMPOLINE_BASE + ((u64)userReturn - (u64)trampoline);
    // printf("out tp: %lx\n", trapframe->tp);
    // printf("return to user!\n");
    ((void(*)(u64, u64))fn)((u64)trapframe, satp);
}

void trapframeDump(Trapframe *tf)
{
    printf(" a0: %lx\n \
a1: %lx\n \
a2: %lx\n \
a3: %lx\n \
a4: %lx\n \
a5: %lx\n \
a6: %lx\n \
a7: %lx\n \
t0: %lx\n \
t1: %lx\n \
t2: %lx\n \
t3: %lx\n \
t4: %lx\n \
t5: %lx\n \
t6: %lx\n \
s0: %lx\n \
s1: %lx\n \
s2: %lx\n \
s3: %lx\n \
s4: %lx\n \
s5: %lx\n \
s6: %lx\n \
s7: %lx\n \
s8: %lx\n \
s9: %lx\n \
s10: %lx\n \
s11: %lx\n \
ra: %lx\n \
sp: %lx\n \
gp: %lx\n \
tp: %lx\n \
epc: %lx\n \
kernelSp: %lx\n \
kernelSatp: %lx\n \
trapHandler: %lx\n \
kernelHartId: %lx\n",
            tf->a0, tf->a1, tf->a2, tf->a3, tf->a4,
            tf->a5, tf->a6, tf->a7, tf->t0, tf->t1,
            tf->t2, tf->t3, tf->t4, tf->t5, tf->t6,
            tf->s0, tf->s1, tf->s2, tf->s3, tf->s4,
            tf->s5, tf->s6, tf->s7, tf->s8, tf->s9,
            tf->s10, tf->s11, tf->ra, tf->sp, tf->gp,
            tf->tp, tf->epc, tf->kernelSp, tf->kernelSatp,
            tf->trapHandler, tf->kernelHartId);
}
