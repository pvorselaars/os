#include "kernel/device.h"
#include "lib/string.h"

#define DEVICE_MAX 32

struct device {
    const device_descriptor_t *descriptor;
    device_state_t state;
    device_t *next;
};

static device_t device_storage[DEVICE_MAX];
static device_t *devices;
static uint32_t device_count;

static bool device_is_ready(const device_t *device, device_class_t class)
{
    return device && device->state == DEVICE_STATE_READY &&
           device->descriptor->class == class;
}

result_t device_register(const device_descriptor_t *descriptor)
{
    if (!descriptor || !descriptor->name ||
        descriptor->class >= DEVICE_CLASS_MAX ||
        device_find_by_name(descriptor->name) || device_count == DEVICE_MAX) {
        return RESULT_ERROR;
    }

    device_t *device = &device_storage[device_count++];
    device->descriptor = descriptor;
    device->state = DEVICE_STATE_INITIALIZING;

    if (descriptor->init && descriptor->init(device) != RESULT_OK) {
        device->state = DEVICE_STATE_ERROR;
        device_count--;
        return RESULT_ERROR;
    }

    device->next = devices;
    devices = device;
    device->state = DEVICE_STATE_READY;
    return RESULT_OK;
}

device_t *device_find_by_name(const char *name)
{
    for (device_t *device = devices; device; device = device->next) {
        if (strcmp(device->descriptor->name, name) == 0) {
            return device;
        }
    }

    return NULL;
}

device_t *device_find_by_class(device_class_t class, uint32_t index)
{
    for (device_t *device = devices; device; device = device->next) {
        if (device->state == DEVICE_STATE_READY &&
            device->descriptor->class == class && index-- == 0) {
            return device;
        }
    }

    return NULL;
}

const char *device_name(const device_t *device)
{
    return device ? device->descriptor->name : NULL;
}

device_class_t device_class(const device_t *device)
{
    return device ? device->descriptor->class : DEVICE_CLASS_MAX;
}

device_state_t device_state(const device_t *device)
{
    return device ? device->state : DEVICE_STATE_REMOVED;
}

void *device_data(device_t *device)
{
    return device ? device->descriptor->data : NULL;
}

int device_char_read(device_t *device, void *buffer, size_t length)
{
    if (!device_is_ready(device, DEVICE_CLASS_CHAR) ||
        !device->descriptor->char_ops.read) {
        return -1;
    }
    return device->descriptor->char_ops.read(device, buffer, length);
}

int device_char_write(device_t *device, const void *buffer, size_t length)
{
    if (!device_is_ready(device, DEVICE_CLASS_CHAR) ||
        !device->descriptor->char_ops.write) {
        return -1;
    }
    return device->descriptor->char_ops.write(device, buffer, length);
}

result_t device_char_flush(device_t *device)
{
    if (!device_is_ready(device, DEVICE_CLASS_CHAR) ||
        !device->descriptor->char_ops.flush) {
        return RESULT_ERROR;
    }
    return device->descriptor->char_ops.flush(device);
}

int device_block_read(device_t *device, void *buffer, uint64_t start_block,
                      uint32_t block_count)
{
    if (!device_is_ready(device, DEVICE_CLASS_BLOCK) ||
        !device->descriptor->block_ops.read_blocks) {
        return -1;
    }
    return device->descriptor->block_ops.read_blocks(device, buffer, start_block, block_count);
}

int device_block_write(device_t *device, const void *buffer, uint64_t start_block,
                       uint32_t block_count)
{
    if (!device_is_ready(device, DEVICE_CLASS_BLOCK) ||
        !device->descriptor->block_ops.write_blocks) {
        return -1;
    }
    return device->descriptor->block_ops.write_blocks(device, buffer, start_block, block_count);
}

result_t device_block_sync(device_t *device)
{
    if (!device_is_ready(device, DEVICE_CLASS_BLOCK) ||
        !device->descriptor->block_ops.sync) {
        return RESULT_ERROR;
    }
    return device->descriptor->block_ops.sync(device);
}

uint32_t device_block_size(device_t *device)
{
    if (!device_is_ready(device, DEVICE_CLASS_BLOCK) ||
        !device->descriptor->block_ops.get_block_size) {
        return 0;
    }
    return device->descriptor->block_ops.get_block_size(device);
}

uint64_t device_block_count(device_t *device)
{
    if (!device_is_ready(device, DEVICE_CLASS_BLOCK) ||
        !device->descriptor->block_ops.get_block_count) {
        return 0;
    }
    return device->descriptor->block_ops.get_block_count(device);
}

result_t device_display_get_mode(device_t *device, uint32_t *width, uint32_t *height,
                                 uint32_t *bpp)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.get_mode) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.get_mode(device, width, height, bpp);
}

result_t device_display_set_cursor(device_t *device, uint32_t x, uint32_t y)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.set_cursor) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.set_cursor(device, x, y);
}

result_t device_display_get_cursor(device_t *device, uint32_t *x, uint32_t *y)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.get_cursor) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.get_cursor(device, x, y);
}

result_t device_display_write_char(device_t *device, uint32_t x, uint32_t y, char c,
                                   uint8_t foreground, uint8_t background)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.write_char) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.write_char(
        device, x, y, c, foreground, background);
}

result_t device_display_clear(device_t *device, uint8_t foreground, uint8_t background)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.clear_screen) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.clear_screen(device, foreground, background);
}

result_t device_display_scroll(device_t *device, uint32_t lines)
{
    if (!device_is_ready(device, DEVICE_CLASS_DISPLAY) ||
        !device->descriptor->display_ops.scroll_up) {
        return RESULT_ERROR;
    }
    return device->descriptor->display_ops.scroll_up(device, lines);
}