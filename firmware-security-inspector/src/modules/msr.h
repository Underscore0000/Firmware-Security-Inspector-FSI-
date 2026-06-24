#ifndef FSI_MSR_H
#define FSI_MSR_H

#include <stdint.h>
#include <stdbool.h>

/* MSR addresses */
#define MSR_IA32_APIC_BASE          0x0000001B
#define MSR_IA32_FEATURE_CONTROL    0x0000003A
#define MSR_IA32_TSC                0x00000010
#define MSR_IA32_MISC_ENABLE        0x000001A0
#define MSR_IA32_MCG_CAP            0x00000179
#define MSR_IA32_PERF_STATUS        0x00000198
#define MSR_IA32_PERF_CTL           0x00000199
#define MSR_IA32_THERM_STATUS       0x0000019C
#define MSR_IA32_UCODE_REV          0x0000008B
#define MSR_IA32_PLATFORM_ID        0x00000017
#define MSR_IA32_EFER               0xC0000080
#define MSR_IA32_STAR               0xC0000081
#define MSR_IA32_LSTAR              0xC0000082
#define MSR_IA32_FMASK              0xC0000084
#define MSR_IA32_FS_BASE            0xC0000100
#define MSR_IA32_GS_BASE            0xC0000101
#define MSR_IA32_KERNEL_GS_BASE     0xC0000102
#define MSR_IA32_TSC_AUX            0xC0000103
#define MSR_IA32_SPEC_CTRL          0x00000048
#define MSR_IA32_PRED_CMD           0x00000049
#define MSR_IA32_ARCH_CAPABILITIES  0x0000010A
#define MSR_IA32_FLUSH_CMD          0x0000010B
#define MSR_IA32_VMX_BASIC          0x00000480

typedef struct {
    uint32_t    index;
    const char *name;
    const char *description;
    bool        security_relevant;
} msr_descriptor_t;

typedef struct {
    uint32_t    index;
    uint64_t    value;
    bool        readable;
    char        name[64];
    char        description[256];
} fsi_msr_entry_t;

typedef struct {
    fsi_msr_entry_t *entries;
    int              count;
    bool             available;
    char             error_msg[256];
} fsi_msr_data_t;


extern uint64_t fsi_rdmsr(uint32_t msr_index);


int  fsi_msr_collect(fsi_msr_data_t *data);
void fsi_msr_free(fsi_msr_data_t *data);
void fsi_msr_print(const fsi_msr_data_t *data);
uint64_t fsi_msr_read_safe(uint32_t index, bool *ok);

#endif 