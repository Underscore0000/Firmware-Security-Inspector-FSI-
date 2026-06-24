#include "registers.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static const register_bit_info_t cr4_bits[] = {
    {CR4_VME,        0,  "VME",        "Virtual-8086 Mode Extensions",               false},
    {CR4_PVI,        1,  "PVI",        "Protected-Mode Virtual Interrupts",           false},
    {CR4_TSD,        2,  "TSD",        "Time Stamp Disable (RDTSC ring 0 only)",      true},
    {CR4_DE,         3,  "DE",         "Debugging Extensions",                        false},
    {CR4_PSE,        4,  "PSE",        "Page Size Extensions (4MB pages)",            false},
    {CR4_PAE,        5,  "PAE",        "Physical Address Extension (>4GB RAM)",       false},
    {CR4_MCE,        6,  "MCE",        "Machine-Check Enable",                        false},
    {CR4_PGE,        7,  "PGE",        "Page Global Enable",                          false},
    {CR4_PCE,        8,  "PCE",        "Performance Counter Enable in user mode",     false},
    {CR4_OSFXSR,     9,  "OSFXSR",     "OS FXSAVE/FXRSTOR Support",                  false},
    {CR4_OSXMMEXCPT, 10, "OSXMMEXCPT", "Unmasked SIMD FP Exceptions",                false},
    {CR4_UMIP,       11, "UMIP",       "User-Mode Instruction Prevention",            true},
    {CR4_LA57,       12, "LA57",       "57-bit Linear Addresses (5-level paging)",    false},
    {CR4_VMXE,       13, "VMXE",       "VMX Enable",                                  false},
    {CR4_SMXE,       14, "SMXE",       "SMX Enable",                                  false},
    {CR4_FSGSBASE,   16, "FSGSBASE",   "FS/GS Base Instructions Enable",              false},
    {CR4_PCIDE,      17, "PCIDE",      "Process-Context Identifiers Enable",          false},
    {CR4_OSXSAVE,    18, "OSXSAVE",    "XSAVE and Extended States Enable",            false},
    {CR4_SMEP,       20, "SMEP",       "Supervisor Mode Execution Prevention",        true},
    {CR4_SMAP,       21, "SMAP",       "Supervisor Mode Access Prevention",           true},
    {CR4_PKE,        22, "PKE",        "Protection Keys Enable",                      false},
    {CR4_CET,        23, "CET",        "Control-flow Enforcement Technology",         true},
    {CR4_PKS,        24, "PKS",        "Protection Keys for Supervisor",              false},
};

static const register_bit_info_t cr0_bits[] = {
    {CR0_PE,  0,  "PE",  "Protection Enable (protected mode)",       false},
    {CR0_MP,  1,  "MP",  "Monitor Coprocessor",                      false},
    {CR0_EM,  2,  "EM",  "Emulation (no FPU)",                       false},
    {CR0_TS,  3,  "TS",  "Task Switched",                            false},
    {CR0_ET,  4,  "ET",  "Extension Type",                           false},
    {CR0_NE,  5,  "NE",  "Numeric Error",                            false},
    {CR0_WP,  16, "WP",  "Write Protect (kernel cannot write RO pages)", true},
    {CR0_AM,  18, "AM",  "Alignment Mask",                           false},
    {CR0_NW,  29, "NW",  "Not Write-through",                        false},
    {CR0_CD,  30, "CD",  "Cache Disable",                            false},
    {CR0_PG,  31, "PG",  "Paging Enable",                            false},
};

/*
 * On Linux userspace, CR0/CR4 are not directly accessible.
 * Requires ring 0 (kernel module or UEFI mode).
 */
int fsi_registers_collect(fsi_registers_t *regs)
{
    if (!regs) return -1;
    memset(regs, 0, sizeof(*regs));

#if defined(__linux__) && !defined(FSI_KERNEL_MODE)
    regs->available = false;
    snprintf(regs->error_msg, sizeof(regs->error_msg),
             "Control registers require ring 0. "
             "Load kernel module fsi_kmod or run in UEFI mode.");
    return 0;
#elif defined(FSI_KERNEL_MODE) || defined(__UEFI__)
    regs->cr0 = fsi_read_cr0();
    regs->cr2 = fsi_read_cr2();
    regs->cr3 = fsi_read_cr3();
    regs->cr4 = fsi_read_cr4();
    regs->rflags = fsi_read_rflags();
    regs->available = true;
    return 0;
#else
    regs->available = false;
    snprintf(regs->error_msg, sizeof(regs->error_msg),
             "Platform not supported for CR register access.");
    return -1;
#endif
}

void fsi_registers_print(const fsi_registers_t *regs)
{
    if (!regs->available) {
        printf("Control Registers: %s\n", regs->error_msg);
        return;
    }

    printf("=== Control Registers ===\n");
    printf("CR0:    0x%016llX\n", (unsigned long long)regs->cr0);
    printf("CR2:    0x%016llX  (Page Fault Address)\n",
           (unsigned long long)regs->cr2);
    printf("CR3:    0x%016llX  (Page Table Base)\n",
           (unsigned long long)regs->cr3);
    printf("CR4:    0x%016llX\n", (unsigned long long)regs->cr4);
    printf("RFLAGS: 0x%016llX\n", (unsigned long long)regs->rflags);

    printf("\n--- CR4 Bit Decode ---\n");
    int count;
    const register_bit_info_t *bits = fsi_cr4_bit_table(&count);
    for (int i = 0; i < count; i++) {
        bool set = (regs->cr4 & bits[i].mask) != 0;
        printf("  Bit %2d %-12s [%s] %s%s\n",
               bits[i].bit,
               bits[i].name,
               set ? "1" : "0",
               bits[i].description,
               bits[i].security_relevant ? " [SECURITY]" : "");
    }
}

const register_bit_info_t *fsi_cr4_bit_table(int *count)
{
    if (count) *count = (int)(sizeof(cr4_bits) / sizeof(cr4_bits[0]));
    return cr4_bits;
}

const register_bit_info_t *fsi_cr0_bit_table(int *count)
{
    if (count) *count = (int)(sizeof(cr0_bits) / sizeof(cr0_bits[0]));
    return cr0_bits;
}