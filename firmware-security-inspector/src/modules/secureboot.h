#ifndef FSI_SECUREBOOT_H
#define FSI_SECUREBOOT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SB_STATE_UNKNOWN = 0,
    SB_STATE_DISABLED,
    SB_STATE_ENABLED,
    SB_STATE_SETUP_MODE,   /* Secure Boot in setup mode (no PK) */
    SB_STATE_AUDIT_MODE,   /* Audit mode */
    SB_STATE_DEPLOYED_MODE
} fsi_sb_state_t;

typedef struct {
    fsi_sb_state_t state;
    char           state_str[32];
    bool           pk_present;     /* Platform Key */
    bool           kek_present;    /* Key Exchange Key */
    bool           db_present;     /* Signature Database */
    bool           dbx_present;    /* Forbidden Signatures Database */
    bool           mok_present;    /* Machine Owner Key (shim) */
    char           error_msg[256];
} fsi_secureboot_info_t;

int  fsi_secureboot_collect(fsi_secureboot_info_t *info);
void fsi_secureboot_print(const fsi_secureboot_info_t *info);
const char *fsi_secureboot_state_description(fsi_sb_state_t state);
const char *fsi_secureboot_recommendation(const fsi_secureboot_info_t *info);

#endif 