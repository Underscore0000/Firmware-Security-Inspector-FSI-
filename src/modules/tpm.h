#ifndef FSI_TPM_H
#define FSI_TPM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TPM_VERSION_NONE = 0,
    TPM_VERSION_12,
    TPM_VERSION_20
} fsi_tpm_version_t;

typedef struct {
    bool              present;
    fsi_tpm_version_t version;
    char              version_str[16];
    bool              enabled;
    bool              activated;
    bool              owned;
    char              manufacturer[64];
    uint32_t          manufacturer_id;
    char              firmware_version[32];
    char              error_msg[256];
} fsi_tpm_info_t;

int  fsi_tpm_collect(fsi_tpm_info_t *info);
void fsi_tpm_print(const fsi_tpm_info_t *info);
const char *fsi_tpm_description(void);
const char *fsi_tpm_recommendation(const fsi_tpm_info_t *info);

#endif 