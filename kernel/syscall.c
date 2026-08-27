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

syscall_handler_t* syscall_handlers[SYSCALL_COUNT] = {
    syscall_write
};

result_t syscall_handle(uint64_t number, uint64_t arg1)
{
    if (number >= SYSCALL_COUNT)
        return RESULT_ERROR;

    syscall_handler_t *handler = syscall_handlers[number];

    if (!handler)
        return RESULT_ERROR;

    return handler(arg1);
}