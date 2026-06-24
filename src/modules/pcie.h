#ifndef FSI_PCIE_H
#define FSI_PCIE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t  bus;
    uint8_t  device;
    uint8_t  function;
    uint8_t  class_code;
    uint8_t  subclass;
    uint8_t  prog_if;
    uint8_t  revision;
    char     vendor_name[64];
    char     device_name[64];
    char     class_name[64];
    bool     is_pcie;        
} fsi_pci_device_t;

typedef struct {
    fsi_pci_device_t *devices;
    int               count;
    bool              available;
    char              error_msg[256];
} fsi_pcie_data_t;

int  fsi_pcie_collect(fsi_pcie_data_t *data);
void fsi_pcie_free(fsi_pcie_data_t *data);
void fsi_pcie_print(const fsi_pcie_data_t *data);
const char *fsi_pci_class_name(uint8_t class_code, uint8_t subclass);
const char *fsi_pci_vendor_name(uint16_t vendor_id);

#endif 