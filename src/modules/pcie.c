#include "pcie.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

/*
 * PCI enumeration on Linux from /sys/bus/pci/devices/
 * Each device directory: DDDD:BB:DD.F (domain:bus:device.function)
 * Contains: vendor, device, class, config (raw config space)
 */

#define PCI_SYSFS "/sys/bus/pci/devices"

const char *fsi_pci_class_name(uint8_t class_code, uint8_t subclass)
{
    switch (class_code) {
        case 0x00: return "Unclassified";
        case 0x01:
            switch (subclass) {
                case 0x01: return "IDE Controller";
                case 0x06: return "SATA Controller (AHCI)";
                case 0x08: return "NVMe Controller";
                default:   return "Mass Storage Controller";
            }
        case 0x02:
            switch (subclass) {
                case 0x00: return "Ethernet Controller";
                case 0x80: return "Network Controller";
                default:   return "Network Controller";
            }
        case 0x03:
            switch (subclass) {
                case 0x00: return "VGA Controller";
                case 0x02: return "XGA Controller";
                default:   return "Display Controller";
            }
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06:
            switch (subclass) {
                case 0x00: return "Host Bridge";
                case 0x01: return "ISA Bridge";
                case 0x04: return "PCI-to-PCI Bridge";
                default:   return "Bridge Device";
            }
        case 0x07: return "Communication Controller";
        case 0x08: return "System Peripheral";
        case 0x09: return "Input Device Controller";
        case 0x0A: return "Docking Station";
        case 0x0B: return "Processor";
        case 0x0C:
            switch (subclass) {
                case 0x03: return "USB Controller";
                case 0x05: return "SMBus Controller";
                default:   return "Serial Bus Controller";
            }
        case 0x0D: return "Wireless Controller";
        case 0x0E: return "Intelligent Controller";
        case 0x0F: return "Satellite Communication Controller";
        case 0x10: return "Encryption Controller";
        case 0x11: return "Signal Processing Controller";
        case 0x12: return "Processing Accelerator";
        case 0x13: return "Non-Essential Instrumentation";
        case 0xFF: return "Unassigned Class";
        default:   return "Unknown Class";
    }
}

const char *fsi_pci_vendor_name(uint16_t vendor_id)
{
    switch (vendor_id) {
        case 0x8086: return "Intel Corporation";
        case 0x1002: return "Advanced Micro Devices (AMD)";
        case 0x10DE: return "NVIDIA Corporation";
        case 0x1022: return "AMD";
        case 0x1AF4: return "Red Hat (VirtIO)";
        case 0x1234: return "QEMU/Bochs";
        case 0x15B3: return "Mellanox Technologies";
        case 0x14E4: return "Broadcom";
        case 0x10EC: return "Realtek Semiconductor";
        case 0x8087: return "Intel (USB/Hub)";
        case 0x1B21: return "ASMedia Technology";
        case 0x1D6A: return "Aquantia";
        case 0x1AE0: return "Google";
        case 0x1414: return "Microsoft";
        case 0x1000: return "Broadcom / LSI Logic";
        case 0x9005: return "Adaptec";
        case 0x1077: return "QLogic Corp.";
        case 0x19E5: return "Huawei Technologies";
        default:     return "Unknown Vendor";
    }
}

static bool read_sysfs_u32_hex(const char *path, uint32_t *val)
{
#if defined(__linux__)
    FILE *f = fopen(path, "r");
    if (!f) return false;
    int r = fscanf(f, "0x%x", val);
    fclose(f);
    return r == 1;
#else
    (void)path; (void)val;
    return false;
#endif
}

