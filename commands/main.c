void cmd_fetch(void) {
    terminal_write("\n    ___           _     _\n");
    terminal_write("   / __\\___  __ _| | __| | ___ _ __\n");
    terminal_write("  / _\\/ _ \\/ _` | |/ _` |/ _ \\ '_ \\\n");
    terminal_write(" / / |  __/ (_| | | (_| |  __/ | | |\n");
    terminal_write(" \\/   \\___|\\__,_|_|\\__,_|\\___|_| |_|\n\n");
    terminal_write(" OS:        HaldenOS 1.0.0\n");
    terminal_write(" Kernel:    1.0.0-halden\n");
    terminal_write(" Arch:      x86_64\n");
    terminal_write(" CPU:       ");
    terminal_write(cpu_brand_string);
    terminal_write("\n Cores:     ");
    char s[16]; uint_to_str(cpu_core_count, s); terminal_write(s);
    terminal_write("\n Memory:    ");
    uint_to_str(total_memory_kb/1024, s); terminal_write(s); terminal_write(" MB\n");
    terminal_write(" Disks:     ");
    uint_to_str(disk_count, s); terminal_write(s); terminal_write(" detected\n");
    terminal_write(is_virtualized() ? " VM:        Yes\n\n" : " VM:        No\n\n");
}

void cmd_ls(const char* arg) {
    if(!arg || !strlen(arg)) {
        if(strcmp(current_directory, "/") == 0) {
            terminal_write("/dev\n/etc\n");
        } else if(strcmp(current_directory, "/dev") == 0) {
            for(int i = 0; i < 4; i++) { terminal_write(files[i].name); terminal_write("\n"); }
        } else if(strcmp(current_directory, "/etc") == 0) {
            for(int i = 4; i < FILE_COUNT; i++) { terminal_write(files[i].name); terminal_write("\n"); }
        }
    } else if(strcmp(arg, "/dev") == 0) {
        for(int i = 0; i < 4; i++) { terminal_write(files[i].name); terminal_write("\n"); }
    } else if(strcmp(arg, "/etc") == 0) {
        for(int i = 4; i < FILE_COUNT; i++) { terminal_write(files[i].name); terminal_write("\n"); }
    } else {
        terminal_write("ls: cannot access: No such directory\n");
    }
}

void cmd_cd(const char* arg) {
    if(!arg || !strlen(arg) || strcmp(arg, "~") == 0 || strcmp(arg, "/") == 0) {
        strcpy(current_directory, "/");
    } else if(strcmp(arg, "/dev") == 0 || strcmp(arg, "dev") == 0) {
        strcpy(current_directory, "/dev");
    } else if(strcmp(arg, "/etc") == 0 || strcmp(arg, "etc") == 0) {
        strcpy(current_directory, "/etc");
    } else if(strcmp(arg, "..") == 0) {
        strcpy(current_directory, "/");
    } else {
        terminal_write("cd: no such directory\n");
    }
}

void cmd_cat(const char* arg) {
    for(int i = 0; i < FILE_COUNT; i++) {
        if(strcmp(files[i].name, arg) == 0) {
            terminal_write(files[i].content); terminal_write("\n");
            return;
        }
    }
    terminal_write("cat: no such file\n");
}

void cmd_pwd(void) { terminal_write(current_directory); terminal_write("\n"); }
void cmd_whoami(void) { terminal_write("root\n"); }
void cmd_hostname(void) { terminal_write("halden-system\n"); }

void cmd_echo(const char* arg) {
    if(arg) terminal_write(arg);
    terminal_write("\n");
}

void cmd_uname(const char* arg) {
    if(!arg || !strlen(arg)) terminal_write("HaldenOS\n");
    else if(strcmp(arg, "-a") == 0) terminal_write("HaldenOS 1.0.0 x86_64\n");
    else if(strcmp(arg, "-r") == 0) terminal_write("1.0.0\n");
    else if(strcmp(arg, "-m") == 0) terminal_write("x86_64\n");
}

void cmd_df(void) {
    terminal_write("Filesystem  Size  Used  Avail  Use%\n");
    for(int i = 0; i < disk_count; i++) {
        if(detected_disks[i].exists) {
            terminal_write("/dev/"); terminal_write(detected_disks[i].name);
            terminal_write("   ");
            char s[16]; uint_to_str(detected_disks[i].size_mb, s);
            terminal_write(s); terminal_write("M  15%\n");
        }
    }
}

