#include "acpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define ACPI_TABLES_PATH "/sys/firmware/acpi/tables"

bool fsi_acpi_verify_checksum(const void *table, size_t length)
{
    const uint8_t *p = (const uint8_t *)table;
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += p[i];
    }
    return sum == 0;
}

const char *fsi_acpi_table_description(const char *sig)
{
    if (!sig) return "";
    if (strncmp(sig, "FACP", 4) == 0)
        return "Fixed ACPI Description Table: DSDT address, PM ports, ACPI timer and power management capabilities.";
    if (strncmp(sig, "DSDT", 4) == 0)
        return "Differentiated System Description Table: AML code describing hardware devices, power management methods and notifications.";
    if (strncmp(sig, "SSDT", 4) == 0)
        return "Secondary System Description Table: additional AML tables for CPU P-states and optional devices.";
    if (strncmp(sig, "APIC", 4) == 0 || strncmp(sig, "MADT", 4) == 0)
        return "Multiple APIC Description Table: interrupt controllers (Local APIC, I/O APIC, interrupt overrides).";
    if (strncmp(sig, "HPET", 4) == 0)
        return "High Precision Event Timer Table: HPET hardware description.";
    if (strncmp(sig, "MCFG", 4) == 0)
        return "PCI Express Memory Mapped Config Space Table: MMIO base address for PCIe configuration.";
    if (strncmp(sig, "DMAR", 4) == 0)
        return "DMA Remapping Table (Intel VT-d): Intel IOMMU description. Critical for DMA security.";
    if (strncmp(sig, "IVRS", 4) == 0)
        return "I/O Virtualization Reporting Structure (AMD-Vi/IOMMU).";
    if (strncmp(sig, "TPM2", 4) == 0)
        return "TPM2 Table: TPM 2.0 presence and configuration.";
    if (strncmp(sig, "BGRT", 4) == 0)
        return "Boot Graphics Resource Table: UEFI boot logo.";
    if (strncmp(sig, "WSMT", 4) == 0)
        return "Windows SMM Security Mitigations Table: SMM mitigations implemented by firmware.";
    if (strncmp(sig, "SRAT", 4) == 0)
        return "System Resource Affinity Table: CPU/memory NUMA affinity.";
    if (strncmp(sig, "SLIT", 4) == 0)
        return "System Locality Distance Information Table: NUMA latencies.";
    if (strncmp(sig, "LPIT", 4) == 0)
        return "Low Power Idle Table: CPU low-power idle states.";
    return "ACPI table (description not available).";
}

#if defined(__linux__)
#include <dirent.h>
#include <sys/stat.h>

static int read_acpi_table_header(const char *path, fsi_acpi_table_entry_t *entry)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    acpi_table_header_t hdr;
    ssize_t n = read(fd, &hdr, sizeof(hdr));

    if (n < (ssize_t)sizeof(hdr)) {
        close(fd);
        return -1;
    }

    lseek(fd, 0, SEEK_SET);
    uint8_t *table_data = malloc(hdr.length);
    bool checksum_ok = false;
    if (table_data && (size_t)hdr.length >= sizeof(hdr)) {
        n = read(fd, table_data, hdr.length);
        if (n == (ssize_t)hdr.length) {
            checksum_ok = fsi_acpi_verify_checksum(table_data, hdr.length);
        }
        free(table_data);
    }
    close(fd);

    memcpy(entry->signature, hdr.signature, 4);
    entry->signature[4] = '\0';
    entry->length = hdr.length;
    entry->revision = hdr.revision;
    memcpy(entry->oem_id, hdr.oem_id, 6);
    entry->oem_id[6] = '\0';
    memcpy(entry->oem_table_id, hdr.oem_table_id, 8);
    entry->oem_table_id[8] = '\0';
    entry->checksum_valid = checksum_ok;
    entry->address = 0;

    return 0;
}
#endif

int fsi_acpi_collect(fsi_acpi_data_t *data)
{
    if (!data) return -1;
    memset(data, 0, sizeof(*data));

#if defined(__linux__)
    DIR *dir = opendir(ACPI_TABLES_PATH);
    if (!dir) {
        data->available = false;
        snprintf(data->error_msg, sizeof(data->error_msg),
                 "Cannot open %s: %s", ACPI_TABLES_PATH, strerror(errno));
        return 0;
    }

    int count = 0;
    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (de->d_name[0] == '.') continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", ACPI_TABLES_PATH, de->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            count++;
        }
    }

    if (count == 0) {
        closedir(dir);
        data->available = false;
        snprintf(data->error_msg, sizeof(data->error_msg),
                 "No ACPI tables found in %s.", ACPI_TABLES_PATH);
        return 0;
    }

    data->tables = calloc((size_t)count, sizeof(fsi_acpi_table_entry_t));
    if (!data->tables) {
        closedir(dir);
        return -1;
    }

    rewinddir(dir);
    int idx = 0;
    while ((de = readdir(dir)) != NULL && idx < count) {
        if (de->d_name[0] == '.') continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", ACPI_TABLES_PATH, de->d_name);
        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        if (read_acpi_table_header(path, &data->tables[idx]) == 0) {
            if (idx == 0) {
                strncpy(data->oem_id, data->tables[idx].oem_id,
                        sizeof(data->oem_id) - 1);
            }
            idx++;
        }
    }
    closedir(dir);

    data->count = idx;
    data->available = true;

    for (int i = 0; i < data->count; i++) {
        if (strncmp(data->tables[i].signature, "FACP", 4) == 0) {
            data->acpi_revision = data->tables[i].revision;
            break;
        }
    }
#else
    data->available = false;
    snprintf(data->error_msg, sizeof(data->error_msg),
             "ACPI reading not supported on this platform.");
#endif

    return 0;
}

void fsi_acpi_free(fsi_acpi_data_t *data)
{
    if (data && data->tables) {
        free(data->tables);
        data->tables = NULL;
        data->count = 0;
    }
}

void fsi_acpi_print(const fsi_acpi_data_t *data)
{
    printf("=== ACPI Tables ===\n");
    if (!data->available) {
        printf("Not available: %s\n", data->error_msg);
        return;
    }
    printf("OEM ID:       %s\n", data->oem_id);
    printf("ACPI Rev:     %u\n", data->acpi_revision);
    printf("Tables found: %d\n\n", data->count);
    printf("%-8s %-10s %-4s %-8s %-8s  %s\n",
           "Sig", "Length", "Rev", "OEM ID", "Checksum", "Description");
    printf("%-8s %-10s %-4s %-8s %-8s  %s\n",
           "---", "------", "---", "------", "--------", "-----------");
    for (int i = 0; i < data->count; i++) {
        const fsi_acpi_table_entry_t *t = &data->tables[i];
        printf("%-8s %-10u %-4u %-8s %-8s  %s\n",
               t->signature,
               t->length,
               t->revision,
               t->oem_id,
               t->checksum_valid ? "OK" : "FAIL",
               fsi_acpi_table_description(t->signature));
    }
}