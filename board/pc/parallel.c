#include "board/pc/parallel.h"

#include "board/pc/io.h"
#include "kernel/device.h"
#include "lib/utils.h"

#define PC_PARALLEL_PORT_0 0x378
#define PC_PARALLEL_CONTROL_IDLE   0x0C
#define PC_PARALLEL_CONTROL_STROBE 0x0D

typedef struct {
    uint16_t base_port;
    bool initialized;
} pc_parallel_t;

static pc_parallel_t pc_parallel0 = {
    .base_port = PC_PARALLEL_PORT_0,
};

static result_t pc_parallel_init(device_t *device)
{
    pc_parallel_t *parallel = device_data(device);

    pc_io_write8(parallel->base_port + 2, PC_PARALLEL_CONTROL_IDLE);
    parallel->initialized = true;
    return RESULT_OK;
}

static bool pc_parallel_present(const pc_parallel_t *parallel)
{
    return pc_io_read8(parallel->base_port + 1) != 0xFF;
}

static int pc_parallel_write(device_t *device, const void *buffer, size_t length)
{
    pc_parallel_t *parallel = device_data(device);
    const uint8_t *data = buffer;

    if (!parallel->initialized || !buffer) {
        return -1;
    }

    for (size_t i = 0; i < length; i++) {
        uint16_t port = parallel->base_port;

        pc_io_write8(port, data[i]);
        pc_io_write8(port + 2, PC_PARALLEL_CONTROL_STROBE);
        pc_io_write8(port + 2, PC_PARALLEL_CONTROL_IDLE);
    }

    return (int)length;
}

static const device_descriptor_t pc_parallel0_device = {
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