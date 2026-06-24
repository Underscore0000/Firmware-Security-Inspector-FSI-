#ifndef FSI_SECURITY_AUDIT_H
#define FSI_SECURITY_AUDIT_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu.h"
#include "secureboot.h"
#include "tpm.h"
#include "registers.h"
#include "msr.h"

typedef enum {
    AUDIT_PASS    = 0,
    AUDIT_WARNING = 1,
    AUDIT_FAIL    = 2,
    AUDIT_INFO    = 3
} fsi_audit_result_t;

typedef struct {
    const char        *name;
    const char        *description;
    const char        *recommendation;
    fsi_audit_result_t result;
    int                score_contribution;
    int                max_contribution;  
} fsi_audit_check_t;

typedef struct {
    fsi_audit_check_t *checks;
    int                check_count;
    int                score;       
    int                max_score;
    char               grade[4];    
} fsi_audit_report_t;

int  fsi_audit_run(fsi_audit_report_t *report,
                   const fsi_cpu_info_t      *cpu,
                   const fsi_secureboot_info_t *sb,
                   const fsi_tpm_info_t        *tpm,
                   const fsi_registers_t       *regs,
                   const fsi_msr_data_t        *msr);

void fsi_audit_free(fsi_audit_report_t *report);
void fsi_audit_print(const fsi_audit_report_t *report);
const char *fsi_audit_result_str(fsi_audit_result_t r);
const char *fsi_audit_result_color_ansi(fsi_audit_result_t r);

#endif 