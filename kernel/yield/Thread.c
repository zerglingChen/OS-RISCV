#include <Thread.h>
#include <Riscv.h>
#include <MemoryConfig.h>
#include <Error.h>
#include <FileSystem.h>
#include <Sysfile.h>
#include <Page.h>
#include <Interrupt.h>
#include <Process.h>
#include <Trap.h>
#include <Futex.h>

Thread threads[PROCESS_TOTAL_NUMBER];

struct ThreadList freeThreads, usedThreads;
struct ThreadList scheduleList[2];
Thread *currentThread[HART_TOTAL_NUMBER] = {0};

struct Spinlock threadListLock, scheduleListLock, threadIdLock;

Thread* myThread() {
    // interruptPush();
    if (currentThread[r_hartid()] == NULL) {
        // printf("[Kernel]No thread run in the hart %d\n", hartId);
        // interruptPop();
        return NULL;
    }
    // interruptPop();
    return currentThread[r_hartid()];
}

u64 getThreadTopSp(Thread* th) {
    return KERNEL_PROCESS_SP_TOP - (u64)(th - threads) * 10 * PAGE_SIZE;
}

extern u64 kernelPageDirectory[];
void threadInit() {
    initLock(&threadListLock, "threadList");
    initLock(&scheduleListLock, "scheduleList");
    initLock(&threadIdLock, "threadId");

    LIST_INIT(&freeThreads);
    LIST_INIT(&usedThreads);
    LIST_INIT(&scheduleList[0]);
    LIST_INIT(&scheduleList[1]);

    int i;
    for (i = PROCESS_TOTAL_NUMBER - 1; i >= 0; i--) {
        threads[i].state = UNUSED;
        threads[i].trapframe.kernelSatp = MAKE_SATP(kernelPageDirectory);
        LIST_INSERT_HEAD(&freeThreads, &threads[i], link);
    }
}

u32 generateThreadId(Thread* th) {
    // acquireLock(&threadIdLock);
    static u32 nextId = 0;
    u32 threadId = (++nextId << (1 + LOG_PROCESS_NUM)) | (u32)(th - threads);
    // releaseLock(&threadIdLock);
    return threadId;
}

void __attribute__ ((noreturn)) sleepRec();
void threadDestroy(Thread *th) {
    threadFree(th);
    int hartId = r_hartid();
    if (currentThread[hartId] == th) {
        currentThread[hartId] = NULL;
        extern char kernelStack[];
        u64 sp = (u64)kernelStack + (hartId + 1) * KERNEL_STACK_SIZE;
        asm volatile("ld sp, 0(%0)" : :"r"(&sp): "memory");
        yield();
    }
}

void threadFree(Thread *th) {
    Process* p = th->process;
    // acquireLock(&th->lock);
    while (!LIST_EMPTY(&th->waitingSignal)) {
        SignalContext* sc = LIST_FIRST(&th->waitingSignal);
        LIST_REMOVE(sc, link);
        signalContextFree(sc);
    }
    // releaseLock(&th->lock);
    // acquireLock(&p->lock);
    if (th->clearChildTid) {
        int val = 0;
        copyout(p->pgdir, th->clearChildTid, (char*)&val, sizeof(int));
        futexWake(th->clearChildTid, 1);
    }
    p->threadCount--;
    if (!p->threadCount) {
        p->retValue = th->retValue;
        // releaseLock(&p->lock);
        processFree(p);    
    } else {
        // releaseLock(&p->lock);
    }

    // acquireLock(&threadListLock);
    th->state = UNUSED;
    LIST_REMOVE(th, link);
    LIST_INSERT_HEAD(&freeThreads, th, link); //test pipe
    // releaseLock(&threadListLock);
}

int tid2Thread(u32 threadId, struct Thread **thread, int checkPerm) {
    struct Thread* th;
    int hartId = r_hartid();

    if (threadId == 0) {
        *thread = currentThread[hartId];
        return 0;
    }

    th = threads + PROCESS_OFFSET(threadId);

    if (th->state == UNUSED || th->id != threadId) {
        *thread = NULL;
        return -ESRCH;
    }

    if (checkPerm) {
        if (th != currentThread[hartId] && th->process->parentId != myProcess()->processId) {
            *thread = NULL;
            return -EPERM;
        }
    }

    *thread = th;
    return 0;
}

extern void userVector();