void cmd_free(void) {
    terminal_write("       total    used    free\n");
    terminal_write("Mem:   ");
    char s[16]; uint_to_str(total_memory_kb, s); terminal_write(s);
    terminal_write("   ");
    uint32_t used = total_memory_kb * 17 / 100;
    uint_to_str(used, s); terminal_write(s);
    terminal_write("   ");
    uint_to_str(total_memory_kb - used, s); terminal_write(s);
    terminal_write("\n");
}

void cmd_lscpu(void) {
    terminal_write("Architecture:  x86_64\n");
    terminal_write("CPU(s):        ");
    char s[16]; uint_to_str(cpu_core_count, s); terminal_write(s); terminal_write("\n");
    terminal_write("Vendor:        "); terminal_write(cpu_vendor_string); terminal_write("\n");
    terminal_write("Model:         "); terminal_write(cpu_brand_string); terminal_write("\n");
}

void cmd_lsblk(void) {
    terminal_write("NAME  SIZE\n");
    for(int i = 0; i < disk_count; i++) {
        if(detected_disks[i].exists) {
            terminal_write(detected_disks[i].name); terminal_write("  ");
            char s[16]; uint_to_str(detected_disks[i].size_mb, s);
            terminal_write(s); terminal_write("M\n");
        }
    }
}

void cmd_ps(void) {
    terminal_write("PID  CMD\n  1  init\n  2  bash\n");
}

void cmd_env(void) {
    terminal_write("PATH=/bin\nHOME=/root\nSHELL=/bin/bash\nUSER=root\n");
}

void cmd_help(void) {
    terminal_write("Commands:\n");
    terminal_write(" fetch     - System info\n");
    terminal_write(" ls [dir]  - List directory\n");
    terminal_write(" cd [dir]  - Change directory\n");
    terminal_write(" pwd       - Working directory\n");
    terminal_write(" cat       - Display file\n");
    terminal_write(" echo      - Print text\n");
    terminal_write(" uname     - System info\n");
    terminal_write(" df        - Disk usage\n");
    terminal_write(" free      - Memory usage\n");
    terminal_write(" lscpu     - CPU info\n");
    terminal_write(" lsblk     - Block devices\n");
    terminal_write(" ps        - Processes\n");
    terminal_write(" env       - Environment\n");
    terminal_write(" clear     - Clear screen\n");
}

void process_command(const char* cmd) {
    if(strcmp(cmd, "fetch") == 0) cmd_fetch();
    else if(strcmp(cmd, "ls") == 0) cmd_ls(0);
    else if(strncmp(cmd, "ls ", 3) == 0) cmd_ls(cmd + 3);
    else if(strcmp(cmd, "cd") == 0) cmd_cd(0);
    else if(strncmp(cmd, "cd ", 3) == 0) cmd_cd(cmd + 3);
    else if(strcmp(cmd, "pwd") == 0) cmd_pwd();
    else if(strncmp(cmd, "cat ", 4) == 0) cmd_cat(cmd + 4);
    else if(strncmp(cmd, "echo ", 5) == 0) cmd_echo(cmd + 5);
    else if(strcmp(cmd, "whoami") == 0) cmd_whoami();
    else if(strcmp(cmd, "hostname") == 0) cmd_hostname();
    else if(strcmp(cmd, "uname") == 0) cmd_uname(0);
    else if(strncmp(cmd, "uname ", 6) == 0) cmd_uname(cmd + 6);
    else if(strcmp(cmd, "df") == 0) cmd_df();
    else if(strcmp(cmd, "free") == 0) cmd_free();
    else if(strcmp(cmd, "lscpu") == 0) cmd_lscpu();
    else if(strcmp(cmd, "lsblk") == 0) cmd_lsblk();
    else if(strcmp(cmd, "ps") == 0) cmd_ps();
    else if(strcmp(cmd, "env") == 0) cmd_env();
    else if(strcmp(cmd, "help") == 0) cmd_help();
    else if(strcmp(cmd, "clear") == 0) terminal_clear();
    else if(strcmp(cmd, "") != 0) {
        terminal_write("bash: command not found\n");
    }
}