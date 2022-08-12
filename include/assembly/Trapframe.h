#ifndef _ASSEMBLY_TRAPFRAME_H_
#define _ASSEMBLY_TRAPFRAME_H_

#define KERNEL_SATP 0
#define KERNEL_SP 8
#define TRAP_HANDLER 16
#define EPC 24
#define KERNEL_HART_ID 32
#define REG_RA 40
#define REG_SP 48
#define REG_GP 56
#define REG_TP 64
#define REG_T0 72
#define REG_T1 80
#define REG_T2 88
#define REG_S0 96
#define REG_S1 104
#define REG_A0 112
#define REG_A1 120
#define REG_A2 128
#define REG_A3 136
#define REG_A4 144
#define REG_A5 152
#define REG_A6 160
#define REG_A7 168
#define REG_S2 176
#define REG_S3 184
#define REG_S4 192
#define REG_S5 200
#define REG_S6 208
#define REG_S7 216
#define REG_S8 224
#define REG_S9 232
#define REG_S10 240
#define REG_S11 248
#define REG_T3 256
#define REG_T4 264
#define REG_T5 272
#define REG_T6 280
#define REG_FT0 288
#define REG_FT1 296
#define REG_FT2 304
#define REG_FT3 312
#define REG_FT4 320
#define REG_FT5 328
#define REG_FT6 336
#define REG_FT7 344
#define REG_FS0 352
#define REG_FS1 360
#define REG_FA0 368
#define REG_FA1 376
#define REG_FA2 384
#define REG_FA3 392
#define REG_FA4 400
#define REG_FA5 408
#define REG_FA6 416
#define REG_FA7 424
#define REG_FS2 432
#define REG_FS3 440
#define REG_FS4 448
#define REG_FS5 456
#define REG_FS6 464
#define REG_FS7 472
#define REG_FS8 480
#define REG_FS9 488
#define REG_FS10 496
#define REG_FS11 504
#define REG_FT8 512
#define REG_FT9 520
#define REG_FT10 528
#define REG_FT11 536
#endif