#ifndef FSI_REPORT_H
#define FSI_REPORT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cpu.h"
#include "firmware.h"
#include "secureboot.h"
#include "tpm.h"
#include "acpi.h"
#include "smbios.h"
#include "pcie.h"
#include "memory.h"
#include "security_audit.h"

typedef enum {
    REPORT_FORMAT_JSON = 0,
    REPORT_FORMAT_HTML,
    REPORT_FORMAT_TXT
} fsi_report_format_t;

typedef struct {
    fsi_cpu_info_t          cpu;
    fsi_firmware_info_t     firmware;
    fsi_secureboot_info_t   secureboot;
    fsi_tpm_info_t          tpm;
    fsi_acpi_data_t         acpi;
    fsi_smbios_info_t       smbios;
    fsi_pcie_data_t         pcie;
    fsi_memory_map_t        memory;
    fsi_msr_data_t          msr;
    fsi_registers_t         registers;
    fsi_audit_report_t      audit;
    char                    timestamp[64];
    char                    hostname[256];
} fsi_system_snapshot_t;

int  fsi_system_collect_all(fsi_system_snapshot_t *snap);
void fsi_system_snapshot_free(fsi_system_snapshot_t *snap);
int  fsi_report_generate(const fsi_system_snapshot_t *snap,
                          fsi_report_format_t format,
                          const char *output_path);


int fsi_report_write_json(const fsi_system_snapshot_t *snap, FILE *f);
int fsi_report_write_html(const fsi_system_snapshot_t *snap, FILE *f);
int fsi_report_write_txt (const fsi_system_snapshot_t *snap, FILE *f);

#endif 