#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned int size_t;

typedef struct {
    uint8_t character;
    uint8_t color;
} vga_char;

static vga_char* vga_buffer = (vga_char*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0F;

unsigned char inb(unsigned short port);
char scancode_to_char(unsigned char scancode);
void outb(unsigned short port, unsigned char val);
void update_cursor(void);
void detect_disks(void);
unsigned short inw(unsigned short port);

typedef struct {
    char name[32];
    uint32_t size_mb;
    int exists;
} disk_info;

disk_info detected_disks[16];
int disk_count = 0;

void terminal_clear(void) {
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index].character = ' ';
            vga_buffer[index].color = terminal_color;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_scroll(void) {
    for(size_t y = 1; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src = y * VGA_WIDTH + x;
            const size_t dst = (y - 1) * VGA_WIDTH + x;
            vga_buffer[dst] = vga_buffer[src];
        }
    }
    
    for(size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buffer[index].character = ' ';
        vga_buffer[index].color = terminal_color;
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if(c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        vga_buffer[index].character = c;
        vga_buffer[index].color = terminal_color;
        terminal_column++;
    }
    
    if(terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    
    if(terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
    
    update_cursor();
}

void terminal_write(const char* str) {
    for(size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while((*dest++ = *src++));
}

typedef struct {
    char name[64];
    int is_dir;
} file_entry;

file_entry root_files[] = {
    {".", 1},
    {"..", 1},
    {"home", 1},
    {"dev", 1},
    {"etc", 1},
    {"bin", 1},
    {"readme.txt", 0},
    {"system.conf", 0}
};

#define ROOT_FILE_COUNT 8

void detect_disks(void) {
    disk_count = 0;
    
    for(int drive = 0; drive < 4; drive++) {
        unsigned char status;
        
        if(drive < 2) {
            outb(0x1F6, 0xA0 | (drive << 4));
            outb(0x1F2, 0);
            outb(0x1F3, 0);
            outb(0x1F4, 0);
            outb(0x1F5, 0);
            outb(0x1F7, 0xEC);
            
            for(volatile int i = 0; i < 10000; i++);
            
            status = inb(0x1F7);
        } else {
            outb(0x176, 0xA0 | ((drive - 2) << 4));
            outb(0x172, 0);
            outb(0x173, 0);
            outb(0x174, 0);
            outb(0x175, 0);
            outb(0x177, 0xEC);
            
            for(volatile int i = 0; i < 10000; i++);
            
            status = inb(0x177);
        }
        
        if(status == 0 || status == 0xFF) {
            continue;
        }
        
        if(!(status & 0x08)) {
            continue;
        }
        
        unsigned short identify[256];
        unsigned short port = (drive < 2) ? 0x1F0 : 0x170;
        
        for(int i = 0; i < 256; i++) {
            identify[i] = inw(port);
        }
        
        char name[32];
        name[0] = 's';
        name[1] = 'd';
        name[2] = 'a' + disk_count;
        name[3] = '\0';
        
        strcpy(detected_disks[disk_count].name, name);
        
        unsigned int lba28 = (identify[61] << 16) | identify[60];
        unsigned int size_mb = (lba28 / 2) / 1024;
        
        if(size_mb == 0) {
            size_mb = 8192;
        }
        
        detected_disks[disk_count].size_mb = size_mb;
        detected_disks[disk_count].exists = 1;
        disk_count++;
    }
    
    if(disk_count == 0) {
        strcpy(detected_disks[0].name, "sda");
        detected_disks[0].size_mb = 8192;
        detected_disks[0].exists = 1;
        disk_count = 1;
    }
}

int is_virtualized(void) {
    unsigned int eax, ebx, ecx, edx;
    eax = 1;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
    return (ecx & (1 << 31)) != 0;
}

void cmd_ls(void) {
    terminal_write("Directory listing:\n");
    for(int i = 0; i < ROOT_FILE_COUNT; i++) {
        terminal_write("  ");
        terminal_write(root_files[i].name);
        if(root_files[i].is_dir) {
            terminal_write("/");
        }
        terminal_write("\n");
    }
}

void cmd_mkdir(const char* arg) {
    terminal_write("mkdir: created directory '");
    terminal_write(arg);
    terminal_write("'\n");
}

void cmd_cat(const char* arg) {
    if(strcmp(arg, "readme.txt") == 0) {
        terminal_write("Welcome to HaldenOS V1\n");
        terminal_write("A minimal operating system\n");
    } else if(strcmp(arg, "system.conf") == 0) {
        terminal_write("version=1.0\n");
        terminal_write("arch=x86_64\n");
    } else {
        terminal_write("cat: ");
        terminal_write(arg);
        terminal_write(": No such file\n");
    }
}

void cmd_info(void) {
    terminal_write("HaldenOS V1\n");
    terminal_write("Architecture: x86_64\n");
    terminal_write("Boot: UEFI\n");
    
    if(is_virtualized()) {
        terminal_write("Environment: Virtual Machine\n\n");
    } else {
        terminal_write("Environment: Physical Hardware\n\n");
    }
    
    terminal_write("Available disks:\n");
    for(int i = 0; i < disk_count; i++) {
        if(detected_disks[i].exists) {
            terminal_write("  /dev/");
            terminal_write(detected_disks[i].name);
            terminal_write(" - ");
            char size_str[32];
            uint32_t size = detected_disks[i].size_mb;
            int idx = 0;
            if(size == 0) {
                size_str[idx++] = '0';
            } else {
                char temp[32];
                int temp_idx = 0;
                while(size > 0) {
                    temp[temp_idx++] = '0' + (size % 10);
                    size /= 10;
                }
                for(int j = temp_idx - 1; j >= 0; j--) {
                    size_str[idx++] = temp[j];
                }
            }
            size_str[idx] = '\0';
            terminal_write(size_str);
            terminal_write(" MB\n");
        }
    }
}

void cmd_install(void) {
    terminal_write("HaldenOS Installer\n");
    terminal_write("===================\n\n");
    terminal_write("Available disks:\n");
    for(int i = 0; i < disk_count; i++) {
        if(detected_disks[i].exists) {
            terminal_write("  [");
            char num[2] = {'0' + i, '\0'};
            terminal_write(num);
            terminal_write("] /dev/");
            terminal_write(detected_disks[i].name);
            terminal_write("\n");
        }
    }
    terminal_write("\nWARNING: Selected disk will be formatted as ext4\n");
    terminal_write("All data will be lost!\n");
    terminal_write("Installation simulated - selecting disk 0\n");
    terminal_write("Formatting /dev/");
    terminal_write(detected_disks[0].name);
    terminal_write(" as ext4...\n");
    terminal_write("Creating directories:\n");
    terminal_write("  /dev\n");
    terminal_write("  /etc\n");
    terminal_write("  /home\n");
    terminal_write("  /bin\n");
    terminal_write("  /boot\n");
    terminal_write("Installing kernel...\n");
    terminal_write("Installing bootloader...\n");
    terminal_write("Installation complete!\n");
}

void process_command(const char* cmd) {
    if(strcmp(cmd, "ls") == 0) {
        cmd_ls();
    } else if(strcmp(cmd, "info") == 0) {
        cmd_info();
    } else if(strcmp(cmd, "install") == 0) {
        cmd_install();
    } else if(strcmp(cmd, "clear") == 0) {
        terminal_clear();
    } else if(cmd[0] == 'm' && cmd[1] == 'k' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r' && cmd[5] == ' ') {
        cmd_mkdir(cmd + 6);
    } else if(cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't' && cmd[3] == ' ') {
        cmd_cat(cmd + 4);
    } else if(strcmp(cmd, "") != 0) {
        terminal_write("bash: ");
        terminal_write(cmd);
        terminal_write(": command not found\n");
    }
}

void kernel_main(void) {
    terminal_clear();
    detect_disks();
    
    terminal_write("kernel is loading...\n");
    terminal_write("Initializing HaldenBoot...\n");
    terminal_write("Loading filesystem drivers...\n");
    terminal_write("Mounting root filesystem...\n\n");
    terminal_write("HaldenOS V1 - 64-bit\n");
    terminal_write("Type 'info' for system information\n");
    terminal_write("Type 'install' to install the OS\n\n");
    
    char input_buffer[256];
    int buffer_pos = 0;
    unsigned char last_scancode = 0;
    
    while(1) {
        terminal_write("bash# ");
        buffer_pos = 0;
        
        while(1) {
            unsigned char status = inb(0x64);
            if(!(status & 0x01)) {
                continue;
            }
            
            unsigned char scancode = inb(0x60);
            
            if(scancode == last_scancode) {
                continue;
            }
            last_scancode = scancode;
            
            if(scancode & 0x80) {
                continue;
            }
            
            char c = scancode_to_char(scancode);
            
            if(c == '\n') {
                terminal_putchar('\n');
                input_buffer[buffer_pos] = '\0';
                process_command(input_buffer);
                break;
            } else if(c == '\b') {
                if(buffer_pos > 0) {
                    buffer_pos--;
                    terminal_column--;
                    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
                    vga_buffer[index].character = ' ';
                    update_cursor();
                }
            } else if(c != 0 && buffer_pos < 255) {
                input_buffer[buffer_pos++] = c;
                terminal_putchar(c);
            }
            
            for(volatile int i = 0; i < 10000; i++);
        }
    }
}

void update_cursor(void) {
    unsigned short pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned short inw(unsigned short port) {
    unsigned short ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

char scancode_to_char(unsigned char scancode) {
    static const char scancode_map[89] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        0, 0, ' '
    };
    
    if(scancode < 89) {
        return scancode_map[scancode];
    }
    return 0;
}