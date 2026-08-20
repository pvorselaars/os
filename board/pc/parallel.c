#include "arch/x86_64/io.h"
#include "board/pc/parallel.h"
#include "kernel/device.h"

#define PC_PARALLEL_PORT_0 0x378

typedef struct {
    uint16_t base_port;
    bool initialized;
} pc_parallel_t;

static pc_parallel_t pc_parallel0 = {
    .base_port = PC_PARALLEL_PORT_0,
};

static result_t pc_parallel_init(device_t *device)
{
    pc_parallel_t *parallel = device->data;

    outb(parallel->base_port + 2, 0x04);
    parallel->initialized = true;
    return RESULT_OK;
}

static bool pc_parallel_present(const pc_parallel_t *parallel)
{
    return inb(parallel->base_port + 1) != 0xFF;
}

static int pc_parallel_write(device_t *device, const void *buffer, size_t length)
{
    pc_parallel_t *parallel = device->data;
    const uint8_t *data = buffer;

    if (!parallel->initialized || !buffer) {
        return -1;
    }

    for (size_t i = 0; i < length; i++) {
        uint16_t port = parallel->base_port;

        // TODO: poll on real hardware
        //while (!(inb(port + 1) & 0x80))
        //    sleep(10);

        outb(port, data[i]);

        uint8_t control = inb(port + 2);
        outb(port + 2, control | 1);
        outb(port + 2, control);
    }

    return (int)length;
}

static device_t pc_parallel0_device = {
    .name = "parallel0",
    .class = DEVICE_CLASS_CHAR,
    .init = pc_parallel_init,
    .char_ops = { .write = pc_parallel_write },
    .data = &pc_parallel0,
};


result_t pc_parallel_register(void)
{
    if (!pc_parallel_present(&pc_parallel0)) {
        return RESULT_OK;
    }

    return device_register(&pc_parallel0_device);
}