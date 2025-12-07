AS = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASFLAGS_BOOT = -f bin
ASFLAGS_KERNEL = -f elf32
CFLAGS = -m32 -ffreestanding -c -fno-pie
LDFLAGS = --oformat binary -melf_i386

IMG = haldenos.img

all: $(IMG)

boot.bin: boot.asm
	$(AS) $(ASFLAGS_BOOT) boot.asm -o boot.bin

kernel_asm.o: kernel_entry.asm
	$(AS) $(ASFLAGS_KERNEL) kernel_entry.asm -o kernel_asm.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

kernel.bin: kernel_asm.o kernel.o
	$(LD) $(LDFLAGS) -Ttext 0x10000 kernel_asm.o kernel.o -o kernel.bin

$(IMG): boot.bin kernel.bin
	dd if=/dev/zero of=$(IMG) bs=512 count=2880 2>/dev/null
	dd if=boot.bin of=$(IMG) conv=notrunc 2>/dev/null
	dd if=kernel.bin of=$(IMG) seek=1 conv=notrunc 2>/dev/null
	@echo "=========================================="
	@echo "Bootable IMG created: $(IMG)"
	@echo "=========================================="
	@echo "Testing boot signature..."
	@xxd -l 512 $(IMG) | tail -1
	@echo "=========================================="
	@echo "Import into UTM or test: make run"
	@echo "=========================================="

clean:
	rm -f *.o *.bin $(IMG)

run: $(IMG)
	qemu-system-x86_64 -drive format=raw,file=$(IMG),if=ide -m 512M

.PHONY: all clean run