[BITS 16]
[ORG 0x7C00]

start:
    cli
    jmp 0x0000:clear_cs

clear_cs:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BOOT_DRIVE], dl

load_kernel:
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 50
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    
    int 0x13
    jc reset_and_retry
    jmp continue_boot

reset_and_retry:
    mov ah, 0x00
    mov dl, [BOOT_DRIVE]
    int 0x13
    
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 50
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    int 0x13
    
    jc continue_boot

continue_boot:
    call enable_a20
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp CODE_SEG:protected_mode

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    
    mov cx, 100
.wait:
    loop .wait
    ret

[BITS 32]
protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov esp, 0x90000
    mov ebp, esp
    
    call 0x10000
    
hang:
    cli
    hlt
    jmp hang

gdt_start:
    dq 0

gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xAA55