#ifndef DEVICE_H
#define DEVICE_H

#include "kernel/base.h"

typedef enum {
    DEVICE_CLASS_CHAR,
    DEVICE_CLASS_BLOCK,
    DEVICE_CLASS_DISPLAY,
    DEVICE_CLASS_MAX
} device_class_t;

typedef enum {
    DEVICE_STATE_UNINITIALIZED = 0,
    DEVICE_STATE_INITIALIZING,
    DEVICE_STATE_READY,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_REMOVED
} device_state_t;

typedef struct device device_t;

typedef struct {
    int (*read)(device_t *dev, void *buf, size_t len);
    int (*write)(device_t *dev, const void *buf, size_t len);
    result_t (*flush)(device_t *dev);
} char_device_ops_t;

typedef struct {
    int (*read_blocks)(device_t *dev, void *buf, uint64_t start_block, uint32_t block_count);
    int (*write_blocks)(device_t *dev, const void *buf, uint64_t start_block, uint32_t block_count);
    result_t (*sync)(device_t *dev);
    uint32_t (*get_block_size)(device_t *dev);
    uint64_t (*get_block_count)(device_t *dev);
} block_device_ops_t;

typedef struct {
    result_t (*get_mode)(device_t *dev, uint32_t *width, uint32_t *height, uint32_t *bpp);
    result_t (*set_cursor)(device_t *dev, uint32_t x, uint32_t y);
    result_t (*get_cursor)(device_t *dev, uint32_t *x, uint32_t *y);
    result_t (*write_char)(device_t *dev, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg);
    result_t (*clear_screen)(device_t *dev, uint8_t fg, uint8_t bg);
    result_t (*scroll_up)(device_t *dev, uint32_t lines);
} display_device_ops_t;

typedef struct {
    const char *name;
    device_class_t class;
    result_t (*init)(device_t *device);

    union {
        char_device_ops_t char_ops;
        block_device_ops_t block_ops;
        display_device_ops_t display_ops;
    };

    void *data;
} device_descriptor_t;

result_t device_register(const device_descriptor_t *descriptor);
device_t *device_find_by_name(const char *name);
device_t *device_find_by_class(device_class_t class, uint32_t index);

const char *device_name(const device_t *device);
device_class_t device_class(const device_t *device);
device_state_t device_state(const device_t *device);
void *device_data(device_t *device);

int device_char_read(device_t *device, void *buffer, size_t length);
int device_char_write(device_t *device, const void *buffer, size_t length);
result_t device_char_flush(device_t *device);

int device_block_read(device_t *device, void *buffer, uint64_t start_block,
                      uint32_t block_count);
int device_block_write(device_t *device, const void *buffer, uint64_t start_block,
                       uint32_t block_count);
result_t device_block_sync(device_t *device);
uint32_t device_block_size(device_t *device);
uint64_t device_block_count(device_t *device);

result_t device_display_get_mode(device_t *device, uint32_t *width, uint32_t *height,
                                 uint32_t *bpp);
result_t device_display_set_cursor(device_t *device, uint32_t x, uint32_t y);
result_t device_display_get_cursor(device_t *device, uint32_t *x, uint32_t *y);
result_t device_display_write_char(device_t *device, uint32_t x, uint32_t y, char c,
                                   uint8_t foreground, uint8_t background);
result_t device_display_clear(device_t *device, uint8_t foreground, uint8_t background);
result_t device_display_scroll(device_t *device, uint32_t lines);

#endif