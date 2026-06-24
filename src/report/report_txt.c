#include "report.h"
#include <stdio.h>
#include <string.h>

static void separator(FILE *f) {
    fprintf(f, "%-72s\n",
            "========================================================================");
}

static void section(FILE *f, const char *title) {
    separator(f);
    fprintf(f, "  %s\n", title);
    separator(f);
}

int fsi_report_write_txt(const fsi_system_snapshot_t *snap, FILE *f)
{
    separator(f);
    fprintf(f, "  FIRMWARE SECURITY INSPECTOR - SECURITY REPORT\n");
    separator(f);
    fprintf(f, "  Generated : %s\n", snap->timestamp);
    fprintf(f, "  Host      : %s\n\n", snap->hostname);

    /* Score */
    fprintf(f, "  SECURITY SCORE: %d/100  [Grade: %s]\n\n",
            snap->audit.score, snap->audit.grade);

    /* Checks */
    section(f, "SECURITY AUDIT CHECKS");
    for (int i = 0; i < snap->audit.check_count; i++) {
        const fsi_audit_check_t *c = &snap->audit.checks[i];
        fprintf(f, "  [%-7s] %-24s %+d/%-2d  %s\n",
                fsi_audit_result_str(c->result),
                c->name,
                c->score_contribution,
                c->max_contribution,
                c->recommendation ? c->recommendation : "");
    }
    fprintf(f, "\n");

    /* CPU */
    section(f, "CPU");
    fprintf(f, "  Vendor        : %s\n", snap->cpu.vendor);
    fprintf(f, "  Brand         : %s\n", snap->cpu.brand);
    fprintf(f, "  Microarch     : %s\n", snap->cpu.microarch);
    fprintf(f, "  Family/Model  : 0x%X / 0x%X\n",
            snap->cpu.effective_family, snap->cpu.effective_model);
    fprintf(f, "  Stepping      : %u\n", snap->cpu.stepping);
    fprintf(f, "  Cores         : %d physical, %d logical\n\n",
            snap->cpu.physical_cores, snap->cpu.logical_cores);

    section(f, "CPU SECURITY FEATURES");
#define FEAT(n, v) fprintf(f, "  %-14s : %s\n", n, (v) ? "YES" : "NO")
    FEAT("NX/XD",      snap->cpu.features.nx);
    FEAT("SMEP",       snap->cpu.features.smep);
    FEAT("SMAP",       snap->cpu.features.smap);
    FEAT("UMIP",       snap->cpu.features.umip);
    FEAT("CET-SS",     snap->cpu.features.cet_ss);
    FEAT("CET-IBT",    snap->cpu.features.cet_ibt);
    FEAT("AES-NI",     snap->cpu.features.aes_ni);
    FEAT("VT-x/AMD-V", snap->cpu.features.vtx || snap->cpu.features.amd_v);
    FEAT("SGX",        snap->cpu.features.sgx);
    FEAT("TXT",        snap->cpu.features.txt);
    FEAT("AVX2",       snap->cpu.features.avx2);
    FEAT("AVX-512",    snap->cpu.features.avx512f);
#undef FEAT
    fprintf(f, "\n");

    section(f, "FIRMWARE");
    fprintf(f, "  Boot Mode     : %s\n", snap->firmware.boot_mode_str);
    fprintf(f, "  BIOS Vendor   : %s\n", snap->firmware.bios_vendor);
    fprintf(f, "  BIOS Version  : %s\n", snap->firmware.bios_version);
    fprintf(f, "  BIOS Date     : %s\n", snap->firmware.bios_date);
    fprintf(f, "  Manufacturer  : %s\n", snap->firmware.system_manufacturer);
    fprintf(f, "  Product       : %s\n\n", snap->firmware.system_product);

    section(f, "SECURE BOOT");
    fprintf(f, "  State         : %s\n", snap->secureboot.state_str);
    fprintf(f, "  PK Present    : %s\n",
            snap->secureboot.pk_present  ? "YES" : "NO");
    fprintf(f, "  KEK Present   : %s\n",
            snap->secureboot.kek_present ? "YES" : "NO");
    fprintf(f, "  DB Present    : %s\n",
            snap->secureboot.db_present  ? "YES" : "NO");
    fprintf(f, "  DBX Present   : %s\n\n",
            snap->secureboot.dbx_present ? "YES" : "NO");

    section(f, "TPM");
    fprintf(f, "  Present       : %s\n", snap->tpm.present ? "YES" : "NO");
    fprintf(f, "  Version       : %s\n", snap->tpm.version_str);
    fprintf(f, "  Manufacturer  : %s\n", snap->tpm.manufacturer);
    fprintf(f, "  Enabled       : %s\n\n",
            snap->tpm.enabled ? "YES" : "NO");

    section(f, "SMBIOS");
    fprintf(f, "  BIOS Vendor   : %s\n", snap->smbios.bios_vendor);
    fprintf(f, "  BIOS Version  : %s\n", snap->smbios.bios_version);
    fprintf(f, "  Manufacturer  : %s\n", snap->smbios.system_manufacturer);
    fprintf(f, "  Product       : %s\n", snap->smbios.system_product);
    fprintf(f, "  UUID          : %s\n\n", snap->smbios.system_uuid);

    section(f, "ACPI TABLES");
    fprintf(f, "  %-8s %-10s %-4s %-8s  %-8s  %s\n",
            "Sig", "Length", "Rev", "OEM ID", "Checksum", "Description");
    for (int i = 0; i < snap->acpi.count; i++) {
        const fsi_acpi_table_entry_t *t = &snap->acpi.tables[i];
        fprintf(f, "  %-8s %-10u %-4u %-8s  %-8s  %s\n",
                t->signature, t->length, t->revision, t->oem_id,
                t->checksum_valid ? "OK" : "FAIL",
                fsi_acpi_table_description(t->signature));
    }
    fprintf(f, "\n");

    section(f, "MEMORY MAP");
    fprintf(f, "  Total Usable: %.2f GB\n",
            (double)snap->memory.total_usable_bytes /
            (1024.0 * 1024.0 * 1024.0));
    for (int i = 0; i < snap->memory.count; i++) {
        const fsi_mem_range_t *r = &snap->memory.ranges[i];
        fprintf(f, "  0x%016llX  0x%014llX  %s\n",
                (unsigned long long)r->base,
                (unsigned long long)r->length,
                r->type_str);
    }
    fprintf(f, "\n");

    section(f, "PCI/PCIE DEVICES");
    fprintf(f, "  %-8s %-8s %-8s %-24s %-6s  %s\n",
            "BDF", "VendorID", "DevID", "Vendor", "Type", "Class");
    for (int i = 0; i < snap->pcie.count; i++) {
        const fsi_pci_device_t *d = &snap->pcie.devices[i];
        fprintf(f, "  %02X:%02X.%X  0x%04X   0x%04X   %-24s %-6s  %s\n",
                d->bus, d->device, d->function,
                d->vendor_id, d->device_id,
                d->vendor_name,
                d->is_pcie ? "PCIe" : "PCI",
                d->class_name);
    }
    fprintf(f, "\n");

    separator(f);
    fprintf(f, "  END OF REPORT\n");
    separator(f);
    return 0;
}