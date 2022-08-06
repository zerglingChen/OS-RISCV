#ifndef _SYSCALL_ID_H_
#define _SYSCALL_ID_H_

#define SYSCALL_PUTCHAR 4
#define SYSCALL_PROCESS_DESTORY 3
#define SYSCALL_PUT_STRING 5
#define SYSCALL_READDIR 10

#define SYSCALL_SBRK 13 // TODO

#define SYSCALL_CWD 17
#define SYSCALL_DEV 20
#define SYSCALL_DUP 23
#define SYSCALL_DUP3 24
#define SYSCALL_fcntl 25
#define SYSCALL_IOCONTROL 29

#define SYSCALL_MKDIRAT 34
#define SYSCALL_UNLINKAT 35
#define SYSCALL_LINKAT 37
#define SYSCALL_UMOUNT 39
#define SYSCALL_MOUNT 40
#define SYSCALL_STATE_FS 43
#define SYSCALL_ACCESS 48
#define SYSCALL_CHDIR 49

#define SYSCALL_OPEN 55
#define SYSCALL_OPENAT 56
#define SYSCALL_CLOSE 57
#define SYSCALL_PIPE2 59
#define SYSCALL_GET_DIRENT 61
#define SYSCALL_LSEEK 62
#define SYSCALL_READ 63
#define SYSCALL_WRITE 64
#define SYSCALL_READ_VECTOR 65
#define SYSCALL_WRITE_VECTOR 66
#define SYSCALL_PREAD 67
#define SYSCALL_PWRITE 68

#define SYSCALL_SEND_FILE 71

#define SYSCALL_POLL 73

#define SYSCALL_READLINKAT 78
#define SYSCALL_FSTATAT 79
#define SYSCALL_FSTAT 80

#define SYSCALL_UTIMENSAT 88

#define SYSCALL_EXIT 93 
#define SYSCALL_EXIT_GROUP 94 // TODO
#define SYSCALL_SET_TID_ADDRESS 96 // TODO
#define SYSCALL_FUTEX 98
#define SYSCALL_SET_ROBUST_LIST 99
#define SYSCALL_GET_ROBUST_LIST 100
#define SYSCALL_SLEEP_TIME 101
#define SYSCALL_GET_TIME 113

#define SYSCALL_LOG 116

#define SYSCALL_SCHED_YIELD 124

#define SYSCALL_PROCESS_KILL 129
#define SYSCALL_THREAD_KILL 130
#define SYSCALL_SIGNAL_ACTION 134
#define SYSCALL_SIGNAL_PROCESS_MASK 135
#define SYSCALL_SIGNAL_TIMED_WAIT 137
#define SYSCALL_SIGNAL_RETURN 139
#define SYSCALL_GET_CPU_TIMES 153
#define SYSCALL_SET_PROCESS_GROUP_ID 154
#define SYSCALL_GET_PROCESS_GROUP_ID 155
#define SYSCALL_UNAME 160
#define SYSCALL_GET_TIME_OF_DAY 169
#define SYSCALL_GET_PID 172
#define SYSCALL_GET_PARENT_PID 173
#define SYSCALL_GET_USER_ID 174
#define SYSCALL_GET_EFFECTIVE_USER_ID 175
#define SYSCALL_GET_EFFECTIVE_GROUP_ID 177
#define SYSCALL_GET_THREAD_ID 178
#define SYSCALL_GET_SYSTEM_INFO 179

#define SYSCALL_SOCKET 198
#define SYSCALL_BIND 200
#define SYSCALL_LISTEN 201
#define SYSCALL_ACCEPT 202
#define SYSCALL_CONNECT 203
#define SYSCALL_GET_SOCKET_NAME 204
#define SYSCALL_SEND_TO 206
#define SYSCALL_RECEIVE_FROM 207
#define SYSCALL_SET_SOCKET_OPTION 208
#define SYSCALL_BRK 214

#define SYSCALL_UNMAP_MEMORY 215
#define SYSCALL_CLONE 220
#define SYSCALL_EXEC 221
#define SYSCALL_MAP_MEMORY 222
#define SYSCALL_MEMORY_PROTECT 226

#define SYSCALL_WAIT 260
#define SYSCALL_PROCESS_RESOURSE_LIMIT 261

#define SYSCALL_RENAMEAT 276

#define SYSCALL_MEMORY_BARRIER 283
#endif