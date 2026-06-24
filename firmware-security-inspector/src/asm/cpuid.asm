; cpuid.asm - CPUID wrapper per Windows x64
;   RCX = leaf (EAX input)
;   RDX = subleaf (ECX input)
;   R8  = puntatore a struct { uint32_t eax, ebx, ecx, edx }

section .text
global fsi_cpuid
global fsi_cpuid_max_leaf
global fsi_cpuid_max_extended_leaf
global fsi_get_vendor_string

; void fsi_cpuid(uint32_t leaf, uint32_t subleaf, cpuid_result_t *out)
; Windows x64: RCX=leaf, RDX=subleaf, R8=out
fsi_cpuid:
    push    rbx
    push    rsi
    
    mov     eax, ecx
    mov     ecx, edx
    mov     rsi, r8
    
    cpuid
    
    mov     dword [rsi + 0], eax
    mov     dword [rsi + 4], ebx
    mov     dword [rsi + 8], ecx
    mov     dword [rsi + 12], edx
    
    pop     rsi
    pop     rbx
    ret

; uint32_t fsi_cpuid_max_leaf(void)
fsi_cpuid_max_leaf:
    push    rbx
    xor     eax, eax
    xor     ecx, ecx
    cpuid
    pop     rbx
    ret

; uint32_t fsi_cpuid_max_extended_leaf(void)
fsi_cpuid_max_extended_leaf:
    push    rbx
    mov     eax, 0x80000000
    xor     ecx, ecx
    cpuid
    pop     rbx
    ret

; void fsi_get_vendor_string(char *buf)
; Windows x64: RCX = puntatore buffer
fsi_get_vendor_string:
    push    rbx
    push    rsi
    mov     rsi, rcx
    
    xor     eax, eax
    xor     ecx, ecx
    cpuid
    
    mov     dword [rsi + 0], ebx
    mov     dword [rsi + 4], edx
    mov     dword [rsi + 8], ecx
    mov     byte  [rsi + 12], 0
    
    pop     rsi
    pop     rbx
    ret