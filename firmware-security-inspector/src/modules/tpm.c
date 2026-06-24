#include "tpm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

static void windows_get_tpm_info(fsi_tpm_info_t *info)
{
    HKEY hKey;
    char buffer[256];
    DWORD dataSize;

    strcpy(info->version_str, "None");
    strcpy(info->manufacturer, "Unknown");
    strcpy(info->firmware_version, "Unknown");
    strcpy(info->error_msg, "");

    // Check TPM 2.0
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Tpm", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        info->present = true;
        info->version = TPM_VERSION_20;
        strcpy(info->version_str, "2.0");
        info->enabled = true;
        info->activated = true;

        dataSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "Manufacturer", NULL, NULL, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
            strncpy(info->manufacturer, buffer, sizeof(info->manufacturer) - 1);
        } else {
            strcpy(info->manufacturer, "Windows TPM 2.0");
        }

        RegCloseKey(hKey);
        return;
    }

    // Check TPM 1.2
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\Tpm", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        info->present = true;
        info->version = TPM_VERSION_12;
        strcpy(info->version_str, "1.2");
        info->enabled = true;
        info->activated = true;
        strcpy(info->manufacturer, "Windows TPM 1.2");
        RegCloseKey(hKey);
        return;
    }

    info->present = false;
    strcpy(info->error_msg, "No TPM detected. System may not have a TPM or driver is not loaded.");
}

#else
static void linux_get_tpm_info(fsi_tpm_info_t *info)
{

}
#endif

int fsi_tpm_collect(fsi_tpm_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#ifdef _WIN32
    windows_get_tpm_info(info);
#else
    linux_get_tpm_info(info);
#endif

    return 0;
}

void fsi_tpm_print(const fsi_tpm_info_t *info)
{
    printf("=== TPM Information ===\n");
    printf("Present:      %s\n", info->present ? "YES" : "NO");
    if (!info->present) {
        printf("Note:         %s\n", info->error_msg);
        return;
    }
    printf("Version:      %s\n", info->version_str);
    printf("Manufacturer: %s\n", info->manufacturer);
    printf("Enabled:      %s\n", info->enabled ? "YES" : "NO");
    printf("Activated:    %s\n", info->activated ? "YES" : "NO");
    printf("Owned:        %s\n", info->owned ? "YES" : "NO");
}