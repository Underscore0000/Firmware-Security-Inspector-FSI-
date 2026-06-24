#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>

static fsi_mem_type_t classify_windows_memory(DWORD type)
{
    switch (type) {
        case MEM_PRIVATE:   return MEM_TYPE_USABLE;
        case MEM_MAPPED:    return MEM_TYPE_USABLE;
        case MEM_IMAGE:     return MEM_TYPE_RESERVED;
        default:            return MEM_TYPE_UNKNOWN;
    }
}

static const char *mem_type_str(fsi_mem_type_t t)
{
    switch (t) {
        case MEM_TYPE_USABLE:           return "System RAM";
        case MEM_TYPE_RESERVED:         return "Reserved";
        case MEM_TYPE_ACPI_RECLAIMABLE: return "ACPI Reclaimable";
        case MEM_TYPE_ACPI_NVS:         return "ACPI NVS";
        case MEM_TYPE_BAD:              return "Bad Memory";
        case MEM_TYPE_PERSISTENT:       return "Persistent";
        default:                        return "Unknown";
    }
}

static void windows_get_memory_info(fsi_memory_map_t *map)
{
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    
    if (!GlobalMemoryStatusEx(&memStatus)) {
        map->available = false;
        snprintf(map->error_msg, sizeof(map->error_msg),
                 "Impossibile ottenere informazioni sulla memoria.");
        return;
    }


    map->available = true;
    map->total_bytes = memStatus.ullTotalPhys;
    map->total_usable_bytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
    

    int range_count = 4;
    map->ranges = calloc((size_t)range_count, sizeof(fsi_mem_range_t));
    if (!map->ranges) {
        map->count = 0;
        return;
    }

    // Region 1: System RAM 
    fsi_mem_range_t *r = &map->ranges[0];
    r->base = 0;
    r->length = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
    r->type = MEM_TYPE_USABLE;
    strcpy(r->type_str, "System RAM (in use)");

    // Region 2: Available RAM
    r = &map->ranges[1];
    r->base = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
    r->length = memStatus.ullAvailPhys;
    r->type = MEM_TYPE_USABLE;
    strcpy(r->type_str, "System RAM (available)");

    // Region 3: Page file
    r = &map->ranges[2];
    r->base = memStatus.ullTotalPhys;
    r->length = memStatus.ullTotalPageFile;
    r->type = MEM_TYPE_RESERVED;
    strcpy(r->type_str, "Page File");

    // Region 4: Virtual memory
    r = &map->ranges[3];
    r->base = memStatus.ullTotalPhys + memStatus.ullTotalPageFile;
    r->length = memStatus.ullTotalVirtual - memStatus.ullTotalPhys - memStatus.ullTotalPageFile;
    r->type = MEM_TYPE_UNKNOWN;
    strcpy(r->type_str, "Virtual Memory");

    map->count = range_count;
}

#else

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define IOMEM_PATH "/proc/iomem"

static fsi_mem_type_t classify_iomem(const char *desc)
{
    if (strstr(desc, "System RAM"))     return MEM_TYPE_USABLE;
    if (strstr(desc, "reserved"))       return MEM_TYPE_RESERVED;
    if (strstr(desc, "ACPI Tables"))    return MEM_TYPE_ACPI_RECLAIMABLE;
    if (strstr(desc, "ACPI Non-volat")) return MEM_TYPE_ACPI_NVS;
    if (strstr(desc, "Persistent"))     return MEM_TYPE_PERSISTENT;
    if (strstr(desc, "bad"))            return MEM_TYPE_BAD;
    return MEM_TYPE_UNKNOWN;
}

static const char *mem_type_str(fsi_mem_type_t t)
{
    switch (t) {
        case MEM_TYPE_USABLE:           return "System RAM";
        case MEM_TYPE_RESERVED:         return "Reserved";
        case MEM_TYPE_ACPI_RECLAIMABLE: return "ACPI Reclaimable";
        case MEM_TYPE_ACPI_NVS:         return "ACPI NVS";
        case MEM_TYPE_BAD:              return "Bad Memory";
        case MEM_TYPE_PERSISTENT:       return "Persistent";
        default:                        return "Unknown";
    }
}

static void linux_get_memory_info(fsi_memory_map_t *map)
{
    FILE *f = fopen(IOMEM_PATH, "r");
    if (!f) {
        map->available = false;
        snprintf(map->error_msg, sizeof(map->error_msg),
                 "Impossibile aprire %s: %s", IOMEM_PATH, strerror(errno));
        return;
    }

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] != ' ') count++;
    }

    map->ranges = calloc((size_t)(count + 1), sizeof(fsi_mem_range_t));
    if (!map->ranges) {
        fclose(f);
        return;
    }

    rewind(f);
    int idx = 0;
    while (fgets(line, sizeof(line), f) && idx <= count) {
        if (line[0] == ' ') continue;

        uint64_t base = 0, end = 0;
        char desc[128] = {0};

        if (sscanf(line, "%llx-%llx : %127[^\n]",
                   (unsigned long long *)&base,
                   (unsigned long long *)&end,
                   desc) < 2) continue;

        fsi_mem_range_t *r = &map->ranges[idx];
        r->base = base;
        r->length = end - base + 1;
        r->type = classify_iomem(desc);
        strncpy(r->type_str, desc, sizeof(r->type_str) - 1);

        if (r->type == MEM_TYPE_USABLE) {
            map->total_usable_bytes += r->length;
        }
        map->total_bytes += r->length;
        idx++;
    }

    fclose(f);
    map->count = idx;
    map->available = true;
}
#endif

int fsi_memory_collect(fsi_memory_map_t *map)
{
    if (!map) return -1;
    memset(map, 0, sizeof(*map));

#ifdef _WIN32
    windows_get_memory_info(map);
#else
    linux_get_memory_info(map);
#endif

    return 0;
}

void fsi_memory_free(fsi_memory_map_t *map)
{
    if (map && map->ranges) {
        free(map->ranges);
        map->ranges = NULL;
        map->count = 0;
    }
}

void fsi_memory_print(const fsi_memory_map_t *map)
{
    printf("=== Memory Map ===\n");
    if (!map->available) {
        printf("Non disponibile: %s\n", map->error_msg);
        return;
    }
    printf("Total RAM:    %.2f GB\n",
           (double)map->total_bytes / (1024.0 * 1024.0 * 1024.0));
    printf("Regions:      %d\n\n", map->count);
    printf("%-20s %-16s %-12s  %s\n",
           "Base", "Length", "Type", "Description");
    printf("%-20s %-16s %-12s  %s\n",
           "----", "------", "----", "-----------");
    for (int i = 0; i < map->count; i++) {
        const fsi_mem_range_t *r = &map->ranges[i];
        printf("0x%016llX  0x%014llX  %-12s  %s\n",
               (unsigned long long)r->base,
               (unsigned long long)r->length,
               mem_type_str(r->type),
               r->type_str);
    }
}