void threadSetup(Thread* th) {
    th->chan = 0;
    th->retValue = 0;
    th->state = UNUSED;
    th->reason = 0;
    th->setChildTid = th->clearChildTid = 0;
    th->awakeTime = 0;
    th->robustHeadPointer = 0;
    th->killed = false;
    LIST_INIT(&th->waitingSignal);
    PhysicalPage *page;
    if (pageAlloc(&page) < 0) {
        panic("");
    }
    pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE, page2pa(page), PTE_READ | PTE_WRITE);
    if (pageAlloc(&page) < 0) {
        panic("");
    }
    pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 2, page2pa(page), PTE_READ | PTE_WRITE);
    if (pageAlloc(&page) < 0) {
        panic("");
    }
    pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 3, page2pa(page), PTE_READ | PTE_WRITE);
    if (pageAlloc(&page) < 0) {
        panic("");
    }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 4, page2pa(page), PTE_READ | PTE_WRITE);
    // if (pageAlloc(&page) < 0) {
    //     panic("");
    // }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 5, page2pa(page), PTE_READ | PTE_WRITE);
    // if (pageAlloc(&page) < 0) {
    //     panic("");
    // }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 6, page2pa(page), PTE_READ | PTE_WRITE);
    // if (pageAlloc(&page) < 0) {
    //     panic("");
    // }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 7, page2pa(page), PTE_READ | PTE_WRITE);
    // if (pageAlloc(&page) < 0) {
    //     panic("");
    // }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 8, page2pa(page), PTE_READ | PTE_WRITE);
    // if (pageAlloc(&page) < 0) {
    //     panic("");
    // }
    // pageInsert(kernelPageDirectory, getThreadTopSp(th) - PAGE_SIZE * 9, page2pa(page), PTE_READ | PTE_WRITE);
}

u64 getSignalHandlerSp(Thread *th) {
    return SIGNAL_HANDLER_SP_BASE + (th - threads + 1) * PAGE_SIZE * 10;
}

int mainThreadAlloc(Thread **new, u64 parentId) {
    int r;
    Thread *th;
    // acquireLock(&threadListLock);
    if (LIST_EMPTY(&freeThreads)) {
        // releaseLock(&threadListLock);
        panic("");
        *new = NULL;
        return -ESRCH;
    }
    th = LIST_FIRST(&freeThreads);
    LIST_REMOVE(th, link);
    LIST_INSERT_HEAD(&usedThreads, th, link);
    // releaseLock(&threadListLock);

    threadSetup(th);
    th->id = generateThreadId(th);
    th->state = RUNNABLE;
    th->trapframe.kernelSp = getThreadTopSp(th);
    th->trapframe.sp = USER_STACK_TOP - 24; //argc = 0, argv = 0, envp = 0
    Process *process;
    r = processAlloc(&process, parentId);
    if (r < 0) {
        *new = NULL;
        return r;
    }
    // acquireLock(&process->lock);
    th->process = process;
    process->threadCount++;
    // releaseLock(&process->lock);
    *new = th;
    return 0;
}

int threadAlloc(Thread **new, Process* process, u64 userSp) {
    Thread *th;
    // acquireLock(&threadListLock);
    if (LIST_EMPTY(&freeThreads)) {
        // releaseLock(&threadListLock);
        panic("");
        *new = NULL;
        return -ESRCH;
    }
    th = LIST_FIRST(&freeThreads);
    LIST_REMOVE(th, link);
    LIST_INSERT_HEAD(&usedThreads, th, link);
    // releaseLock(&threadListLock);

    threadSetup(th);
    th->id = generateThreadId(th);
    th->state = RUNNABLE;
    th->trapframe.kernelSp = getThreadTopSp(th);
    th->trapframe.sp = userSp;

    // acquireLock(&process->lock);
    th->process = process;
    process->threadCount++;
    // releaseLock(&process->lock);

    *new = th;
    return 0;
}

