#include "msr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static const msr_descriptor_t msr_table[] = {
    {MSR_IA32_UCODE_REV,        "IA32_UCODE_REV",
     "Processor microcode version. Microcode updates fix vulnerabilities like Spectre/Meltdown.", true},

    {MSR_IA32_APIC_BASE,        "IA32_APIC_BASE",
     "APIC base address and enable/bootstrap flags. Bit 11 = APIC global enable.", false},

    {MSR_IA32_FEATURE_CONTROL,  "IA32_FEATURE_CONTROL",
     "VMX and SMX control. Bit 0 = lock, Bit 2 = VMX enable, Bit 1 = VMX in SMX. Must be locked (bit 0 = 1).", true},

    {MSR_IA32_EFER,             "IA32_EFER",
     "Extended Feature Enable Register. Bit 0 = SYSCALL, Bit 8 = LME (Long Mode Enable), "
     "Bit 10 = LMA, Bit 11 = NXE (No-Execute Enable). NXE must be 1 for NX/XD to work.", true},

    {MSR_IA32_MISC_ENABLE,      "IA32_MISC_ENABLE",
     "Miscellaneous feature enables. Bit 18 = Enhanced SpeedStep, Bit 34 = xTPR Message Disable.", false},

    {MSR_IA32_SPEC_CTRL,        "IA32_SPEC_CTRL",
     "Speculative Execution Control. Bit 0 = IBRS (Indirect Branch Restricted Speculation), "
     "Bit 1 = STIBP (Single Thread Indirect Branch Predictors). Spectre v2 mitigations.", true},

    {MSR_IA32_ARCH_CAPABILITIES,"IA32_ARCH_CAPABILITIES",
     "Architectural capabilities for security. Bit 0 = RDCL_NO (not vulnerable to Meltdown), "
     "Bit 1 = IBRS_ALL, Bit 2 = RSBA, Bit 3 = SKIP_L1DFL_VMENTRY, Bit 4 = SSB_NO (not vulnerable to Spectre v4).", true},

    {MSR_IA32_PERF_STATUS,      "IA32_PERF_STATUS",
     "Current CPU frequency status (P-state).", false},

    {MSR_IA32_MCG_CAP,          "IA32_MCG_CAP",
     "Machine Check Global Capabilities. Number of available MCA banks.", false},

    {MSR_IA32_PLATFORM_ID,      "IA32_PLATFORM_ID",
     "Processor platform ID, used for microcode selection.", false},

    {MSR_IA32_TSC_AUX,          "IA32_TSC_AUX",
     "TSC auxiliary value, typically contains core/socket ID.", false},

    {MSR_IA32_STAR,             "IA32_STAR",
     "SYSCALL/SYSRET selectors (CS/SS segments).", false},

    {MSR_IA32_LSTAR,            "IA32_LSTAR",
     "Long mode SYSCALL target RIP (kernel syscall handler address).", false},

    {MSR_IA32_VMX_BASIC,        "IA32_VMX_BASIC",
     "VMX basic info: VMCS revision ID, VMCS region size, supported memory type.", false},
};

#define MSR_TABLE_SIZE ((int)(sizeof(msr_table) / sizeof(msr_table[0])))

/*
 * On Linux userspace: reads /dev/cpu/0/msr
 * Requires modprobe msr and root privileges.
 */
uint64_t fsi_msr_read_safe(uint32_t index, bool *ok)
{
    uint64_t value = 0;

#if defined(__linux__)
    int fd = open("/dev/cpu/0/msr", O_RDONLY);
    if (fd < 0) {
        if (ok) *ok = false;
        return 0;
    }
    ssize_t n = pread(fd, &value, sizeof(value), (off_t)index);
    close(fd);
    if (n != sizeof(value)) {
        if (ok) *ok = false;
        return 0;
    }
    if (ok) *ok = true;
    return value;
#elif defined(FSI_KERNEL_MODE) || defined(__UEFI__)
    value = fsi_rdmsr(index);
    if (ok) *ok = true;
    return value;
#else
    (void)index;
    if (ok) *ok = false;
    return 0;
#endif
}

int fsi_msr_collect(fsi_msr_data_t *data)
{
    if (!data) return -1;
    memset(data, 0, sizeof(*data));

    data->entries = calloc((size_t)MSR_TABLE_SIZE, sizeof(fsi_msr_entry_t));
    if (!data->entries) return -1;

    data->count = MSR_TABLE_SIZE;

#if defined(__linux__)
    int test_fd = open("/dev/cpu/0/msr", O_RDONLY);
    if (test_fd < 0) {
        data->available = false;
        snprintf(data->error_msg, sizeof(data->error_msg),
                 "Cannot open /dev/cpu/0/msr: %s\n"
                 "Run: sudo modprobe msr\n"
                 "Then run the program as root.",
                 strerror(errno));
        for (int i = 0; i < MSR_TABLE_SIZE; i++) {
            data->entries[i].index = msr_table[i].index;
            data->entries[i].readable = false;
            data->entries[i].value = 0;
            strncpy(data->entries[i].name,
                    msr_table[i].name, sizeof(data->entries[i].name) - 1);
            strncpy(data->entries[i].description,
                    msr_table[i].description,
                    sizeof(data->entries[i].description) - 1);
        }
        return 0;
    }
    close(test_fd);
#endif

    data->available = true;

    for (int i = 0; i < MSR_TABLE_SIZE; i++) {
        bool ok = false;
        uint64_t val = fsi_msr_read_safe(msr_table[i].index, &ok);

        data->entries[i].index = msr_table[i].index;
        data->entries[i].value = val;
        data->entries[i].readable = ok;
        strncpy(data->entries[i].name,
                msr_table[i].name, sizeof(data->entries[i].name) - 1);
        strncpy(data->entries[i].description,
                msr_table[i].description,
                sizeof(data->entries[i].description) - 1);
    }

    return 0;
}

void fsi_msr_free(fsi_msr_data_t *data)
{
    if (data && data->entries) {
        free(data->entries);
        data->entries = NULL;
        data->count = 0;
    }
}

void fsi_msr_print(const fsi_msr_data_t *data)
{
    if (!data) return;
    printf("=== MSR Values ===\n");
    if (!data->available) {
        printf("Not available: %s\n", data->error_msg);
        printf("\nKnown MSRs (values not read):\n");
    }
    for (int i = 0; i < data->count; i++) {
        const fsi_msr_entry_t *e = &data->entries[i];
        if (e->readable) {
            printf("  0x%08X  %-30s = 0x%016llX\n",
                   e->index, e->name, (unsigned long long)e->value);
        } else {
            printf("  0x%08X  %-30s = (not readable)\n",
                   e->index, e->name);
        }
    }
}