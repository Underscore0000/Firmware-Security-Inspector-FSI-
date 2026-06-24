#ifndef FSI_ACPI_H
#define FSI_ACPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#pragma pack(push, 1)


typedef struct {
    char     signature[8];   
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;       
    uint32_t rsdt_address;   
    
    uint32_t length;
    uint64_t xsdt_address;   
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} acpi_rsdp_t;


typedef struct {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    char     creator_id[4];
    uint32_t creator_revision;
} acpi_table_header_t;

#pragma pack(pop)


typedef struct {
    char     signature[5];   
    uint64_t address;        
    uint32_t length;
    uint8_t  revision;
    char     oem_id[7];
    char     oem_table_id[9];
    bool     checksum_valid;
} fsi_acpi_table_entry_t;

typedef struct {
    fsi_acpi_table_entry_t *tables;
    int                     count;
    char                    oem_id[7];
    uint8_t                 acpi_revision;
    uint64_t                rsdp_address;
    uint64_t                xsdt_address;
    bool                    available;
    char                    error_msg[256];
} fsi_acpi_data_t;

int  fsi_acpi_collect(fsi_acpi_data_t *data);
void fsi_acpi_free(fsi_acpi_data_t *data);
void fsi_acpi_print(const fsi_acpi_data_t *data);
bool fsi_acpi_verify_checksum(const void *table, size_t length);
const char *fsi_acpi_table_description(const char *sig);

#endif 