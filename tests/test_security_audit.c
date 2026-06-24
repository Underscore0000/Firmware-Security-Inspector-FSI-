#include <stdio.h>
#include <assert.h>
#include "security_audit.h"
#include "cpu.h"
#include "secureboot.h"
#include "tpm.h"
#include "registers.h"
#include "msr.h"

int main() {
    fsi_cpu_info_t cpu = {0};
    fsi_secureboot_info_t sb = {0};
    fsi_tpm_info_t tpm = {0};
    fsi_registers_t regs = {0};
    fsi_msr_data_t msr = {0};
    fsi_audit_report_t report = {0};
    

    fsi_cpu_collect(&cpu);
    fsi_secureboot_collect(&sb);
    fsi_tpm_collect(&tpm);
    fsi_registers_collect(&regs);
    fsi_msr_collect(&msr);
    

    int ret = fsi_audit_run(&report, &cpu, &sb, &tpm, &regs, &msr);
    assert(ret == 0);
    

    fsi_audit_print(&report);
    

    assert(report.score >= 0 && report.score <= 100);
    assert(report.grade[0] >= 'A' && report.grade[0] <= 'F');
    
    printf("All audit tests passed!\n");
    return 0;
}