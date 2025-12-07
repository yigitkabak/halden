[BITS 32]
[EXTERN kernel_main]
[GLOBAL _start]

_start:
    cli
    
    mov esp, kernel_stack_top
    
    call kernel_main
    
hang:
    cli
    hlt
    jmp hang

section .bss
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top: