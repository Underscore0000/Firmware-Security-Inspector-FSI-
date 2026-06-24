#ifndef WINDOWS_PLATFORM_H
#define WINDOWS_PLATFORM_H

#ifdef _WIN32
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>


void windows_get_firmware_info(void *info);
void windows_get_secureboot_info(void *info);
void windows_get_tpm_info(void *info);
void windows_get_smbios_info(void *info);
void windows_get_pcie_info(void *info);
void windows_get_memory_info(void *info);
void windows_get_msr_info(void *info);
void windows_get_registers_info(void *info);

#endif 
#endif 