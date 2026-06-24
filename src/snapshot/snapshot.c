#include "snapshot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static uint32_t crc32_simple(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= p[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else         crc >>= 1;
        }
    }
    return ~crc;
}

int fsi_snapshot_save(const fsi_system_snapshot_t *snap, const char *path)
{
    if (!snap || !path) return -1;

    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Cannot create snapshot %s: %s\n",
                path, strerror(errno));
        return -1;
    }

    fsi_snapshot_header_t hdr = {0};
    hdr.magic     = FSI_SNAPSHOT_MAGIC;
    hdr.version   = FSI_SNAPSHOT_VERSION;
    hdr.data_size = sizeof(fsi_system_snapshot_t);
    strncpy(hdr.timestamp, snap->timestamp, sizeof(hdr.timestamp) - 1);
    strncpy(hdr.hostname,  snap->hostname,  sizeof(hdr.hostname)  - 1);
    hdr.checksum  = crc32_simple(snap, sizeof(*snap));

    fwrite(&hdr,  sizeof(hdr),  1, f);
    fwrite(snap,  sizeof(*snap), 1, f);
    fclose(f);
    return 0;
}

int fsi_snapshot_load(fsi_system_snapshot_t *snap, const char *path)
{
    if (!snap || !path) return -1;

    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open snapshot %s: %s\n",
                path, strerror(errno));
        return -1;
    }

    fsi_snapshot_header_t hdr = {0};
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        fprintf(stderr, "Error reading snapshot header.\n");
        fclose(f);
        return -1;
    }

    if (hdr.magic != FSI_SNAPSHOT_MAGIC) {
        fprintf(stderr, "Invalid snapshot file (bad magic).\n");
        fclose(f);
        return -1;
    }

    if (hdr.version != FSI_SNAPSHOT_VERSION) {
        fprintf(stderr, "Snapshot version mismatch: got %u expected %u.\n",
                hdr.version, FSI_SNAPSHOT_VERSION);
        fclose(f);
        return -1;
    }

    if (fread(snap, sizeof(*snap), 1, f) != 1) {
        fprintf(stderr, "Error reading snapshot data.\n");
        fclose(f);
        return -1;
    }
    fclose(f);


    uint32_t computed = crc32_simple(snap, sizeof(*snap));
    if (computed != hdr.checksum) {
        fprintf(stderr, "Warning: snapshot checksum mismatch "
                "(file may be corrupted).\n");
    }

    return 0;
}

void fsi_snapshot_free_loaded(fsi_system_snapshot_t *snap)
{

    if (!snap) return;
    snap->acpi.tables    = NULL;
    snap->pcie.devices   = NULL;
    snap->memory.ranges  = NULL;
    snap->msr.entries    = NULL;
    snap->audit.checks   = NULL;
}