int fsi_pcie_collect(fsi_pcie_data_t *data)
{
    if (!data) return -1;
    memset(data, 0, sizeof(*data));

#if defined(__linux__)
    DIR *dir = opendir(PCI_SYSFS);
    if (!dir) {
        data->available = false;
        snprintf(data->error_msg, sizeof(data->error_msg),
                 "Cannot open %s: %s", PCI_SYSFS, strerror(errno));
        return 0;
    }

    int count = 0;
    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (de->d_name[0] == '.') continue;
        count++;
    }

    if (count == 0) {
        closedir(dir);
        data->available = false;
        return 0;
    }

    data->devices = calloc((size_t)count, sizeof(fsi_pci_device_t));
    if (!data->devices) {
        closedir(dir);
        return -1;
    }

    rewinddir(dir);
    int idx = 0;
    char path[512];

    while ((de = readdir(dir)) != NULL && idx < count) {
        if (de->d_name[0] == '.') continue;

        fsi_pci_device_t *dev = &data->devices[idx];
        uint32_t val;

        snprintf(path, sizeof(path), "%s/%s/vendor", PCI_SYSFS, de->d_name);
        if (!read_sysfs_u32_hex(path, &val)) continue;
        dev->vendor_id = (uint16_t)val;

        snprintf(path, sizeof(path), "%s/%s/device", PCI_SYSFS, de->d_name);
        if (!read_sysfs_u32_hex(path, &val)) continue;
        dev->device_id = (uint16_t)val;

        snprintf(path, sizeof(path), "%s/%s/class", PCI_SYSFS, de->d_name);
        if (read_sysfs_u32_hex(path, &val)) {
            dev->class_code = (uint8_t)((val >> 16) & 0xFF);
            dev->subclass = (uint8_t)((val >> 8) & 0xFF);
            dev->prog_if = (uint8_t)(val & 0xFF);
        }

        unsigned int domain, bus, device, function;
        if (sscanf(de->d_name, "%x:%x:%x.%x",
                   &domain, &bus, &device, &function) == 4) {
            dev->bus = (uint8_t)bus;
            dev->device = (uint8_t)device;
            dev->function = (uint8_t)function;
        }

        strncpy(dev->vendor_name,
                fsi_pci_vendor_name(dev->vendor_id),
                sizeof(dev->vendor_name) - 1);
        strncpy(dev->class_name,
                fsi_pci_class_name(dev->class_code, dev->subclass),
                sizeof(dev->class_name) - 1);
        snprintf(dev->device_name, sizeof(dev->device_name),
                 "0x%04X", dev->device_id);

        char pcie_check[512];
        snprintf(pcie_check, sizeof(pcie_check),
                 "%s/%s/current_link_speed", PCI_SYSFS, de->d_name);
        dev->is_pcie = (access(pcie_check, F_OK) == 0);

        idx++;
    }

    closedir(dir);
    data->count = idx;
    data->available = true;
#else
    data->available = false;
    snprintf(data->error_msg, sizeof(data->error_msg),
             "PCI enumeration not supported on this platform.");
#endif

    return 0;
}

void fsi_pcie_free(fsi_pcie_data_t *data)
{
    if (data && data->devices) {
        free(data->devices);
        data->devices = NULL;
        data->count = 0;
    }
}

void fsi_pcie_print(const fsi_pcie_data_t *data)
{
    printf("=== PCI/PCIe Devices ===\n");
    if (!data->available) {
        printf("Not available: %s\n", data->error_msg);
        return;
    }
    printf("Devices found: %d\n\n", data->count);
    printf("%-4s %-5s %-7s %-7s %-24s %-10s  %s\n",
           "Bus", "D:F", "VendorID", "DevID", "Vendor", "Type", "Class");
    printf("%-4s %-5s %-7s %-7s %-24s %-10s  %s\n",
           "---", "---", "--------", "------", "------", "----", "-----");
    for (int i = 0; i < data->count; i++) {
        const fsi_pci_device_t *d = &data->devices[i];
        printf("%02X  %02X:%X  0x%04X  0x%04X  %-24s %-10s  %s\n",
               d->bus, d->device, d->function,
               d->vendor_id, d->device_id,
               d->vendor_name,
               d->is_pcie ? "PCIe" : "PCI",
               d->class_name);
    }
}