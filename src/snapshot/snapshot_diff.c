#include "snapshot.h"
#include "security_audit.h"
#include <stdio.h>
#include <string.h>

/* Confronta due snapshot e stampa differenze */
void fsi_snapshot_diff(const fsi_system_snapshot_t *a,
                       const fsi_system_snapshot_t *b)
{
    printf("=== Snapshot Comparison ===\n");
    printf("A: %s @ %s\n", a->hostname, a->timestamp);
    printf("B: %s @ %s\n\n", b->hostname, b->timestamp);

    int changes = 0;

#define DIFF_STR(section, name, fa, fb) \
    if (strcmp((fa), (fb)) != 0) { \
        printf("[CHANGED] %s.%s:\n  A: %s\n  B: %s\n", \
               section, name, fa, fb); \
        changes++; \
    }

#define DIFF_BOOL(section, name, fa, fb) \
    if ((fa) != (fb)) { \
        printf("[CHANGED] %s.%s: %s -> %s\n", \
               section, name, \
               (fa) ? "YES" : "NO", \
               (fb) ? "YES" : "NO"); \
        changes++; \
    }

#define DIFF_INT(section, name, fa, fb) \
    if ((fa) != (fb)) { \
        printf("[CHANGED] %s.%s: %d -> %d\n", \
               section, name, (int)(fa), (int)(fb)); \
        changes++; \
    }

    /* Security Score */
    DIFF_INT("Security", "score", a->audit.score, b->audit.score)

    /* Secure Boot */
    DIFF_STR("SecureBoot", "state",
             a->secureboot.state_str, b->secureboot.state_str)
    DIFF_BOOL("SecureBoot", "pk",
              a->secureboot.pk_present,  b->secureboot.pk_present)
    DIFF_BOOL("SecureBoot", "kek",
              a->secureboot.kek_present, b->secureboot.kek_present)
    DIFF_BOOL("SecureBoot", "db",
              a->secureboot.db_present,  b->secureboot.db_present)
    DIFF_BOOL("SecureBoot", "dbx",
              a->secureboot.dbx_present, b->secureboot.dbx_present)

    /* TPM */
    DIFF_BOOL("TPM", "present", a->tpm.present, b->tpm.present)
    DIFF_STR ("TPM", "version", a->tpm.version_str, b->tpm.version_str)
    DIFF_BOOL("TPM", "enabled", a->tpm.enabled, b->tpm.enabled)

    /* Firmware */
    DIFF_STR("Firmware", "boot_mode",
             a->firmware.boot_mode_str, b->firmware.boot_mode_str)
    DIFF_STR("Firmware", "bios_version",
             a->firmware.bios_version, b->firmware.bios_version)

    /* CPU Features */
    DIFF_BOOL("CPU", "nx",      a->cpu.features.nx,     b->cpu.features.nx)
    DIFF_BOOL("CPU", "smep",    a->cpu.features.smep,   b->cpu.features.smep)
    DIFF_BOOL("CPU", "smap",    a->cpu.features.smap,   b->cpu.features.smap)
    DIFF_BOOL("CPU", "cet_ss",  a->cpu.features.cet_ss, b->cpu.features.cet_ss)
    DIFF_BOOL("CPU", "cet_ibt", a->cpu.features.cet_ibt,b->cpu.features.cet_ibt)

    /* ACPI table count */
    DIFF_INT("ACPI", "table_count", a->acpi.count, b->acpi.count)

    /* PCIe device count */
    DIFF_INT("PCIe", "device_count", a->pcie.count, b->pcie.count)

    if (changes == 0)
        printf("No differences found between snapshots.\n");
    else
        printf("\nTotal changes: %d\n", changes);
}