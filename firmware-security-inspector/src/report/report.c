/**
 * report.c - FSI report system
 * Handles data collection and report generation in various formats.
 * Compatible with Windows (MinGW64) and Linux.
 */

#include "report.h"
#include "registers.h"
#include "msr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
    #include <windows.h>

    static int win_gethostname(char *name, size_t len) {
        DWORD size = (DWORD)len;
        return GetComputerNameA(name, &size) ? 0 : -1;
    }
    #define gethostname win_gethostname
#else
    #include <unistd.h>
#endif

int fsi_system_collect_all(fsi_system_snapshot_t *snap)
{
    if (!snap) return -1;
    memset(snap, 0, sizeof(*snap));

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(snap->timestamp, sizeof(snap->timestamp),
                 "%Y-%m-%dT%H:%M:%S", tm_info);
    } else {
        strcpy(snap->timestamp, "1970-01-01T00:00:00");
    }

    if (gethostname(snap->hostname, sizeof(snap->hostname) - 1) != 0) {
#ifdef _WIN32
        strcpy(snap->hostname, "Windows-Host");
#else
        strcpy(snap->hostname, "Unknown-Host");
#endif
    }

    fsi_cpu_collect(&snap->cpu);
    fsi_firmware_collect(&snap->firmware);
    fsi_secureboot_collect(&snap->secureboot);
    fsi_tpm_collect(&snap->tpm);
    fsi_acpi_collect(&snap->acpi);
    fsi_smbios_collect(&snap->smbios);
    fsi_pcie_collect(&snap->pcie);
    fsi_memory_collect(&snap->memory);
    fsi_msr_collect(&snap->msr);
    fsi_registers_collect(&snap->registers);

    fsi_audit_run(&snap->audit, &snap->cpu, &snap->secureboot,
                  &snap->tpm, &snap->registers, &snap->msr);

    return 0;
}

void fsi_system_snapshot_free(fsi_system_snapshot_t *snap)
{
    if (!snap) return;
    fsi_acpi_free(&snap->acpi);
    fsi_pcie_free(&snap->pcie);
    fsi_memory_free(&snap->memory);
    fsi_msr_free(&snap->msr);
    fsi_audit_free(&snap->audit);
}

int fsi_report_generate(const fsi_system_snapshot_t *snap,
                        fsi_report_format_t format,
                        const char *output_path)
{
    if (!snap || !output_path) return -1;

    FILE *f = fopen(output_path, "w");
    if (!f) {
        fprintf(stderr, "Cannot open output file %s: %s\n",
                output_path, strerror(errno));
        return -1;
    }

    int ret = 0;
    switch (format) {
        case REPORT_FORMAT_JSON:
            ret = fsi_report_write_json(snap, f);
            break;
        case REPORT_FORMAT_HTML:
            ret = fsi_report_write_html(snap, f);
            break;
        case REPORT_FORMAT_TXT:
            ret = fsi_report_write_txt(snap, f);
            break;
        default:
            ret = -1;
            break;
    }

    fclose(f);
    return ret;
}