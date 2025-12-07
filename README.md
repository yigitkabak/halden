# HaldenOS

A minimal 32-bit operating system written in C and Assembly from scratch.

## Features

- Custom bootloader (no GRUB dependency)
- 32-bit protected mode kernel
- VGA text mode display (80x25)
- PS/2 keyboard support
- Basic shell with command interpretation
- Disk detection (IDE/SATA)
- Virtual machine and physical hardware detection
- Mock filesystem with basic commands

## System Requirements

### Minimum
- CPU: x86 (32-bit) or x86_64 (64-bit)
- RAM: 1 MB
- Storage: 1.44 MB
- Display: VGA compatible (text mode)
- BIOS: Legacy BIOS (not UEFI)

### Recommended
- CPU: Any x86/x64 processor (Intel/AMD)
- RAM: 512 MB
- Storage: 10 MB free space
- BIOS: Legacy BIOS mode enabled

## Building from Source

### Prerequisites

**macOS:**
```bash
brew install nasm qemu
brew tap messense/macos-cross-toolchains
brew install x86_64-unknown-linux-gnu
```

**Linux:**
```bash
sudo apt install nasm qemu-system-x86 build-essential
sudo apt install gcc-multilib
```

### Build Commands

```bash
make clean
make
make run
```

This creates `haldenos.img` - a bootable disk image.

## Running HaldenOS

### QEMU (Recommended for Testing)
```bash
make run
```

### VirtualBox
1. Create new VM (Type: Other, Version: Other/Unknown)
2. Use existing disk → Select `haldenos.img`
3. Enable Legacy BIOS mode
4. Start VM

### VMware
1. Create new VM → Custom
2. Use existing disk → Select `haldenos.img`
3. Set firmware to BIOS (not UEFI)
4. Start VM

### UTM (macOS)
1. Create new VM → Emulate → Other
2. Skip ISO boot
3. VM Settings → Drives → Import `haldenos.img`
4. Interface: IDE
5. Disable UEFI boot
6. Start VM

Note: UTM may have compatibility issues. Use QEMU for reliable testing.

### Physical Hardware (USB Boot)

**macOS:**
```bash
diskutil list
diskutil unmountDisk /dev/diskX
sudo dd if=haldenos.img of=/dev/rdiskX bs=1m
diskutil eject /dev/diskX
```

**Linux:**
```bash
lsblk
sudo dd if=haldenos.img of=/dev/sdX bs=1M status=progress
sync
```

**Windows:**
Use [Rufus](https://rufus.ie) in DD Image mode.

Replace `diskX` or `sdX` with your USB device identifier.

### Android (Limbo Emulator)
1. Install Limbo PC Emulator from Play Store
2. Copy `haldenos.img` to phone storage
3. Limbo settings:
   - Machine: PC (i386)
   - RAM: 128 MB
   - Hard Disk A: Select `haldenos.img`
   - Boot: HDD
4. Start emulation

## Available Commands

Once booted, you can use these commands in the shell:

- `info` - Display system information
- `ls` - List directory contents
- `cat <file>` - Display file contents (readme.txt, system.conf)
- `mkdir <name>` - Create directory (simulated)
- `install` - Simulate OS installation
- `clear` - Clear screen

## Project Structure

```
.
├── boot.asm          # Bootloader (512 bytes)
├── kernel_entry.asm  # Kernel entry point
├── kernel.c          # Main kernel code
├── linker.ld         # Linker script
├── Makefile          # Build configuration
└── README.md         # This file
```

## Technical Details

- **Boot Process:** Custom bootloader loads kernel from sector 2
- **Memory Layout:** Kernel loads at 0x10000 (64KB)
- **Mode:** 32-bit protected mode
- **Display:** Direct VGA memory access (0xB8000)
- **Input:** PS/2 keyboard via port 0x60
- **Disk I/O:** BIOS INT 13h for loading

## Troubleshooting

### "No bootable device" error
- Ensure Legacy BIOS is enabled (not UEFI)
- Verify boot signature with: `xxd -l 512 haldenos.img | tail -1`
- Should see `55aa` at the end

### Boots in QEMU but not in VM software
- Try converting format: `qemu-img convert -f raw -O qcow2 haldenos.img haldenos.qcow2`
- Use the converted file in your VM software

### Keyboard not working
- Try different keyboard layouts in VM settings
- Ensure VM has keyboard input focus
- Some keys may not be mapped (limited scancode support)

### Boot loop or freezing
- Increase RAM allocation to 512 MB
- Disable UEFI/Secure Boot
- Try different virtual disk interface (IDE vs SATA)

## Compatibility

### Tested and Working
- ✅ QEMU (all platforms)
- ✅ VirtualBox
- ✅ VMware Workstation/Fusion
- ✅ Physical x86/x64 hardware
- ✅ Limbo (Android)
- ✅ Bochs emulator

### Known Issues
- ⚠️ UTM may have boot compatibility issues
- ⚠️ UEFI-only systems require CSM/Legacy mode
- ❌ ARM processors (Apple M1/M2) require x86 emulation
- ❌ Does not support UEFI boot

## Development

To modify the OS:

1. Edit source files (`kernel.c`, `boot.asm`, etc.)
2. Rebuild: `make clean && make`
3. Test: `make run`

## License

[LICENSE](LICENSE)
