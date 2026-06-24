#include "cpu.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t family;
    uint32_t model;
    const char *name;
} intel_uarch_t;

static const intel_uarch_t intel_uarch_table[] = {
    {6, 0x2A, "Sandy Bridge"},
    {6, 0x2D, "Sandy Bridge-EP"},
    {6, 0x3A, "Ivy Bridge"},
    {6, 0x3E, "Ivy Bridge-EP"},
    {6, 0x3C, "Haswell"},
    {6, 0x3F, "Haswell-EP"},
    {6, 0x45, "Haswell ULT"},
    {6, 0x3D, "Broadwell"},
    {6, 0x56, "Broadwell-EP"},
    {6, 0x4E, "Skylake"},
    {6, 0x5E, "Skylake-S"},
    {6, 0x8E, "Kaby Lake"},
    {6, 0x9E, "Kaby Lake-S"},
    {6, 0xA5, "Comet Lake"},
    {6, 0xA6, "Comet Lake-U"},
    {6, 0x7D, "Ice Lake"},
    {6, 0x7E, "Ice Lake-U"},
    {6, 0x8C, "Tiger Lake"},
    {6, 0x8D, "Tiger Lake-H"},
    {6, 0x97, "Alder Lake"},
    {6, 0x9A, "Alder Lake-P"},
    {6, 0xB7, "Raptor Lake"},
    {6, 0xBA, "Raptor Lake-P"},
    {0,  0,   NULL}
};

typedef struct {
    uint32_t family;
    const char *name;
} amd_uarch_t;

static const amd_uarch_t amd_uarch_table[] = {
    {0x15, "Bulldozer/Piledriver/Steamroller/Excavator"},
    {0x16, "Jaguar/Puma"},
    {0x17, "Zen/Zen+/Zen2"},
    {0x19, "Zen3/Zen4"},
    {0x1A, "Zen5"},
    {0,    NULL}
};

