#ifdef _WIN32
#include "windows_platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <intrin.h>
#include <psapi.h>
#include <setupapi.h>
#include <devguid.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "psapi.lib")

static void reg_get_string(HKEY hKey, const char *value, char *out, size_t outSize) {
    char buffer[256];
    DWORD dataSize = sizeof(buffer);
    DWORD type;
    if (RegQueryValueExA(hKey, value, NULL, &type, (LPBYTE)buffer, &dataSize) == ERROR_SUCCESS) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            strncpy(out, buffer, outSize - 1);
            out[outSize - 1] = '\0';
        }
    }
}

void windows_get_firmware_info(void *info_ptr) {
    struct {
        int boot_mode;
        char boot_mode_str[32];
        char uefi_vendor[128];
        char uefi_version[64];
        char uefi_release_date[32];
        char bios_vendor[128];
        char bios_version[64];
        char bios_date[32];
        char system_manufacturer[128];
        char system_product[128];
        char system_version[64];
        char system_serial[64];
        bool secure_boot_capable;
        bool csm_enabled;
    } *info = (void*)info_ptr;

    HKEY hKey;
    char buffer[8];

    strcpy(info->boot_mode_str, "UEFI");
    info->boot_mode = 1;
    info->secure_boot_capable = true;

    DWORD size = GetFirmwareEnvironmentVariableA("BootCurrent", "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}", buffer, sizeof(buffer));
    if (size == 0) {
        strcpy(info->boot_mode_str, "Legacy BIOS");
        info->boot_mode = 2;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        reg_get_string(hKey, "BIOSVendor", info->bios_vendor, sizeof(info->bios_vendor));
        reg_get_string(hKey, "BIOSVersion", info->bios_version, sizeof(info->bios_version));
        reg_get_string(hKey, "BIOSReleaseDate", info->bios_date, sizeof(info->bios_date));
        RegCloseKey(hKey);
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        reg_get_string(hKey, "SystemManufacturer", info->system_manufacturer, sizeof(info->system_manufacturer));
        reg_get_string(hKey, "SystemProductName", info->system_product, sizeof(info->system_product));
        reg_get_string(hKey, "SystemVersion", info->system_version, sizeof(info->system_version));
        RegCloseKey(hKey);
    }

    if (strlen(info->bios_vendor) == 0) strcpy(info->bios_vendor, "Unknown");
    if (strlen(info->bios_version) == 0) strcpy(info->bios_version, "Unknown");
    if (strlen(info->system_manufacturer) == 0) strcpy(info->system_manufacturer, "Unknown");
    if (strlen(info->system_product) == 0) strcpy(info->system_product, "Unknown");
}

void windows_get_secureboot_info(void *info_ptr) {
    struct {
        int state;
        char state_str[32];
        bool pk_present;
        bool kek_present;
        bool db_present;
        bool dbx_present;
        bool mok_present;
        char error_msg[256];
    } *info = (void*)info_ptr;

    char buffer[8];
    DWORD size = GetFirmwareEnvironmentVariableA("SecureBoot", "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}", buffer, sizeof(buffer));

    if (size > 0 && buffer[0] == 1) {
        info->state = 2;
        strcpy(info->state_str, "Enabled");
        info->pk_present = true;
        info->kek_present = true;
        info->db_present = true;
        info->dbx_present = true;
        return;
    }

    HKEY hKey;
    DWORD value = 0;
    DWORD dataSize = sizeof(value);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "UEFISecureBootEnabled", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
            if (value == 1) {
                info->state = 2;
                strcpy(info->state_str, "Enabled");
                info->pk_present = true;
                info->kek_present = true;
                info->db_present = true;
                info->dbx_present = true;
            } else {
                info->state = 1;
                strcpy(info->state_str, "Disabled");
            }
            RegCloseKey(hKey);
            return;
        }
        RegCloseKey(hKey);
    }

    info->state = 0;
    strcpy(info->state_str, "Unknown");
    strcpy(info->error_msg, "Secure Boot not available");
}

void windows_get_tpm_info(void *info_ptr) {
    struct {
        bool present;
        int version;
        char version_str[16];
        bool enabled;
        bool activated;
        bool owned;
        char manufacturer[64];
        uint32_t manufacturer_id;
        char firmware_version[32];
        char error_msg[256];
    } *info = (void*)info_ptr;

    HKEY hKey;
    strcpy(info->version_str, "None");
    strcpy(info->manufacturer, "Unknown");

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tpm", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        info->present = true;
        info->enabled = true;
        info->activated = true;
        strcpy(info->version_str, "2.0");
        info->version = 2;
        strcpy(info->manufacturer, "Windows TPM");
        RegCloseKey(hKey);
        return;
    }

    info->present = false;
    strcpy(info->error_msg, "TPM not found");
}

