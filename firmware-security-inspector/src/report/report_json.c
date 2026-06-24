#include "report.h"
#include <stdio.h>
#include <string.h>


static void json_str(FILE *f, const char *s)
{
    fputc('"', f);
    if (!s) { fputc('"', f); return; }
    for (; *s; s++) {
        switch (*s) {
            case '"':  fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\n': fputs("\\n",  f); break;
            case '\r': fputs("\\r",  f); break;
            case '\t': fputs("\\t",  f); break;
            default:
                if ((unsigned char)*s < 0x20)
                    fprintf(f, "\\u%04x", (unsigned char)*s);
                else
                    fputc(*s, f);
                break;
        }
    }
    fputc('"', f);
}

#define JS(key, val)  fprintf(f, "  \"%s\": ", key); json_str(f, val); \
                      fputc(',', f); fputc('\n', f)
#define JB(key, val)  fprintf(f, "  \"%s\": %s,\n", key, (val) ? "true" : "false")
#define JI(key, val)  fprintf(f, "  \"%s\": %d,\n", key, (int)(val))
#define JX(key, val)  fprintf(f, "  \"%s\": \"0x%llX\",\n", key, \
                              (unsigned long long)(val))

int fsi_report_write_json(const fsi_system_snapshot_t *snap, FILE *f)
{
    fprintf(f, "{\n");
    fprintf(f, "\"fsi_report\": {\n");

    /* Metadata */
    fprintf(f, "\"metadata\": {\n");
    JS("timestamp", snap->timestamp);
    JS("hostname",  snap->hostname);
    fprintf(f, "\"version\": \"1.0\"\n},\n");

    /* Security Score */
    fprintf(f, "\"security_audit\": {\n");
    JI("score", snap->audit.score);
    JS("grade", snap->audit.grade);
    fprintf(f, "\"checks\": [\n");
    for (int i = 0; i < snap->audit.check_count; i++) {
        const fsi_audit_check_t *c = &snap->audit.checks[i];
        fprintf(f, "  {\n");
        fprintf(f, "    \"name\": "); json_str(f, c->name); fputs(",\n", f);
        fprintf(f, "    \"result\": \"%s\",\n",
                fsi_audit_result_str(c->result));
        fprintf(f, "    \"score\": %d,\n", c->score_contribution);
        fprintf(f, "    \"max\": %d,\n", c->max_contribution);
        fprintf(f, "    \"recommendation\": ");
        json_str(f, c->recommendation ? c->recommendation : "");
        fprintf(f, "\n  }%s\n", i < snap->audit.check_count - 1 ? "," : "");
    }
    fprintf(f, "]\n},\n");

    /* CPU */
    fprintf(f, "\"cpu\": {\n");
    JS("vendor",        snap->cpu.vendor);
    JS("brand",         snap->cpu.brand);
    JS("microarch",     snap->cpu.microarch);
    JI("family",        snap->cpu.effective_family);
    JI("model",         snap->cpu.effective_model);
    JI("stepping",      snap->cpu.stepping);
    JI("physical_cores",snap->cpu.physical_cores);
    JI("logical_cores", snap->cpu.logical_cores);
    fprintf(f, "\"features\": {\n");
    JB("nx",      snap->cpu.features.nx);
    JB("smep",    snap->cpu.features.smep);
    JB("smap",    snap->cpu.features.smap);
    JB("umip",    snap->cpu.features.umip);
    JB("cet_ss",  snap->cpu.features.cet_ss);
    JB("cet_ibt", snap->cpu.features.cet_ibt);
    JB("aes_ni",  snap->cpu.features.aes_ni);
    JB("vtx",     snap->cpu.features.vtx);
    JB("amd_v",   snap->cpu.features.amd_v);
    JB("sgx",     snap->cpu.features.sgx);
    JB("txt",     snap->cpu.features.txt);
    JB("avx",     snap->cpu.features.avx);
    JB("avx2",    snap->cpu.features.avx2);
    fprintf(f, "\"avx512\": %s\n", snap->cpu.features.avx512f ? "true" : "false");
    fprintf(f, "}\n},\n");

    /* Firmware */
    fprintf(f, "\"firmware\": {\n");
    JS("boot_mode",   snap->firmware.boot_mode_str);
    JS("bios_vendor", snap->firmware.bios_vendor);
    JS("bios_version",snap->firmware.bios_version);
    JS("bios_date",   snap->firmware.bios_date);
    JS("manufacturer",snap->firmware.system_manufacturer);
    fprintf(f, "\"product\": ");
    json_str(f, snap->firmware.system_product);
    fprintf(f, "\n},\n");

    /* Secure Boot */
    fprintf(f, "\"secure_boot\": {\n");
    JS("state",    snap->secureboot.state_str);
    JB("pk",       snap->secureboot.pk_present);
    JB("kek",      snap->secureboot.kek_present);
    JB("db",       snap->secureboot.db_present);
    fprintf(f, "\"dbx\": %s\n",
            snap->secureboot.dbx_present ? "true" : "false");
    fprintf(f, "},\n");

    /* TPM */
    fprintf(f, "\"tpm\": {\n");
    JB("present",    snap->tpm.present);
    JS("version",    snap->tpm.version_str);
    JS("manufacturer",snap->tpm.manufacturer);
    JB("enabled",    snap->tpm.enabled);
    fprintf(f, "\"activated\": %s\n",
            snap->tpm.activated ? "true" : "false");
    fprintf(f, "},\n");

    /* SMBIOS */
    fprintf(f, "\"smbios\": {\n");
    JS("bios_vendor",  snap->smbios.bios_vendor);
    JS("bios_version", snap->smbios.bios_version);
    JS("system_uuid",  snap->smbios.system_uuid);
    fprintf(f, "\"smbios_version\": \"%u.%u\"\n",
            snap->smbios.smbios_major, snap->smbios.smbios_minor);
    fprintf(f, "},\n");

    /* ACPI */
    fprintf(f, "\"acpi\": {\n");
    fprintf(f, "\"tables\": [\n");
    for (int i = 0; i < snap->acpi.count; i++) {
        const fsi_acpi_table_entry_t *t = &snap->acpi.tables[i];
        fprintf(f, "  {\"signature\": ");
        json_str(f, t->signature);
        fprintf(f, ", \"length\": %u, \"revision\": %u, "
                   "\"checksum_valid\": %s}%s\n",
                t->length, t->revision,
                t->checksum_valid ? "true" : "false",
                i < snap->acpi.count - 1 ? "," : "");
    }
    fprintf(f, "]\n},\n");

    /* Memory */
    fprintf(f, "\"memory\": {\n");
    fprintf(f, "\"total_usable_gb\": %.2f,\n",
            (double)snap->memory.total_usable_bytes /
            (1024.0 * 1024.0 * 1024.0));
    fprintf(f, "\"regions\": %d\n},\n", snap->memory.count);

    /* PCIe */
    fprintf(f, "\"pcie\": {\n");
    fprintf(f, "\"device_count\": %d\n}\n", snap->pcie.count);

    fprintf(f, "}\n}\n");
    return 0;
}