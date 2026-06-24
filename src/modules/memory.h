#ifndef FSI_MEMORY_H
#define FSI_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MEM_TYPE_USABLE = 1,
    MEM_TYPE_RESERVED,
    MEM_TYPE_ACPI_RECLAIMABLE,
    MEM_TYPE_ACPI_NVS,
    MEM_TYPE_BAD,
    MEM_TYPE_PERSISTENT,
    MEM_TYPE_UNKNOWN
} fsi_mem_type_t;

typedef struct {
    uint64_t     base;
    uint64_t     length;
    fsi_mem_type_t type;
    char         type_str[32];
} fsi_mem_range_t;

typedef struct {
    fsi_mem_range_t *ranges;
    int              count;
    uint64_t         total_usable_bytes;
    uint64_t         total_bytes;
    bool             available;
    char             error_msg[256];
} fsi_memory_map_t;

int  fsi_memory_collect(fsi_memory_map_t *map);
void fsi_memory_free(fsi_memory_map_t *map);
void fsi_memory_print(const fsi_memory_map_t *map);

#endif 