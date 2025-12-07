typedef unsigned int size_t;

int posix_open(const char* path, int flags) { return -1; }
int posix_close(int fd) { return -1; }
int posix_read(int fd, void* buf, size_t count) { return -1; }
int posix_write(int fd, const void* buf, size_t count) { return -1; }
int posix_lseek(int fd, int offset, int whence) { return -1; }
int posix_unlink(const char* path) { return -1; }
int posix_mkdir(const char* path, int mode) { return 0; }
int posix_rmdir(const char* path) { return -1; }
int posix_chdir(const char* path) { return 0; }
int posix_getpid(void) { return 1; }
int posix_getppid(void) { return 0; }
int posix_getuid(void) { return 0; }
int posix_getgid(void) { return 0; }
int posix_fork(void) { return -1; }
int posix_wait(int* status) { return -1; }
int posix_kill(int pid, int sig) { return -1; }
int posix_pipe(int fd[2]) { return -1; }
int posix_dup(int fd) { return -1; }
int posix_dup2(int old, int new) { return -1; }
int posix_access(const char* path, int mode) { return -1; }
int posix_chmod(const char* path, int mode) { return -1; }
int posix_chown(const char* path, int uid, int gid) { return -1; }
long posix_time(long* t) { long tm = 1733529600; if(t) *t = tm; return tm; }

void posix_exit(int status) {
    while(1) { asm volatile("hlt"); }
}

unsigned int posix_sleep(unsigned int sec) {
    for(volatile unsigned int i = 0; i < sec * 1000000; i++);
    return 0;
}

char* posix_getcwd(char* buf, size_t size) {
    if(buf && size > 0) { buf[0] = '/'; buf[1] = '\0'; return buf; }
    return 0;
}