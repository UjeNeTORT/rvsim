#pragma once

extern long read( int fd, char* data, long maxlen);
extern long write( int fd, const char* data, long len);
extern __attribute__((noreturn)) void exit( long status);

static int read_exact(void* dst, long len)
{
    char* ptr = (char*)dst;
    long read_total = 0;

    while (read_total < len) {
        long chunk = read(0, ptr + read_total, len - read_total);
        if (chunk <= 0) {
            return 0;
        }
        read_total += chunk;
    }

    return 1;
}

static void write_all(const void* src, long len) {
    const char* ptr = (const char*)src;
    long written = 0;

    while (written < len) {
        long chunk = write(1, ptr + written, len - written);
        if (chunk <= 0) {
            exit(1);
        }
        written += chunk;
    }
}
