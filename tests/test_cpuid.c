#include <stdio.h>
#include <assert.h>
#include "cpu.h"

int main() {
    cpuid_result_t result;
    char vendor[13];
    
    // Test CPUID leaf 0
    fsi_cpuid(0, 0, &result);
    printf("CPUID leaf 0: EAX=0x%X, EBX=0x%X, ECX=0x%X, EDX=0x%X\n",
           result.eax, result.ebx, result.ecx, result.edx);
    assert(result.eax > 0); 
    
    
    fsi_get_vendor_string(vendor);
    printf("Vendor: %s\n", vendor);
    assert(strlen(vendor) > 0);
    
    
    uint32_t max = fsi_cpuid_max_leaf();
    printf("Max standard leaf: 0x%X\n", max);
    assert(max >= 0);
    
    
    uint32_t max_ext = fsi_cpuid_max_extended_leaf();
    printf("Max extended leaf: 0x%X\n", max_ext);
    assert(max_ext >= 0x80000000);
    
    printf("All CPUID tests passed!\n");
    return 0;
}