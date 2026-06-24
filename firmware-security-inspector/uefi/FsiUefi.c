#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleTextOut.h>
#include "cpu.h"
#include "firmware.h"
#include "secureboot.h"
#include "tpm.h"
#include "memory.h"

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    fsi_cpu_info_t cpu = {0};
    fsi_firmware_info_t fw = {0};
    fsi_secureboot_info_t sb = {0};
    

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Firmware Security Inspector - UEFI Edition\n");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"=========================================\n\n");
    

    fsi_cpu_collect(&cpu);
    Print(L"CPU Vendor: %a\n", cpu.vendor);
    Print(L"CPU Brand: %a\n", cpu.brand);
    Print(L"Microarchitecture: %a\n", cpu.microarch);
    Print(L"Cores: %d physical, %d logical\n\n", cpu.physical_cores, cpu.logical_cores);
    

    fsi_secureboot_collect(&sb);
    Print(L"Secure Boot: %a\n", sb.state_str);
    Print(L"PK Present: %s\n", sb.pk_present ? L"YES" : L"NO");
    Print(L"DBX Present: %s\n\n", sb.dbx_present ? L"YES" : L"NO");
    

    fsi_firmware_collect(&fw);
    Print(L"BIOS Vendor: %a\n", fw.bios_vendor);
    Print(L"BIOS Version: %a\n", fw.bios_version);
    Print(L"BIOS Date: %a\n\n", fw.bios_date);
    
    Print(L"Press any key to exit...\n");
    SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &Status);
    
    return EFI_SUCCESS;
}