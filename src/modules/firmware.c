#include "firmware.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

static void windows_get_firmware_info(fsi_firmware_info_t *info)
{
    char buffer[256];
    HKEY hKey;
    DWORD dataSize;
    DWORD type;

    strcpy(info->boot_mode_str, "Unknown");
    strcpy(info->bios_vendor, "Unknown");
    strcpy(info->bios_version, "Unknown");
    strcpy(info->bios_date, "Unknown");
    strcpy(info->system_manufacturer, "Unknown");
    strcpy(info->system_product, "Unknown");
    strcpy(info->system_version, "Unknown");
    strcpy(info->system_serial, "Unknown");

    DWORD size = GetFirmwareEnvironmentVariableA(
        "BootCurrent",
        "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}",
        buffer, sizeof(buffer));

    if (size > 0) {
        strcpy(info->boot_mode_str, "UEFI");
        info->boot_mode = BOOT_MODE_UEFI;
        info->secure_boot_capable = true;

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            dataSize = sizeof(buffer);
            if (RegQueryValueExA(hKey, "BIOSVendor", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
                strncpy(info->uefi_vendor, buffer, sizeof(info->uefi_vendor) - 1);
            }

            dataSize = sizeof(buffer);
            if (RegQueryValueExA(hKey, "BIOSVersion", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
                strncpy(info->uefi_version, buffer, sizeof(info->uefi_version) - 1);
            }

            RegCloseKey(hKey);
        }
    } else {
        strcpy(info->boot_mode_str, "Legacy BIOS");
        info->boot_mode = BOOT_MODE_LEGACY_BIOS;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "BIOSVendor", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->bios_vendor, buffer, sizeof(info->bios_vendor) - 1);
        }

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "BIOSVersion", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->bios_version, buffer, sizeof(info->bios_version) - 1);
        }

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "BIOSReleaseDate", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->bios_date, buffer, sizeof(info->bios_date) - 1);
        }

        RegCloseKey(hKey);
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "SystemManufacturer", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->system_manufacturer, buffer, sizeof(info->system_manufacturer) - 1);
        }

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "SystemProductName", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->system_product, buffer, sizeof(info->system_product) - 1);
        }

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "SystemVersion", NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->system_version, buffer, sizeof(info->system_version) - 1);
        }

        RegCloseKey(hKey);
    }

    if (info->boot_mode == BOOT_MODE_UEFI) {
        if (strlen(info->bios_vendor) > 0)
            strncpy(info->uefi_vendor, info->bios_vendor, sizeof(info->uefi_vendor) - 1);
        if (strlen(info->bios_version) > 0)
            strncpy(info->uefi_version, info->bios_version, sizeof(info->uefi_version) - 1);
        if (strlen(info->bios_date) > 0)
            strncpy(info->uefi_release_date, info->bios_date, sizeof(info->uefi_release_date) - 1);
    }
}

#else
static void linux_get_firmware_info(fsi_firmware_info_t *info)
{
    
}
#endif

int fsi_firmware_collect(fsi_firmware_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#ifdef _WIN32
    windows_get_firmware_info(info);
#else
    linux_get_firmware_info(info);
#endif

    return 0;
}

void fsi_firmware_print(const fsi_firmware_info_t *info)
{
    printf("=== Firmware Information ===\n");
    printf("Boot Mode:    %s\n", info->boot_mode_str);
    printf("BIOS Vendor:  %s\n", info->bios_vendor);
    printf("BIOS Version: %s\n", info->bios_version);
    printf("BIOS Date:    %s\n", info->bios_date);
    if (info->boot_mode == BOOT_MODE_UEFI) {
        printf("UEFI Vendor:  %s\n", info->uefi_vendor);
        printf("UEFI Version: %s\n", info->uefi_version);
        printf("UEFI Date:    %s\n", info->uefi_release_date);
    }
    printf("Manufacturer: %s\n", info->system_manufacturer);
    printf("Product:      %s\n", info->system_product);
    printf("Version:      %s\n", info->system_version);
}