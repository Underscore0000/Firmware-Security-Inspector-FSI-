#ifndef FSI_REGISTERS_H
#define FSI_REGISTERS_H

#include <stdint.h>
#include <stdbool.h>

/* Bit definitions CR0 */
#define CR0_PE   (1ULL << 0)   /* Protection Enable */
#define CR0_MP   (1ULL << 1)   /* Monitor Coprocessor */
#define CR0_EM   (1ULL << 2)   /* Emulation */
#define CR0_TS   (1ULL << 3)   /* Task Switched */
#define CR0_ET   (1ULL << 4)   /* Extension Type */
#define CR0_NE   (1ULL << 5)   /* Numeric Error */
#define CR0_WP   (1ULL << 16)  /* Write Protect */
#define CR0_AM   (1ULL << 18)  /* Alignment Mask */
#define CR0_NW   (1ULL << 29)  /* Not Write-through */
#define CR0_CD   (1ULL << 30)  /* Cache Disable */
#define CR0_PG   (1ULL << 31)  /* Paging */

/* Bit definitions CR4 */
#define CR4_VME        (1ULL << 0)   /* Virtual-8086 Mode Extensions */
#define CR4_PVI        (1ULL << 1)   /* Protected-Mode Virtual Interrupts */
#define CR4_TSD        (1ULL << 2)   /* Time Stamp Disable */
#define CR4_DE         (1ULL << 3)   /* Debugging Extensions */
#define CR4_PSE        (1ULL << 4)   /* Page Size Extensions */
#define CR4_PAE        (1ULL << 5)   /* Physical Address Extension */
#define CR4_MCE        (1ULL << 6)   /* Machine-Check Enable */
#define CR4_PGE        (1ULL << 7)   /* Page Global Enable */
#define CR4_PCE        (1ULL << 8)   /* Perf Monitor Counter Enable */
#define CR4_OSFXSR     (1ULL << 9)   /* OS FXSAVE/FXRSTOR */
#define CR4_OSXMMEXCPT (1ULL << 10)  /* OS Unmasked SIMD FP Exceptions */
#define CR4_UMIP       (1ULL << 11)  /* User-Mode Instruction Prevention */
#define CR4_LA57       (1ULL << 12)  /* 57-bit Linear Addresses */
#define CR4_VMXE       (1ULL << 13)  /* VMX Enable */
#define CR4_SMXE       (1ULL << 14)  /* SMX Enable */
#define CR4_FSGSBASE   (1ULL << 16)  /* FS/GS Base Enable */
#define CR4_PCIDE      (1ULL << 17)  /* PCID Enable */
#define CR4_OSXSAVE    (1ULL << 18)  /* XSAVE Enable */
#define CR4_SMEP       (1ULL << 20)  /* Supervisor Mode Exec Prevention */
#define CR4_SMAP       (1ULL << 21)  /* Supervisor Mode Access Prevention */
#define CR4_PKE        (1ULL << 22)  /* Protection Key Enable */
#define CR4_CET        (1ULL << 23)  /* Control-flow Enforcement */
#define CR4_PKS        (1ULL << 24)  /* Protection Keys Supervisor */

typedef struct {
    uint64_t cr0;
    uint64_t cr2;
    uint64_t cr3;
    uint64_t cr4;
    uint64_t rflags;
    bool     available;
    char     error_msg[128];
} fsi_registers_t;

typedef struct {
    uint64_t    mask;
    int         bit;
    const char *name;
    const char *description;
    bool        security_relevant;
} register_bit_info_t;


extern uint64_t fsi_read_cr0(void);
extern uint64_t fsi_read_cr2(void);
extern uint64_t fsi_read_cr3(void);
extern uint64_t fsi_read_cr4(void);
extern uint64_t fsi_read_rflags(void);


int  fsi_registers_collect(fsi_registers_t *regs);
void fsi_registers_print(const fsi_registers_t *regs);
const register_bit_info_t *fsi_cr4_bit_table(int *count);
const register_bit_info_t *fsi_cr0_bit_table(int *count);

#endif 