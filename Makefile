AS = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASFLAGS_BOOT = -f bin
ASFLAGS_KERNEL = -f elf32
CFLAGS = -m32 -ffreestanding -c -fno-pie -I.
LDFLAGS = --oformat binary -melf_i386

BUILD_DIR = build
IMG = haldenos.img

all: $(IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.bin: boot/boot.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS_BOOT) boot/boot.asm -o $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/kernel_asm.o: boot/kernel.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS_KERNEL) boot/kernel.asm -o $(BUILD_DIR)/kernel_asm.o

$(BUILD_DIR)/kernel.o: kernel.c commands/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) kernel.c -o $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/posix.o: posix/posix.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) posix/posix.c -o $(BUILD_DIR)/posix.o

$(BUILD_DIR)/intel.o: drivers/intel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) drivers/intel.c -o $(BUILD_DIR)/intel.o

$(BUILD_DIR)/amd.o: drivers/amd.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) drivers/amd.c -o $(BUILD_DIR)/amd.o

$(BUILD_DIR)/ethernet.o: drivers/ethernet.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) drivers/ethernet.c -o $(BUILD_DIR)/ethernet.o

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_asm.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/posix.o $(BUILD_DIR)/intel.o $(BUILD_DIR)/amd.o $(BUILD_DIR)/ethernet.o
	$(LD) $(LDFLAGS) -Ttext 0x10000 $(BUILD_DIR)/kernel_asm.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/posix.o $(BUILD_DIR)/intel.o $(BUILD_DIR)/amd.o $(BUILD_DIR)/ethernet.o -o $(BUILD_DIR)/kernel.bin

$(IMG): $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$(IMG) bs=512 count=2880 2>/dev/null
	dd if=$(BUILD_DIR)/boot.bin of=$(IMG) conv=notrunc 2>/dev/null
	dd if=$(BUILD_DIR)/kernel.bin of=$(IMG) seek=1 conv=notrunc 2>/dev/null
	@echo "=========================================="
	@echo "HaldenOS built successfully"
	@echo "=========================================="

clean:
	rm -rf $(BUILD_DIR) $(IMG)

run: $(IMG)
	qemu-system-x86_64 -drive format=raw,file=$(IMG),if=ide -m 512M

.PHONY: all clean run