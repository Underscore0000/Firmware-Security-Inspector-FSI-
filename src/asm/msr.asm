ì; msr.asm - RDMSR for Windows x64

section .text

global fsi_rdmsr

; uint64_t fsi_rdmsr(uint32_t msr_index)
; Windows x64: RCX = msr_index
fsi_rdmsr:
    push    rbx
    mov     ecx, edx           ; index MSR on ECX (RDX for Windows x64)
    rdmsr
    shl     rdx, 32
    or      rax, rdx
    pop     rbx
    ret