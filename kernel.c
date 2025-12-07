#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
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

typedef struct {
    char name[32];
    uint32_t size_mb;
    int exists;
} disk_info;

typedef struct {
    char name[64];
    char path[128];
    char content[512];
} file_entry;

file_entry files[] = {
    {"boot.asm", "/dev/boot.asm", "[BITS 16]\n[ORG 0x7C00]\nstart:\n    cli"},
    {"kernel.c", "/dev/kernel.c", "#include <stdint.h>\nvoid kernel_main(void) {}"},
    {"linker.ld", "/dev/linker.ld", "OUTPUT_FORMAT(elf64-x86-64)\nENTRY(_start)"},
    {"Makefile", "/dev/Makefile", "AS = nasm\nCC = x86_64-elf-gcc"},
    {"os-release", "/etc/os-release", "NAME=\"HaldenOS\"\nVERSION=\"1.0.0\"\nID=haldenos"},
    {"hostname", "/etc/hostname", "halden-system"},
    {"passwd", "/etc/passwd", "root:x:0:0:root:/root:/bin/bash"},
    {"hosts", "/etc/hosts", "127.0.0.1   localhost"},
};

#define FILE_COUNT 8

disk_info detected_disks[16];
int disk_count = 0;
uint32_t total_memory_kb = 0;
char cpu_vendor_string[13] = {0};
char cpu_brand_string[49] = {0};
int cpu_core_count = 0;
char current_directory[128] = "/";

void terminal_clear(void);
void terminal_write(const char* str);
void terminal_putchar(char c);
void update_cursor(void);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char val);
unsigned short inw(unsigned short port);
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
void strcpy(char* dest, const char* src);
int strncmp(const char* s1, const char* s2, size_t n);
void uint_to_str(uint32_t num, char* str);
void process_command(const char* cmd);
char scancode_to_char(unsigned char scancode);

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
            vga_buffer[(y-1)*VGA_WIDTH+x] = vga_buffer[y*VGA_WIDTH+x];
        }
    }
    for(size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT-1)*VGA_WIDTH+x].character = ' ';
        vga_buffer[(VGA_HEIGHT-1)*VGA_WIDTH+x].color = terminal_color;
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if(c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        vga_buffer[terminal_row*VGA_WIDTH+terminal_column].character = c;
        vga_buffer[terminal_row*VGA_WIDTH+terminal_column].color = terminal_color;
        terminal_column++;
    }
    if(terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    if(terminal_row >= VGA_HEIGHT) terminal_scroll();
    update_cursor();
}

void terminal_write(const char* str) {
    for(size_t i = 0; str[i] != '\0'; i++) terminal_putchar(str[i]);
}

void update_cursor(void) {
    unsigned short pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
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

size_t strlen(const char* str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while((*dest++ = *src++));
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(n && *s1 && (*s1 == *s2)) { s1++; s2++; n--; }
    if(n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void uint_to_str(uint32_t num, char* str) {
    if(num == 0) { str[0] = '0'; str[1] = '\0'; return; }
    char temp[32];
    int i = 0;
    while(num > 0) { temp[i++] = '0' + (num % 10); num /= 10; }
    int j = 0;
    while(i > 0) str[j++] = temp[--i];
    str[j] = '\0';
}

uint32_t get_memory_kb(void) {
    outb(0x70, 0x30);
    uint32_t low = inb(0x71);
    outb(0x70, 0x31);
    uint32_t high = inb(0x71);
    uint32_t kb = (low | (high << 8));
    if(kb == 0) {
        outb(0x70, 0x17);
        low = inb(0x71);
        outb(0x70, 0x18);
        high = inb(0x71);
        kb = (low | (high << 8));
    }
    return kb + 1024;
}

void get_cpu_vendor(char* vendor) {
    uint32_t ebx, edx, ecx;
    asm volatile("cpuid" : "=b"(ebx), "=d"(edx), "=c"(ecx) : "a"(0));
    for(int i = 0; i < 4; i++) vendor[i] = (char)((ebx >> (i*8)) & 0xFF);
    for(int i = 0; i < 4; i++) vendor[i+4] = (char)((edx >> (i*8)) & 0xFF);
    for(int i = 0; i < 4; i++) vendor[i+8] = (char)((ecx >> (i*8)) & 0xFF);
    vendor[12] = '\0';
}

void get_cpu_brand(char* brand) {
    uint32_t eax, ebx, ecx, edx;
    int idx = 0;
    for(uint32_t func = 0x80000002; func <= 0x80000004; func++) {
        asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func));
        for(int i = 0; i < 4; i++) brand[idx++] = (char)((eax >> (i*8)) & 0xFF);
        for(int i = 0; i < 4; i++) brand[idx++] = (char)((ebx >> (i*8)) & 0xFF);
        for(int i = 0; i < 4; i++) brand[idx++] = (char)((ecx >> (i*8)) & 0xFF);
        for(int i = 0; i < 4; i++) brand[idx++] = (char)((edx >> (i*8)) & 0xFF);
    }
    brand[idx] = '\0';
}

int get_cpu_cores(void) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    return ((ebx >> 16) & 0xFF);
}