int fsi_cpu_collect(fsi_cpu_info_t *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

    fsi_get_vendor_string(info->vendor);
    fsi_cpuid(0, 0, &info->leaf0);
    fsi_cpuid(1, 0, &info->leaf1);

    uint32_t eax = info->leaf1.eax;
    uint32_t ecx = info->leaf1.ecx;

    info->stepping = eax & 0xF;
    info->model = (eax >> 4) & 0xF;
    info->family = (eax >> 8) & 0xF;
    info->extended_model = (eax >> 16) & 0xF;
    info->extended_family = (eax >> 20) & 0xFF;

    if (info->family == 0xF) {
        info->effective_family = info->family + info->extended_family;
    } else {
        info->effective_family = info->family;
    }

    if (info->family == 0x6 || info->family == 0xF) {
        info->effective_model = info->model | (info->extended_model << 4);
    } else {
        info->effective_model = info->model;
    }

    info->features.aes_ni = (ecx >> 25) & 1;
    info->features.avx = (ecx >> 28) & 1;
    info->features.vtx = (ecx >> 5) & 1;
    info->features.nx = false;

    if (info->leaf0.eax >= 7) {
        fsi_cpuid(7, 0, &info->leaf7);
        uint32_t ebx7 = info->leaf7.ebx;
        uint32_t ecx7 = info->leaf7.ecx;
        uint32_t edx7 = info->leaf7.edx;

        info->features.smep = (ebx7 >> 7) & 1;
        info->features.avx2 = (ebx7 >> 5) & 1;
        info->features.sgx = (ebx7 >> 2) & 1;
        info->features.rdseed = (ebx7 >> 18) & 1;
        info->features.avx512f = (ebx7 >> 16) & 1;
        info->features.smap = (ebx7 >> 20) & 1;
        info->features.umip = (ecx7 >> 2) & 1;
        info->features.cet_ss = (ecx7 >> 7) & 1;
        info->features.cet_ibt = (edx7 >> 20) & 1;
    }

    uint32_t max_ext = fsi_cpuid_max_extended_leaf();
    if (max_ext >= 0x80000001) {
        fsi_cpuid(0x80000001, 0, &info->leaf80000001);
        uint32_t edx_ext = info->leaf80000001.edx;
        uint32_t ecx_ext = info->leaf80000001.ecx;

        info->features.nx = (edx_ext >> 20) & 1;
        info->features.amd_v = (ecx_ext >> 2) & 1;
        info->features.rdrand = (ecx >> 30) & 1;
    }

    if (max_ext >= 0x80000004) {
        cpuid_result_t r;
        char *p = info->brand;
        for (uint32_t l = 0x80000002; l <= 0x80000004; l++) {
            fsi_cpuid(l, 0, &r);
            memcpy(p, &r.eax, 4);
            memcpy(p + 4, &r.ebx, 4);
            memcpy(p + 8, &r.ecx, 4);
            memcpy(p + 12, &r.edx, 4);
            p += 16;
        }
        info->brand[48] = '\0';
        char *start = info->brand;
        while (*start == ' ') start++;
        if (start != info->brand) {
            memmove(info->brand, start, strlen(start) + 1);
        }
    }

    if (info->leaf0.eax >= 0xB) {
        cpuid_result_t topo;
        fsi_cpuid(0xB, 0, &topo);
        int threads_per_core = topo.ebx & 0xFFFF;
        fsi_cpuid(0xB, 1, &topo);
        int logical_total = topo.ebx & 0xFFFF;
        info->logical_cores = logical_total > 0 ? logical_total : 1;
        info->physical_cores = (threads_per_core > 0)
                               ? info->logical_cores / threads_per_core
                               : info->logical_cores;
    } else {
        info->logical_cores = (info->leaf1.ebx >> 16) & 0xFF;
        info->physical_cores = info->logical_cores;
    }

    if (strncmp(info->vendor, "GenuineIntel", 12) == 0) {
        info->cache_count = 0;
        for (int i = 0; i < 8; i++) {
            cpuid_result_t c;
            fsi_cpuid(4, (uint32_t)i, &c);
            int cache_type = c.eax & 0x1F;
            if (cache_type == 0) break;

            fsi_cache_t *ca = &info->caches[info->cache_count++];
            ca->level = (c.eax >> 5) & 0x7;
            ca->line_size = (c.ebx & 0xFFF) + 1;
            ca->ways = ((c.ebx >> 22) & 0x3FF) + 1;
            uint32_t sets = c.ecx + 1;
            uint32_t partitions = ((c.ebx >> 12) & 0x3FF) + 1;
            ca->size_kb = (ca->ways * partitions * ca->line_size * sets) / 1024;

            switch (cache_type) {
                case 1: strncpy(ca->type, "Data", sizeof(ca->type)); break;
                case 2: strncpy(ca->type, "Instruction", sizeof(ca->type)); break;
                case 3: strncpy(ca->type, "Unified", sizeof(ca->type)); break;
                default: strncpy(ca->type, "Unknown", sizeof(ca->type)); break;
            }
        }
    }

    info->features.txt = (info->leaf1.ecx >> 6) & 1;
    fsi_cpu_decode_microarch(info);

    return 0;
}

void fsi_cpu_decode_microarch(fsi_cpu_info_t *info)
{
    if (strncmp(info->vendor, "GenuineIntel", 12) == 0) {
        for (int i = 0; intel_uarch_table[i].name; i++) {
            if (intel_uarch_table[i].family == info->effective_family &&
                intel_uarch_table[i].model == info->effective_model) {
                strncpy(info->microarch, intel_uarch_table[i].name,
                        sizeof(info->microarch) - 1);
                return;
            }
        }
        snprintf(info->microarch, sizeof(info->microarch),
                 "Intel Family 0x%X Model 0x%X",
                 info->effective_family, info->effective_model);
    } else if (strncmp(info->vendor, "AuthenticAMD", 12) == 0) {
        for (int i = 0; amd_uarch_table[i].name; i++) {
            if (amd_uarch_table[i].family == info->effective_family) {
                strncpy(info->microarch, amd_uarch_table[i].name,
                        sizeof(info->microarch) - 1);
                return;
            }
        }
        snprintf(info->microarch, sizeof(info->microarch),
                 "AMD Family 0x%X", info->effective_family);
    } else {
        strncpy(info->microarch, "Unknown", sizeof(info->microarch) - 1);
    }
}