void threadRun(Thread* th) {
    static volatile int first = 0;
    Trapframe* trapframe = getHartTrapFrame();
    if (currentThread[r_hartid()]) {
        bcopy(trapframe, &(currentThread[r_hartid()]->trapframe),
              sizeof(Trapframe));
    }
    
    th->state = RUNNING;
    if (th->reason & KERNEL_GIVE_UP) {
        th->reason &= ~KERNEL_GIVE_UP;
        currentThread[r_hartid()] = th;
        bcopy(&currentThread[r_hartid()]->trapframe, trapframe, sizeof(Trapframe));
        asm volatile("ld sp, 0(%0)" : : "r"(&th->currentKernelSp));
        sleepRec();
    } else {
        currentThread[r_hartid()] = th;
        if (first == 0) {
            // File system initialization must be run in the context of a
            // regular process (e.g., because it calls sleep), and thus cannot
            // be run from main().
            first = 1;
            extern FileSystem *rootFileSystem;
            if (rootFileSystem == NULL) {
                fsAlloc(&rootFileSystem);
            }
            strncpy(rootFileSystem->name, "fat32", 6);
            
            rootFileSystem->read = blockRead;
            
            direntInit();
            fatInit(rootFileSystem);
            void testfat();
            testfat();

            Dirent* ep = create(AT_FDCWD, "/var/tmp/XXX", T_FILE, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/vda2", T_DIR, O_RDONLY); //driver
            assert(ep != NULL);
            ep->head = rootFileSystem;            
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/shm", T_DIR, O_RDONLY); //share memory
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/null", T_CHAR, O_RDONLY); //share memory
            ep->dev = NONE;
            assert(ep != NULL);
            eunlock(ep);
            ep = create(AT_FDCWD, "/tmp", T_DIR, O_RDONLY); //share memory
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/zero", T_CHAR, O_RDONLY);
            // panic("");
            assert(ep != NULL);
            ep->dev = ZERO;
            eunlock(ep);
            // Don't eput here

            // ep = ename(AT_FDCWD, "/lib");
            // if (ep == NULL) {
            //     if ((ep = create(AT_FDCWD, "/lib", T_DIR, O_RDONLY)) == 0) {
            //         printf("create /lib error");
            //     } else {
            //         eunlock(ep);
            //         eput(ep);
            //     }
            // }
            // if (ep == NULL || do_linkat(AT_FDCWD, "/libc.so", AT_FDCWD,
            //                             "/lib/ld-musl-riscv64-sf.so.1") < 0) {
            //     printf("pre_link error!\n");
            // }
            ep = create(AT_FDCWD, "/dev/tty", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/rtc", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/rtc0", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/misc", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/dev/misc/rtc", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/meminfo", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/mounts", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/localtime", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/sys", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/sys/kernel", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/sys/kernel/osrelease", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            ep->dev = OSRELEASE;
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/proc/self", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);   
            if (do_linkat(AT_FDCWD, "/", AT_FDCWD, "/proc/self/exe") < 0) {
                panic("");
            }
            ep = create(AT_FDCWD, "/etc", T_DIR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/etc/adjtime", T_CHAR, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/etc/localtime", T_CHAR, O_RDONLY);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/var", T_DIR, O_RDONLY);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/var/tmp", T_DIR, O_RDONLY);
            eunlock(ep);
            eput(ep);
            ep = create(AT_FDCWD, "/var/tmp/XXX", T_FILE, O_RDONLY);
            assert(ep != NULL);
            eunlock(ep);
            eput(ep);
            // ep = create(AT_FDCWD, "/hello", T_FILE, O_RDONLY);
            // assert(ep != NULL);
            // eunlock(ep);
            // eput(ep);
            // setNextTimeout();
        }
        bcopy(&(currentThread[r_hartid()]->trapframe), trapframe, sizeof(Trapframe));
        u64 sp = getHartKernelTopSp(th);
        asm volatile("ld sp, 0(%0)" : :"r"(&sp): "memory");
        // releaseLock(&currentProcessLock);
        userTrapReturn();
    }
}

void sleepSave();
void sleep(void* chan, struct Spinlock* lk) { 
    Thread* th = myThread();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.

    kernelProcessCpuTimeEnd();
    // acquireLock(&th->lock);  // DOC: sleeplock1
    // releaseLock(lk);

    // Go to sleep.
    th->chan = (u64)chan;
    th->state = SLEEPING;
    th->reason |= KERNEL_GIVE_UP;
    // releaseLock(&th->lock);

    if (th->killed) {
        threadDestroy(th);
    }    

	asm volatile("sd sp, 0(%0)" : :"r"(&th->currentKernelSp));

    sleepSave();

    // // Tidy up.
    // acquireLock(&th->lock);  // DOC: sleeplock1
    th->chan = 0;
    // releaseLock(&th->lock);

    kernelProcessCpuTimeBegin();
    
    if (th->killed) {
        threadDestroy(th);
    }    
    // Reacquire original lock.
    // acquireLock(lk);
}

void wakeup(void* channel) {
    Thread* thread = NULL;
    LIST_FOREACH(thread, &usedThreads, link) {
        // acquireLock(&thread->lock);
        if (thread->state == SLEEPING && thread->chan == (u64)channel) {
            thread->state = RUNNABLE;
        }
        // releaseLock(&thread->lock);
    }
}

