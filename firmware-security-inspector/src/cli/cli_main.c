#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "registers.h"
#include "msr.h"
#include "firmware.h"
#include "secureboot.h"
#include "tpm.h"
#include "acpi.h"
#include "smbios.h"
#include "pcie.h"
#include "memory.h"
#include "security_audit.h"
#include "report.h"
#include "snapshot.h"

static void print_usage(const char *prog)
{
    printf("Firmware Security Inspector - CLI\n");
    printf("Usage: %s [COMMAND] [OPTIONS]\n\n", prog);
    printf("Commands:\n");
    printf("  audit              Run security audit and print score\n");
    printf("  cpu                Show CPU information\n");
    printf("  firmware           Show firmware information\n");
    printf("  secureboot         Show Secure Boot status\n");
    printf("  tpm                Show TPM information\n");
    printf("  acpi               List ACPI tables\n");
    printf("  smbios             Show SMBIOS information\n");
    printf("  pcie               List PCI/PCIe devices\n");
    printf("  memory             Show memory map\n");
    printf("  msr                List MSR values\n");
    printf("  report <fmt> <out> Generate report (fmt: json|html|txt)\n");
    printf("  snapshot save <f>  Save system snapshot to file\n");
    printf("  snapshot load <f>  Load and display snapshot\n");
    printf("  snapshot diff <a> <b>  Compare two snapshots\n");
    printf("  all                Run all modules\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "cpu") == 0) {
        fsi_cpu_info_t cpu = {0};
        fsi_cpu_collect(&cpu);
        fsi_cpu_print(&cpu);
        return 0;
    }

    if (strcmp(cmd, "firmware") == 0) {
        fsi_firmware_info_t fw = {0};
        fsi_firmware_collect(&fw);
        fsi_firmware_print(&fw);
        return 0;
    }

    if (strcmp(cmd, "secureboot") == 0) {
        fsi_secureboot_info_t sb = {0};
        fsi_secureboot_collect(&sb);
        fsi_secureboot_print(&sb);
        return 0;
    }

    if (strcmp(cmd, "tpm") == 0) {
        fsi_tpm_info_t tpm = {0};
        fsi_tpm_collect(&tpm);
        fsi_tpm_print(&tpm);
        return 0;
    }

    if (strcmp(cmd, "acpi") == 0) {
        fsi_acpi_data_t acpi = {0};
        fsi_acpi_collect(&acpi);
        fsi_acpi_print(&acpi);
        fsi_acpi_free(&acpi);
        return 0;
    }

    if (strcmp(cmd, "smbios") == 0) {
        fsi_smbios_info_t smbios = {0};
        fsi_smbios_collect(&smbios);
        fsi_smbios_print(&smbios);
        return 0;
    }

    if (strcmp(cmd, "pcie") == 0) {
        fsi_pcie_data_t pcie = {0};
        fsi_pcie_collect(&pcie);
        fsi_pcie_print(&pcie);
        fsi_pcie_free(&pcie);
        return 0;
    }

    if (strcmp(cmd, "memory") == 0) {
        fsi_memory_map_t mem = {0};
        fsi_memory_collect(&mem);
        fsi_memory_print(&mem);
        fsi_memory_free(&mem);
        return 0;
    }

    if (strcmp(cmd, "msr") == 0) {
        fsi_msr_data_t msr = {0};
        fsi_msr_collect(&msr);
        fsi_msr_print(&msr);
        fsi_msr_free(&msr);
        return 0;
    }

    if (strcmp(cmd, "audit") == 0 || strcmp(cmd, "all") == 0) {
        fsi_system_snapshot_t snap = {0};
        fsi_system_collect_all(&snap);

        if (strcmp(cmd, "all") == 0) {
            fsi_cpu_print(&snap.cpu);
            fsi_firmware_print(&snap.firmware);
            fsi_secureboot_print(&snap.secureboot);
            fsi_tpm_print(&snap.tpm);
            fsi_acpi_print(&snap.acpi);
            fsi_smbios_print(&snap.smbios);
            fsi_pcie_print(&snap.pcie);
            fsi_memory_print(&snap.memory);
            fsi_msr_print(&snap.msr);
        }

        fsi_audit_print(&snap.audit);
        fsi_system_snapshot_free(&snap);
        return 0;
    }

    if (strcmp(cmd, "report") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s report <json|html|txt> <output>\n",
                    argv[0]);
            return 1;
        }
        fsi_report_format_t fmt;
        if      (strcmp(argv[2], "json") == 0) fmt = REPORT_FORMAT_JSON;
        else if (strcmp(argv[2], "html") == 0) fmt = REPORT_FORMAT_HTML;
        else if (strcmp(argv[2], "txt")  == 0) fmt = REPORT_FORMAT_TXT;
        else {
            fprintf(stderr, "Unknown format: %s\n", argv[2]);
            return 1;
        }

        fsi_system_snapshot_t snap = {0};
        printf("Collecting system data...\n");
        fsi_system_collect_all(&snap);
        printf("Generating report to %s...\n", argv[3]);
        int ret = fsi_report_generate(&snap, fmt, argv[3]);
        fsi_system_snapshot_free(&snap);
        if (ret == 0)
            printf("Report saved to %s\n", argv[3]);
        return ret;
    }

    if (strcmp(cmd, "snapshot") == 0) {
        if (argc < 4) {
            fprintf(stderr,
                    "Usage: %s snapshot <save|load|diff> <file> [file2]\n",
                    argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "save") == 0) {
            fsi_system_snapshot_t snap = {0};
            printf("Collecting system data...\n");
            fsi_system_collect_all(&snap);
            int ret = fsi_snapshot_save(&snap, argv[3]);
            fsi_system_snapshot_free(&snap);
            if (ret == 0)
                printf("Snapshot saved to %s\n", argv[3]);
            return ret;
        }
        if (strcmp(argv[2], "load") == 0) {
            fsi_system_snapshot_t snap = {0};
            int ret = fsi_snapshot_load(&snap, argv[3]);
            if (ret == 0) {
                printf("Snapshot from: %s @ %s\n",
                       snap.hostname, snap.timestamp);
                fsi_audit_print(&snap.audit);
            }
            fsi_snapshot_free_loaded(&snap);
            return ret;
        }
        if (strcmp(argv[2], "diff") == 0) {
            if (argc < 5) {
                fprintf(stderr, "Usage: %s snapshot diff <fileA> <fileB>\n",
                        argv[0]);
                return 1;
            }
            fsi_system_snapshot_t a = {0}, b = {0};
            if (fsi_snapshot_load(&a, argv[3]) != 0) return 1;
            if (fsi_snapshot_load(&b, argv[4]) != 0) return 1;
            fsi_snapshot_diff(&a, &b);
            fsi_snapshot_free_loaded(&a);
            fsi_snapshot_free_loaded(&b);
            return 0;
        }
        fprintf(stderr, "Unknown snapshot command: %s\n", argv[2]);
        return 1;
    }

    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage(argv[0]);
    return 1;
}