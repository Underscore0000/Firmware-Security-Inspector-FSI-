#ifndef FSI_FIRMWARE_H
#define FSI_FIRMWARE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BOOT_MODE_UNKNOWN = 0,
    BOOT_MODE_UEFI,
    BOOT_MODE_LEGACY_BIOS,
    BOOT_MODE_UEFI_CSM   
} fsi_boot_mode_t;

typedef struct {
    fsi_boot_mode_t boot_mode;
    char            boot_mode_str[32];
    char            uefi_vendor[128];
    char            uefi_version[64];
    char            uefi_release_date[32];
    char            bios_vendor[128];
    char            bios_version[64];
    char            bios_date[32];
    char            system_manufacturer[128];
    char            system_product[128];
    char            system_version[64];
    char            system_serial[64];
    bool            secure_boot_capable;
    bool            csm_enabled;
} fsi_firmware_info_t;

int  fsi_firmware_collect(fsi_firmware_info_t *info);
void fsi_firmware_print(const fsi_firmware_info_t *info);
const char *fsi_boot_mode_description(fsi_boot_mode_t mode);

#endif 