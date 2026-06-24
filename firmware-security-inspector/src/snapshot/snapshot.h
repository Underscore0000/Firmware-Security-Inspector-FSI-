#ifndef FSI_SNAPSHOT_H
#define FSI_SNAPSHOT_H

#include "report.h"
#include <stdint.h>
#include <stdbool.h>

#define FSI_SNAPSHOT_MAGIC   0x46534953   
#define FSI_SNAPSHOT_VERSION 1


typedef struct {
    uint32_t magic;
    uint32_t version;
    char     timestamp[64];
    char     hostname[256];
    uint32_t data_size;
    uint32_t checksum;   
} fsi_snapshot_header_t;

typedef struct {
    fsi_snapshot_header_t header;
    fsi_system_snapshot_t data;
} fsi_snapshot_file_t;

int  fsi_snapshot_save(const fsi_system_snapshot_t *snap,
                       const char *path);
int  fsi_snapshot_load(fsi_system_snapshot_t *snap,
                       const char *path);
void fsi_snapshot_free_loaded(fsi_system_snapshot_t *snap);


void fsi_snapshot_diff(const fsi_system_snapshot_t *a,
                       const fsi_system_snapshot_t *b);

#endif 