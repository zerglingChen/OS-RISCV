#ifndef _SYSFILE_H_
#define _SYSFILE_H_

//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include <Driver.h>
#include <Process.h>
#include <Sysarg.h>
#include <Fat.h>
#include <File.h>
#include <String.h>

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
int argfd(int n, int* pfd, struct File** pf);
// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
int fdalloc(struct File* f);

void syscallDup(void);
void syscallDupAndSet(void);
void syscall_fcntl(void);
void syscallRead(void);
void syscallWrite(void);
void syscallClose(void);
void syscallGetFileState(void);
void syscallGetFileStateAt(void);
void syscallOpenAt(void);
void syscallMakeDirAt(void);
void syscallChangeDir(void);
void syscallGetWorkDir(void);
void syscallPipe(void);
void syscallDevice(void);
void syscallGetDirent();
void syscallMount(void);
void syscallUmount(void);
void syscallUnlinkAt(void);
void syscallLinkAt(void);
void syscallLSeek(void);
void syscallWriteVector(void);
void syscallReadVector(void);
void syscallPRead();
void syscallUtimensat();
void syscallSendFile();
void syscallAccess();
void syscallRenameAt(void);
void syscallReadLinkAt(void);
void syscallUmask(void);
void syscallFileSychornize();
void syscallOpen(void);
void syscallChangeModAt(void);

int getAbsolutePath(Dirent* d, int isUser, u64 buf, int maxLen);

u64 sys_remove(void);
int do_linkat(int oldDirFd, char* oldPath, int newDirFd, char* newPath);
int do_unlinkat(int fd, char* path);

#define AT_FDCWD -100

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif