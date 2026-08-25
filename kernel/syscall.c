#include <lib/utils.h>
#include <kernel/device.h>
#include <kernel/syscall.h>

typedef result_t (syscall_handler_t)(uint64_t);

static result_t syscall_write(uint64_t arg1) {
    device_t* console = device_find_by_name("console0");

    if (!console)
        return RESULT_ERROR;

    device_char_write(console, (const char*)arg1, 2);

    return RESULT_OK;
}

static syscall_handler_t* syscall_handlers[SYSCALL_COUNT] = {
    syscall_write
};

result_t syscall_dispatch(uint64_t syscall, uint64_t arg1) {
    assert(syscall < SYSCALL_COUNT);
    return syscall_handlers[syscall](arg1);
}