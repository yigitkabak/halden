# Halden

A minimal 32-bit operating system kernel written in C and Assembly, designed for x86_64 architecture.

## Features

- **Custom Bootloader**: 16-bit real mode bootloader with protected mode transition
- **Kernel**: 32-bit kernel with VGA text mode terminal
- **Command Line Interface**: Interactive bash-like shell with common Unix commands
- **Hardware Drivers**:
  - Intel CPU driver with feature detection (SSE, AVX, Hyper-Threading, Turbo Boost)
  - AMD CPU driver with AMD-specific features (3DNow!, XOP, FMA4, SVM)
  - Ethernet/NIC driver with PCI device detection
- **POSIX Stubs**: Basic POSIX-compliant function stubs
- **System Information**: CPU detection, memory detection, disk detection

## Project Structure

```
HaldenOS/
├── boot/
│   ├── boot.asm          # Bootloader (real mode → protected mode)
│   └── kernel.asm        # Kernel entry point
├── drivers/
│   ├── intel.c           # Intel processor driver
│   ├── amd.c             # AMD processor driver
│   └── ethernet.c        # Ethernet/NIC driver
├── posix/
│   └── posix.c           # POSIX function stubs
├── commands/
│   └── main.c            # Command implementations
├── build/                # Compiled object files (auto-generated)
├── kernel.c              # Main kernel code
├── linker.ld             # Linker script
├── Makefile              # Build configuration
└── haldenos.img          # Bootable disk image (output)
```

## Requirements

- **Assembler**: NASM (Netwide Assembler)
- **Compiler**: x86_64-elf-gcc (cross-compiler)
- **Linker**: x86_64-elf-ld
- **Emulator**: QEMU (for testing)
- **Make**: GNU Make

### Installing Dependencies

**macOS (using Homebrew)**:
```bash
brew install nasm qemu
brew install x86_64-elf-gcc x86_64-elf-binutils
```

**Linux (Debian/Ubuntu)**:
```bash
sudo apt install nasm qemu-system-x86 build-essential
```

For cross-compiler, you may need to build it manually or use a distribution-specific package.

## Building

Build the OS image:
```bash
make
```

This will create `haldenos.img`, a bootable 1.44MB floppy disk image.

Clean build artifacts:
```bash
make clean
```

## Running

Run in QEMU:
```bash
make run
```

Or manually:
```bash
qemu-system-x86_64 -drive format=raw,file=haldenos.img,if=ide -m 512M
```

## Available Commands

Once booted, HaldenOS provides the following commands:

- `fetch` - Display system information with ASCII art
- `ls [dir]` - List directory contents
- `cd [dir]` - Change directory
- `pwd` - Print working directory
- `cat <file>` - Display file contents
- `echo <text>` - Print text to terminal
- `uname [-a|-r|-m]` - System information
- `df` - Disk usage
- `free` - Memory usage
- `lscpu` - CPU information
- `lsblk` - Block devices
- `ps` - Process list
- `env` - Environment variables
- `clear` - Clear screen
- `help` - Show available commands
- `whoami` - Current user
- `hostname` - System hostname
- `date` - Current date
- `uptime` - System uptime

## File System

HaldenOS includes a minimal read-only filesystem with the following structure:

```
/
├── dev/
│   ├── boot.asm
│   ├── kernel.c
│   ├── linker.ld
│   └── Makefile
├── etc/
│   ├── os-release
│   ├── hostname
│   ├── passwd
└── └── hosts

```

## Technical Details

- **Architecture**: x86 (32-bit protected mode)
- **Memory**: Uses BIOS INT 13h for loading, CMOS for memory detection
- **Display**: VGA text mode (80x25 characters)
- **Keyboard**: PS/2 keyboard via scancode translation
- **Disk**: IDE/ATA disk detection (up to 4 drives)
- **CPU**: CPUID-based detection with Intel/AMD specific optimizations

## License

This project is licensed [GNU General Public License](LICENSE)