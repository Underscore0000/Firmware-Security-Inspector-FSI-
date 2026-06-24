#include "smbios.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
 * On Linux: SMBIOS accessible via:
 * /sys/firmware/dmi/tables/DMI  -> raw SMBIOS structures
 * /sys/firmware/dmi/tables/smbios_entry_point -> entry point
 * /sys/class/dmi/id/            -> decoded individual fields
 */

#define DMI_PATH "/sys/class/dmi/id"

static bool dmi_read(const char *field, char *buf, size_t size)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", DMI_PATH, field);

    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;

    ssize_t n = read(fd, buf, (ssize_t)(size - 1));
    close(fd);

    if (n <= 0) {
        buf[0] = '\0';
        return false;
    }
    buf[n] = '\0';
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';
    return true;
}

int fsi_smbios_collect(fsi_smbios_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#if defined(__linux__)
    if (access(DMI_PATH, R_OK) != 0) {
        info->available = false;
        snprintf(info->error_msg, sizeof(info->error_msg),
                 "Cannot access %s: %s", DMI_PATH, strerror(errno));
        return 0;
    }

    info->available = true;

    dmi_read("bios_vendor",    info->bios_vendor,   sizeof(info->bios_vendor));
    dmi_read("bios_version",   info->bios_version,  sizeof(info->bios_version));
    dmi_read("bios_date",      info->bios_release_date,
             sizeof(info->bios_release_date));
    dmi_read("sys_vendor",     info->system_manufacturer,
             sizeof(info->system_manufacturer));
    dmi_read("product_name",   info->system_product,
             sizeof(info->system_product));
    dmi_read("product_version",info->system_version,
             sizeof(info->system_version));
    dmi_read("product_serial", info->system_serial,
             sizeof(info->system_serial));
    dmi_read("product_uuid",   info->system_uuid,
             sizeof(info->system_uuid));
    dmi_read("board_vendor",   info->baseboard_manufacturer,
             sizeof(info->baseboard_manufacturer));
    dmi_read("board_name",     info->baseboard_product,
             sizeof(info->baseboard_product));
    dmi_read("board_serial",   info->baseboard_serial,
             sizeof(info->baseboard_serial));
    dmi_read("chassis_vendor", info->chassis_manufacturer,
             sizeof(info->chassis_manufacturer));
    dmi_read("chassis_version",info->chassis_version,
             sizeof(info->chassis_version));

    char ep_path[] = "/sys/firmware/dmi/tables/smbios_entry_point";
    int ep_fd = open(ep_path, O_RDONLY);
    if (ep_fd >= 0) {
        uint8_t ep[32] = {0};
        if (read(ep_fd, ep, sizeof(ep)) >= 8) {
            if (memcmp(ep, "_SM3_", 5) == 0) {
                info->smbios_major = ep[7];
                info->smbios_minor = ep[8];
            } else if (memcmp(ep, "_SM_", 4) == 0) {
                info->smbios_major = ep[6];
                info->smbios_minor = ep[7];
            }
        }
        close(ep_fd);
    }
#else
    info->available = false;
    snprintf(info->error_msg, sizeof(info->error_msg),
             "SMBIOS not accessible on this platform.");
#endif

    return 0;
}

void fsi_smbios_print(const fsi_smbios_info_t *info)
{
    printf("=== SMBIOS Information ===\n");
    if (!info->available) {
        printf("Not available: %s\n", info->error_msg);
        return;
    }
    if (info->smbios_major > 0)
        printf("SMBIOS Version:  %u.%u\n",
               info->smbios_major, info->smbios_minor);
    printf("BIOS Vendor:     %s\n", info->bios_vendor);
    printf("BIOS Version:    %s\n", info->bios_version);
    printf("BIOS Date:       %s\n", info->bios_release_date);
    printf("Manufacturer:    %s\n", info->system_manufacturer);
    printf("Product:         %s\n", info->system_product);
    printf("Version:         %s\n", info->system_version);
    printf("Serial:          %s\n", info->system_serial);
    printf("UUID:            %s\n", info->system_uuid);
    printf("Board Vendor:    %s\n", info->baseboard_manufacturer);
    printf("Board Product:   %s\n", info->baseboard_product);
    printf("Board Serial:    %s\n", info->baseboard_serial);
    printf("Chassis Vendor:  %s\n", info->chassis_manufacturer);
}