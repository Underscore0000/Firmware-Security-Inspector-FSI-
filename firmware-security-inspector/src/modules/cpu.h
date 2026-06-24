#ifndef FSI_CPU_H
#define FSI_CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_result_t;

typedef struct {
    int      level;
    char     type[16];
    uint32_t size_kb;
    uint32_t ways;
    uint32_t line_size;
} fsi_cache_t;

typedef struct {
    bool nx;        /* No-Execute / Execute Disable */
    bool smep;      /* Supervisor Mode Execution Prevention */
    bool smap;      /* Supervisor Mode Access Prevention */
    bool umip;      /* User Mode Instruction Prevention */
    bool cet_ss;    /* Control-flow Enforcement - Shadow Stack */
    bool cet_ibt;   /* Control-flow Enforcement - Indirect Branch Tracking */
    bool aes_ni;    /* AES New Instructions */
    bool vtx;       /* Intel VT-x */
    bool amd_v;     /* AMD-V (SVM) */
    bool txt;       /* Intel TXT */
    bool sgx;       /* Intel SGX */
    bool avx;       /* AVX */
    bool avx2;      /* AVX2 */
    bool avx512f;   /* AVX-512 Foundation */
    bool rdrand;    /* Hardware RNG */
    bool rdseed;    /* Hardware Seed */
} fsi_cpu_features_t;

typedef struct {
    char     vendor[13];
    char     brand[49];
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t extended_family;
    uint32_t extended_model;
    uint32_t effective_family;
    uint32_t effective_model;
    char     microarch[64];
    int      physical_cores;
    int      logical_cores;
    int      cache_count;
    fsi_cache_t      caches[8];
    fsi_cpu_features_t features;
    cpuid_result_t   leaf0;          /* CPUID leaf 0: max standard leaf + vendor */
    cpuid_result_t   leaf1;          /* CPUID leaf 1: family/model/stepping + features */
    cpuid_result_t   leaf7;          /* CPUID leaf 7: extended features (SMEP, SMAP, etc.) */
    cpuid_result_t   leaf80000001;   /* CPUID leaf 0x80000001: NX, AMD-V */
} fsi_cpu_info_t;

extern void     fsi_cpuid(uint32_t leaf, uint32_t subleaf, cpuid_result_t *out);
extern uint32_t fsi_cpuid_max_leaf(void);
extern uint32_t fsi_cpuid_max_extended_leaf(void);
extern void     fsi_get_vendor_string(char *buf);
extern uint64_t fsi_xgetbv(uint32_t xcr_index);

int  fsi_cpu_collect(fsi_cpu_info_t *info);
void fsi_cpu_decode_microarch(fsi_cpu_info_t *info);
const char *fsi_cpu_feature_description(const char *feature_name);
void fsi_cpu_print(const fsi_cpu_info_t *info);

#endif