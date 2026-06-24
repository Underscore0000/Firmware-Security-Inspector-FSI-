#ifndef FSI_SMBIOS_H
#define FSI_SMBIOS_H

#include <stdint.h>
#include <stdbool.h>


#define SMBIOS_TYPE_BIOS_INFO        0
#define SMBIOS_TYPE_SYSTEM_INFO      1
#define SMBIOS_TYPE_BASEBOARD_INFO   2
#define SMBIOS_TYPE_CHASSIS_INFO     3
#define SMBIOS_TYPE_PROCESSOR_INFO   4
#define SMBIOS_TYPE_MEMORY_ARRAY     16
#define SMBIOS_TYPE_MEMORY_DEVICE    17
#define SMBIOS_TYPE_END_OF_TABLE     127

typedef struct {
    char     bios_vendor[128];
    char     bios_version[128];
    char     bios_release_date[64];
    uint8_t  bios_major_release;
    uint8_t  bios_minor_release;
    char     system_manufacturer[128];
    char     system_product[128];
    char     system_version[128];
    char     system_serial[128];
    char     system_uuid[37];
    char     baseboard_manufacturer[128];
    char     baseboard_product[128];
    char     baseboard_serial[128];
    char     chassis_manufacturer[128];
    char     chassis_version[128];
    uint8_t  smbios_major;
    uint8_t  smbios_minor;
    bool     available;
    char     error_msg[256];
} fsi_smbios_info_t;

int  fsi_smbios_collect(fsi_smbios_info_t *info);
void fsi_smbios_print(const fsi_smbios_info_t *info);

#endif 