const char *fsi_cpu_feature_description(const char *feature_name)
{
    if (!feature_name) return "";
    if (strcmp(feature_name, "nx") == 0)
        return "No-Execute (NX/XD): Prevents execution of pages marked as data. Source: CPUID 0x80000001 EDX bit 20. Critical against buffer overflow exploits.";
    if (strcmp(feature_name, "smep") == 0)
        return "Supervisor Mode Execution Prevention: Prevents kernel from executing code in user pages. CPUID leaf 7 EBX bit 7, enabled via CR4 bit 20.";
    if (strcmp(feature_name, "smap") == 0)
        return "Supervisor Mode Access Prevention: Prevents kernel from accessing user page data without STAC/CLAC. CPUID leaf 7 EBX bit 20, CR4 bit 21.";
    if (strcmp(feature_name, "umip") == 0)
        return "User Mode Instruction Prevention: Blocks SGDT, SIDT, SLDT, SMSW, STR in ring 3. CPUID leaf 7 ECX bit 2, CR4 bit 11.";
    if (strcmp(feature_name, "cet_ss") == 0)
        return "Control-flow Enforcement Shadow Stack: Maintains separate stack for return addresses, protects against ROP attacks.";
    if (strcmp(feature_name, "cet_ibt") == 0)
        return "Control-flow Enforcement Indirect Branch Tracking: Validates indirect call/jmp targets via ENDBR64.";
    if (strcmp(feature_name, "aes_ni") == 0)
        return "AES New Instructions: Hardware acceleration for AES. CPUID leaf 1 ECX bit 25.";
    if (strcmp(feature_name, "vtx") == 0)
        return "Intel VT-x: Intel hardware virtualization. Required for hypervisors. CPUID leaf 1 ECX bit 5.";
    if (strcmp(feature_name, "amd_v") == 0)
        return "AMD-V (SVM): AMD hardware virtualization. CPUID 0x80000001 ECX bit 2.";
    if (strcmp(feature_name, "txt") == 0)
        return "Intel Trusted Execution Technology: Hardware-based secure execution environment. CPUID leaf 1 ECX bit 6.";
    if (strcmp(feature_name, "sgx") == 0)
        return "Software Guard Extensions: Hardware-protected enclaves for sensitive code and data. CPUID leaf 7 EBX bit 2.";
    if (strcmp(feature_name, "avx") == 0)
        return "Advanced Vector Extensions 256-bit: Extended SIMD. CPUID leaf 1 ECX bit 28.";
    if (strcmp(feature_name, "avx2") == 0)
        return "AVX2: 256-bit integer operations. CPUID leaf 7 EBX bit 5.";
    if (strcmp(feature_name, "avx512f") == 0)
        return "AVX-512 Foundation: 512-bit SIMD. CPUID leaf 7 EBX bit 16.";
    if (strcmp(feature_name, "rdrand") == 0)
        return "RDRAND: Hardware random number generator. CPUID leaf 1 ECX bit 30.";
    return "Unknown feature.";
}

void fsi_cpu_print(const fsi_cpu_info_t *info)
{
    printf("=== CPU Information ===\n");
    printf("Vendor:       %s\n", info->vendor);
    printf("Brand:        %s\n", info->brand);
    printf("Family:       0x%X (effective 0x%X)\n",
           info->family, info->effective_family);
    printf("Model:        0x%X (effective 0x%X)\n",
           info->model, info->effective_model);
    printf("Stepping:     %u\n", info->stepping);
    printf("Microarch:    %s\n", info->microarch);
    printf("Cores:        %d physical, %d logical\n",
           info->physical_cores, info->logical_cores);
    printf("\n=== Security Features ===\n");
    printf("NX/XD:        %s\n", info->features.nx ? "YES" : "NO");
    printf("SMEP:         %s\n", info->features.smep ? "YES" : "NO");
    printf("SMAP:         %s\n", info->features.smap ? "YES" : "NO");
    printf("UMIP:         %s\n", info->features.umip ? "YES" : "NO");
    printf("CET-SS:       %s\n", info->features.cet_ss ? "YES" : "NO");
    printf("CET-IBT:      %s\n", info->features.cet_ibt ? "YES" : "NO");
    printf("AES-NI:       %s\n", info->features.aes_ni ? "YES" : "NO");
    printf("VT-x/AMD-V:   %s\n",
           (info->features.vtx || info->features.amd_v) ? "YES" : "NO");
    printf("SGX:          %s\n", info->features.sgx ? "YES" : "NO");
    printf("TXT:          %s\n", info->features.txt ? "YES" : "NO");
}