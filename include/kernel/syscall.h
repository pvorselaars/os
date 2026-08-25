#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/base.h>
#include <syscall.h>

typedef enum {
    SYSCALL_WRITE,
    SYSCALL_COUNT
} syscall_number_t;

__attribute__((always_inline))
inline result_t sys_write(const char *text) {
    return arch_syscall(SYSCALL_WRITE, (uint64_t)text);
}

#endif