int is_virtualized(void) {
    uint32_t eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    return (ecx & (1 << 31)) != 0;
}

void detect_disks(void) {
    disk_count = 0;
    for(int drive = 0; drive < 4; drive++) {
        unsigned char status;
        if(drive < 2) {
            outb(0x1F6, 0xA0 | (drive << 4));
            outb(0x1F7, 0xEC);
            for(volatile int i = 0; i < 10000; i++);
            status = inb(0x1F7);
        } else {
            outb(0x176, 0xA0 | ((drive-2) << 4));
            outb(0x177, 0xEC);
            for(volatile int i = 0; i < 10000; i++);
            status = inb(0x177);
        }
        if(status == 0 || status == 0xFF || !(status & 0x08)) continue;
        
        unsigned short identify[256];
        unsigned short port = (drive < 2) ? 0x1F0 : 0x170;
        for(int i = 0; i < 256; i++) identify[i] = inw(port);
        
        detected_disks[disk_count].name[0] = 's';
        detected_disks[disk_count].name[1] = 'd';
        detected_disks[disk_count].name[2] = 'a' + disk_count;
        detected_disks[disk_count].name[3] = '\0';
        detected_disks[disk_count].size_mb = (((uint32_t)identify[61] << 16) | identify[60]) / 2048;
        detected_disks[disk_count].exists = 1;
        disk_count++;
    }
}

#include "commands/main.c"

void kernel_main(void) {
    terminal_clear();
    total_memory_kb = get_memory_kb();
    get_cpu_vendor(cpu_vendor_string);
    get_cpu_brand(cpu_brand_string);
    cpu_core_count = get_cpu_cores();
    detect_disks();
    
    terminal_write("  _   _    _    _     ____  _____ _   _ \n");
    terminal_write(" | | | |  / \\  | |   |  _ \\| ____| \\ | |\n");
    terminal_write(" | |_| | / _ \\ | |   | | | |  _| |  \\| |\n");
    terminal_write(" |  _  |/ ___ \\| |___| |_| | |___| |\\  |\n");
    terminal_write(" |_| |_/_/   \\_\\_____|____/|_____|_| \\_|\n\n");
    
    for(volatile int i = 0; i < 5000000; i++);
    terminal_write("kernel is loading...\n");
    for(volatile int i = 0; i < 3000000; i++);
    terminal_write("welcome to halden\n\n");
    for(volatile int i = 0; i < 2000000; i++);
    
    terminal_write("HaldenOS V1.0.0 - 64-bit\n");
    terminal_write("Type 'fetch' or 'help' for information\n\n");
    
    char input_buffer[256];
    int buffer_pos = 0;
    unsigned char last_scancode = 0;
    
    while(1) {
        terminal_write("bash# ");
        buffer_pos = 0;
        
        while(1) {
            if(!(inb(0x64) & 0x01)) continue;
            unsigned char scancode = inb(0x60);
            if(scancode == last_scancode) continue;
            last_scancode = scancode;
            if(scancode & 0x80) continue;
            
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
                    vga_buffer[terminal_row*VGA_WIDTH+terminal_column].character = ' ';
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

char scancode_to_char(unsigned char scancode) {
    static const char map[89] = {
        0,0,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
        0,'q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0,'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,'\\','z','x','c','v','b','n','m',',','.','/',0,0,0,' '
    };
    return (scancode < 89) ? map[scancode] : 0;
}