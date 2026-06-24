; registers.asm 

section .text

global fsi_read_cr0
global fsi_read_cr2
global fsi_read_cr3
global fsi_read_cr4
global fsi_read_rflags

fsi_read_cr0:
    mov     rax, cr0
    ret

fsi_read_cr2:
    mov     rax, cr2
    ret

fsi_read_cr3:
    mov     rax, cr3
    ret

fsi_read_cr4:
    mov     rax, cr4
    ret

fsi_read_rflags:
    pushfq
    pop     rax
    ret