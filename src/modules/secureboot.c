#include "secureboot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

static void windows_get_secureboot_info(fsi_secureboot_info_t *info)
{
    char buffer[8];
    HKEY hKey;
    DWORD value = 0;
    DWORD dataSize = sizeof(value);

    strcpy(info->state_str, "Unknown");
    strcpy(info->error_msg, "");


    DWORD size = GetFirmwareEnvironmentVariableA(
        "SecureBoot",
        "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}",
        buffer, sizeof(buffer));

    if (size > 0 && buffer[0] == 1) {
        info->state = SB_STATE_ENABLED;
        strcpy(info->state_str, "Enabled");
        info->pk_present = true;
        info->kek_present = true;
        info->db_present = true;
        info->dbx_present = true;
        return;
    } else if (size > 0 && buffer[0] == 0) {
        info->state = SB_STATE_DISABLED;
        strcpy(info->state_str, "Disabled");
        return;
    }


    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        if (RegQueryValueExA(hKey, "UEFISecureBootEnabled", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
            if (value == 1) {
                info->state = SB_STATE_ENABLED;
                strcpy(info->state_str, "Enabled");
                info->pk_present = true;
                info->kek_present = true;
                info->db_present = true;
                info->dbx_present = true;
            } else {
                info->state = SB_STATE_DISABLED;
                strcpy(info->state_str, "Disabled");
            }
        }
        RegCloseKey(hKey);
    } else {
        info->state = SB_STATE_UNKNOWN;
        strcpy(info->state_str, "Unknown");
        strcpy(info->error_msg, "Secure Boot not available (Legacy BIOS or no UEFI)");
    }
}

#else
static void linux_get_secureboot_info(fsi_secureboot_info_t *info)
{

}
#endif

int fsi_secureboot_collect(fsi_secureboot_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

#ifdef _WIN32
    windows_get_secureboot_info(info);
#else
    linux_get_secureboot_info(info);
#endif

    return 0;
}

void fsi_secureboot_print(const fsi_secureboot_info_t *info)
{
    printf("=== Secure Boot ===\n");
    printf("State:        %s\n", info->state_str);
    if (info->error_msg[0])
        printf("Note:         %s\n", info->error_msg);
    printf("PK present:   %s\n", info->pk_present  ? "YES" : "NO");
    printf("KEK present:  %s\n", info->kek_present ? "YES" : "NO");
    printf("DB present:   %s\n", info->db_present  ? "YES" : "NO");
    printf("DBX present:  %s\n", info->dbx_present ? "YES" : "NO");
    printf("MOK present:  %s\n", info->mok_present ? "YES" : "NO");
}