void windows_get_smbios_info(void *info_ptr) {
    struct {
        char bios_vendor[128];
        char bios_version[128];
        char bios_release_date[64];
        uint8_t bios_major_release;
        uint8_t bios_minor_release;
        char system_manufacturer[128];
        char system_product[128];
        char system_version[128];
        char system_serial[128];
        char system_uuid[37];
        char baseboard_manufacturer[128];
        char baseboard_product[128];
        char baseboard_serial[128];
        char chassis_manufacturer[128];
        char chassis_version[128];
        uint8_t smbios_major;
        uint8_t smbios_minor;
        bool available;
        char error_msg[256];
    } *info = (void*)info_ptr;

    HKEY hKey;
    info->available = true;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        reg_get_string(hKey, "BIOSVendor", info->bios_vendor, sizeof(info->bios_vendor));
        reg_get_string(hKey, "BIOSVersion", info->bios_version, sizeof(info->bios_version));
        reg_get_string(hKey, "BIOSReleaseDate", info->bios_release_date, sizeof(info->bios_release_date));
        RegCloseKey(hKey);
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        reg_get_string(hKey, "SystemManufacturer", info->system_manufacturer, sizeof(info->system_manufacturer));
        reg_get_string(hKey, "SystemProductName", info->system_product, sizeof(info->system_product));
        reg_get_string(hKey, "SystemVersion", info->system_version, sizeof(info->system_version));
        RegCloseKey(hKey);
    }

    if (strlen(info->bios_vendor) == 0) strcpy(info->bios_vendor, "Unknown");
    if (strlen(info->bios_version) == 0) strcpy(info->bios_version, "Unknown");
    if (strlen(info->system_manufacturer) == 0) strcpy(info->system_manufacturer, "Unknown");
    if (strlen(info->system_product) == 0) strcpy(info->system_product, "Unknown");

    info->smbios_major = 3;
    info->smbios_minor = 5;
}

void windows_get_pcie_info(void *info_ptr) {
    struct {
        void *devices;
        int count;
        bool available;
        char error_msg[256];
    } *info = (void*)info_ptr;

    HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(&GUID_DEVCLASS_SYSTEM, NULL, NULL, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        info->available = false;
        strcpy(info->error_msg, "PCIe enumeration failed");
        return;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD index = 0;
    info->count = 0;

    while (SetupDiEnumDeviceInfo(deviceInfoSet, index, &deviceInfoData)) {
        info->count++;
        index++;
    }

    info->available = true;
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

void windows_get_memory_info(void *info_ptr) {
    struct {
        void *ranges;
        int count;
        uint64_t total_usable_bytes;
        uint64_t total_bytes;
        bool available;
        char error_msg[256];
    } *info = (void*)info_ptr;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        info->available = false;
        strcpy(info->error_msg, "Memory status not available");
        return;
    }

    info->available = true;
    info->total_bytes = memStatus.ullTotalPhys;
    info->total_usable_bytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
    info->count = 4;

    struct {
        uint64_t base;
        uint64_t length;
        int type;
        char type_str[32];
    } *ranges = malloc(4 * sizeof(*ranges));

    if (ranges) {
        ranges[0].base = 0;
        ranges[0].length = memStatus.ullTotalPhys;
        ranges[0].type = 1;
        strcpy(ranges[0].type_str, "System RAM");

        ranges[1].base = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        ranges[1].length = memStatus.ullAvailPhys;
        ranges[1].type = 1;
        strcpy(ranges[1].type_str, "Available RAM");

        ranges[2].base = memStatus.ullTotalPhys;
        ranges[2].length = memStatus.ullTotalPageFile;
        ranges[2].type = 2;
        strcpy(ranges[2].type_str, "Page File");

        ranges[3].base = memStatus.ullTotalPhys + memStatus.ullTotalPageFile;
        ranges[3].length = memStatus.ullTotalVirtual - memStatus.ullTotalPhys - memStatus.ullTotalPageFile;
        ranges[3].type = 0;
        strcpy(ranges[3].type_str, "Virtual Memory");

        info->ranges = ranges;
        info->count = 4;
    }
}

void windows_get_msr_info(void *info_ptr) {
    struct {
        void *entries;
        int count;
        bool available;
        char error_msg[256];
    } *info = (void*)info_ptr;

    info->available = false;
    strcpy(info->error_msg, "MSR access requires kernel driver on Windows");
}

void windows_get_registers_info(void *info_ptr) {
    struct {
        uint64_t cr0;
        uint64_t cr2;
        uint64_t cr3;
        uint64_t cr4;
        uint64_t rflags;
        bool available;
        char error_msg[128];
    } *info = (void*)info_ptr;

    info->available = false;
    strcpy(info->error_msg, "Control registers require kernel driver on Windows");
}

void windows_get_acpi_info(void *info_ptr) {
    struct {
        void *tables;
        int count;
        char oem_id[7];
        uint8_t acpi_revision;
        uint64_t rsdp_address;
        uint64_t xsdt_address;
        bool available;
        char error_msg[256];
    } *info = (void*)info_ptr;

    DWORD size = GetSystemFirmwareTable('ACPI', 0, NULL, 0);
    if (size > 0) {
        info->available = true;
        info->count = 5;
        strcpy(info->oem_id, "Unknown");
        info->acpi_revision = 2;
    } else {
        info->available = false;
        strcpy(info->error_msg, "ACPI tables not available");
    }
}

#endif