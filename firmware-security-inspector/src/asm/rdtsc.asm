; rdtsc.asm - Utility TSC for Windows x64

section .text

global fsi_rdtsc_value
global fsi_rdtscp
global fsi_xgetbv
global fsi_cpu_has_cpuid

; uint64_t fsi_rdtsc_value(void)
fsi_rdtsc_value:
    push    rbx
    rdtsc
    shl     rdx, 32
    or      rax, rdx
    pop     rbx
    ret

; uint64_t fsi_rdtscp(uint32_t *aux)
; Windows x64: RCX = puntatore aux
fsi_rdtscp:
    push    rbx
    push    rsi
    mov     rsi, rcx
    
    rdtscp
    shl     rdx, 32
    or      rax, rdx
    
    test    rsi, rsi
    jz      .done
    mov     dword [rsi], ecx
.done:
    pop     rsi
    pop     rbx
    ret

; uint64_t fsi_xgetbv(uint32_t xcr_index)
; Windows x64: RCX = xcr_index
fsi_xgetbv:
    push    rbx
    mov     ecx, edx           ; xcr_index in ECX (RDX for Windows x64)
    xgetbv
    shl     rdx, 32
    or      rax, rdx
    pop     rbx
    ret

; int fsi_cpu_has_cpuid(void)
fsi_cpu_has_cpuid:
    push    rbx
    
    pushfq
    pop     rax
    mov     rcx, rax
    
    xor     rax, (1 << 21)
    push    rax
    popfq
    
    pushfq
    pop     rax
    
    push    rcx
    popfq
    
    xor     rax, rcx
    shr     rax, 21
    and     rax, 1
    
    pop     rbx
    ret