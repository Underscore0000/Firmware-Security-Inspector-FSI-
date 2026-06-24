#include "report.h"
#include <stdio.h>
#include <string.h>

static const char *result_badge(fsi_audit_result_t r)
{
    switch (r) {
        case AUDIT_PASS:    return "badge-pass";
        case AUDIT_WARNING: return "badge-warn";
        case AUDIT_FAIL:    return "badge-fail";
        default:            return "badge-info";
    }
}

int fsi_report_write_html(const fsi_system_snapshot_t *snap, FILE *f)
{
    fprintf(f,
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><meta charset=\"UTF-8\">\n"
        "<title>FSI Security Report</title>\n"
        "<style>\n"
        "body{font-family:monospace;background:#1e1e1e;color:#d4d4d4;"
        "margin:20px;font-size:13px}\n"
        "h1{color:#569cd6;border-bottom:1px solid #333;padding-bottom:8px}\n"
        "h2{color:#9cdcfe;margin-top:24px;font-size:14px}\n"
        "table{border-collapse:collapse;width:100%%;margin-bottom:16px}\n"
        "th{background:#252526;color:#9cdcfe;text-align:left;"
        "padding:6px 10px;border:1px solid #333}\n"
        "td{padding:5px 10px;border:1px solid #2d2d2d}\n"
        "tr:nth-child(even){background:#252526}\n"
        ".score-box{background:#252526;border:1px solid #333;"
        "padding:16px;margin-bottom:16px;display:inline-block;"
        "min-width:200px}\n"
        ".score-num{font-size:48px;color:#569cd6;font-weight:bold}\n"
        ".grade{font-size:32px;margin-left:16px;color:#9cdcfe}\n"
        ".badge-pass{color:#4ec9b0;font-weight:bold}\n"
        ".badge-warn{color:#dcdcaa;font-weight:bold}\n"
        ".badge-fail{color:#f44747;font-weight:bold}\n"
        ".badge-info{color:#9cdcfe;font-weight:bold}\n"
        ".meta{color:#6a9955;font-size:11px;margin-bottom:16px}\n"
        "</style></head><body>\n"
        "<h1>Firmware Security Inspector &mdash; Security Report</h1>\n"
        "<div class=\"meta\">Generated: %s &nbsp;|&nbsp; Host: %s</div>\n",
        snap->timestamp, snap->hostname);

    /* Score */
    fprintf(f,
        "<div class=\"score-box\">\n"
        "<span class=\"score-num\">%d</span>"
        "<span>/100</span>"
        "<span class=\"grade\">[%s]</span>\n"
        "</div>\n",
        snap->audit.score, snap->audit.grade);

    /* Audit checks table */
    fprintf(f, "<h2>Security Checks</h2>\n"
               "<table><tr><th>Check</th><th>Result</th>"
               "<th>Score</th><th>Recommendation</th></tr>\n");
    for (int i = 0; i < snap->audit.check_count; i++) {
        const fsi_audit_check_t *c = &snap->audit.checks[i];
        fprintf(f, "<tr><td>%s</td>"
                   "<td class=\"%s\">%s</td>"
                   "<td>%d/%d</td>"
                   "<td>%s</td></tr>\n",
                c->name,
                result_badge(c->result),
                fsi_audit_result_str(c->result),
                c->score_contribution, c->max_contribution,
                c->recommendation ? c->recommendation : "");
    }
    fprintf(f, "</table>\n");

    /* CPU */
    fprintf(f, "<h2>CPU</h2>\n<table>\n");
    fprintf(f, "<tr><th>Field</th><th>Value</th></tr>\n");
    fprintf(f, "<tr><td>Vendor</td><td>%s</td></tr>\n", snap->cpu.vendor);
    fprintf(f, "<tr><td>Brand</td><td>%s</td></tr>\n",  snap->cpu.brand);
    fprintf(f, "<tr><td>Microarchitecture</td><td>%s</td></tr>\n",
            snap->cpu.microarch);
    fprintf(f, "<tr><td>Family/Model/Step</td><td>%u/%u/%u</td></tr>\n",
            snap->cpu.effective_family, snap->cpu.effective_model,
            snap->cpu.stepping);
    fprintf(f, "<tr><td>Cores</td><td>%d physical / %d logical</td></tr>\n",
            snap->cpu.physical_cores, snap->cpu.logical_cores);
    fprintf(f, "</table>\n");

    /* Security Features */
    fprintf(f, "<h2>CPU Security Features</h2>\n<table>\n");
    fprintf(f, "<tr><th>Feature</th><th>Status</th></tr>\n");
#define FEAT_ROW(name, val) \
    fprintf(f, "<tr><td>%s</td><td class=\"%s\">%s</td></tr>\n", \
            name, (val) ? "badge-pass" : "badge-fail", \
            (val) ? "YES" : "NO");
    FEAT_ROW("NX/XD",      snap->cpu.features.nx)
    FEAT_ROW("SMEP",       snap->cpu.features.smep)
    FEAT_ROW("SMAP",       snap->cpu.features.smap)
    FEAT_ROW("UMIP",       snap->cpu.features.umip)
    FEAT_ROW("CET-SS",     snap->cpu.features.cet_ss)
    FEAT_ROW("CET-IBT",    snap->cpu.features.cet_ibt)
    FEAT_ROW("AES-NI",     snap->cpu.features.aes_ni)
    FEAT_ROW("VT-x/AMD-V", snap->cpu.features.vtx || snap->cpu.features.amd_v)
    FEAT_ROW("SGX",        snap->cpu.features.sgx)
    FEAT_ROW("TXT",        snap->cpu.features.txt)
    FEAT_ROW("AVX2",       snap->cpu.features.avx2)
    FEAT_ROW("AVX-512",    snap->cpu.features.avx512f)
#undef FEAT_ROW
    fprintf(f, "</table>\n");

    /* Firmware */
    fprintf(f, "<h2>Firmware</h2>\n<table>\n");
    fprintf(f, "<tr><th>Field</th><th>Value</th></tr>\n");
    fprintf(f, "<tr><td>Boot Mode</td><td>%s</td></tr>\n",
            snap->firmware.boot_mode_str);
    fprintf(f, "<tr><td>BIOS Vendor</td><td>%s</td></tr>\n",
            snap->firmware.bios_vendor);
    fprintf(f, "<tr><td>BIOS Version</td><td>%s</td></tr>\n",
            snap->firmware.bios_version);
    fprintf(f, "<tr><td>BIOS Date</td><td>%s</td></tr>\n",
            snap->firmware.bios_date);
    fprintf(f, "<tr><td>Manufacturer</td><td>%s</td></tr>\n",
            snap->firmware.system_manufacturer);
    fprintf(f, "<tr><td>Product</td><td>%s</td></tr>\n",
            snap->firmware.system_product);
    fprintf(f, "</table>\n");

    /* Secure Boot */
    fprintf(f, "<h2>Secure Boot</h2>\n<table>\n");
    fprintf(f, "<tr><th>Field</th><th>Value</th></tr>\n");
    const char *sb_class =
        (snap->secureboot.state == SB_STATE_ENABLED) ? "badge-pass" :
        (snap->secureboot.state == SB_STATE_DISABLED) ? "badge-fail" :
        "badge-warn";
    fprintf(f, "<tr><td>State</td><td class=\"%s\">%s</td></tr>\n",
            sb_class, snap->secureboot.state_str);
    fprintf(f, "<tr><td>PK</td><td>%s</td></tr>\n",
            snap->secureboot.pk_present  ? "Present" : "Absent");
    fprintf(f, "<tr><td>KEK</td><td>%s</td></tr>\n",
            snap->secureboot.kek_present ? "Present" : "Absent");
    fprintf(f, "<tr><td>DB</td><td>%s</td></tr>\n",
            snap->secureboot.db_present  ? "Present" : "Absent");
    fprintf(f, "<tr><td>DBX</td><td>%s</td></tr>\n",
            snap->secureboot.dbx_present ? "Present" : "Absent");
    fprintf(f, "</table>\n");

    /* TPM */
    fprintf(f, "<h2>TPM</h2>\n<table>\n");
    fprintf(f, "<tr><th>Field</th><th>Value</th></tr>\n");
    fprintf(f, "<tr><td>Present</td><td>%s</td></tr>\n",
            snap->tpm.present ? "YES" : "NO");
    fprintf(f, "<tr><td>Version</td><td>%s</td></tr>\n",
            snap->tpm.version_str);
    fprintf(f, "<tr><td>Manufacturer</td><td>%s</td></tr>\n",
            snap->tpm.manufacturer);
    fprintf(f, "</table>\n");

    /* ACPI Tables */
    fprintf(f, "<h2>ACPI Tables</h2>\n<table>\n");
    fprintf(f, "<tr><th>Signature</th><th>Length</th>"
               "<th>Revision</th><th>OEM ID</th>"
               "<th>Checksum</th><th>Description</th></tr>\n");
    for (int i = 0; i < snap->acpi.count; i++) {
        const fsi_acpi_table_entry_t *t = &snap->acpi.tables[i];
        fprintf(f, "<tr><td><b>%s</b></td><td>%u</td>"
                   "<td>%u</td><td>%s</td>"
                   "<td class=\"%s\">%s</td>"
                   "<td>%s</td></tr>\n",
                t->signature, t->length, t->revision, t->oem_id,
                t->checksum_valid ? "badge-pass" : "badge-fail",
                t->checksum_valid ? "OK" : "FAIL",
                fsi_acpi_table_description(t->signature));
    }
    fprintf(f, "</table>\n");

    /* Memory */
    fprintf(f, "<h2>Memory Map</h2>\n<table>\n");
    fprintf(f, "<tr><th>Base</th><th>Length</th><th>Type</th></tr>\n");
    for (int i = 0; i < snap->memory.count; i++) {
        const fsi_mem_range_t *r = &snap->memory.ranges[i];
        fprintf(f, "<tr><td>0x%016llX</td><td>0x%016llX</td>"
                   "<td>%s</td></tr>\n",
                (unsigned long long)r->base,
                (unsigned long long)r->length,
                r->type_str);
    }
    fprintf(f, "</table>\n");

    /* PCIe */
    fprintf(f, "<h2>PCI/PCIe Devices (%d)</h2>\n<table>\n",
            snap->pcie.count);
    fprintf(f, "<tr><th>BDF</th><th>Vendor ID</th><th>Device ID</th>"
               "<th>Vendor</th><th>Type</th><th>Class</th></tr>\n");
    for (int i = 0; i < snap->pcie.count; i++) {
        const fsi_pci_device_t *d = &snap->pcie.devices[i];
        fprintf(f, "<tr><td>%02X:%02X.%X</td>"
                   "<td>0x%04X</td><td>0x%04X</td>"
                   "<td>%s</td><td>%s</td><td>%s</td></tr>\n",
                d->bus, d->device, d->function,
                d->vendor_id, d->device_id,
                d->vendor_name,
                d->is_pcie ? "PCIe" : "PCI",
                d->class_name);
    }
    fprintf(f, "</table>\n");

    fprintf(f, "</body></html>\n");
    return 0;
}