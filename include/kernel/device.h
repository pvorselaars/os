#ifndef DEVICE_H
#define DEVICE_H

#include "definitions.h"
#include "arch/arch.h"

typedef enum {
    DEVICE_CLASS_CHAR = 0,
    DEVICE_CLASS_BLOCK,
    DEVICE_CLASS_MAX
} device_class_t;

typedef enum {
    DEVICE_STATE_UNINITIALIZED = 0,
    DEVICE_STATE_INITIALIZING,
    DEVICE_STATE_READY,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_REMOVED
} device_state_t;

struct device;

/* Character device operations */
typedef struct {
    int (*read)(struct device *dev, void *buf, size_t len);
    int (*write)(struct device *dev, const void *buf, size_t len);
    arch_result (*flush)(struct device *dev);
} char_device_ops_t;

/* Block device operations */
typedef struct {
    int (*read_blocks)(struct device *dev, void *buf, uint64_t start_block, uint32_t block_count);
    int (*write_blocks)(struct device *dev, const void *buf, uint64_t start_block, uint32_t block_count);
    arch_result (*sync)(struct device *dev);
    uint32_t (*get_block_size)(struct device *dev);
    uint64_t (*get_block_count)(struct device *dev);
} block_device_ops_t;


/* Display device operations 
typedef struct {
    arch_result (*set_mode)(struct device *dev, uint32_t width, uint32_t height, uint32_t bpp);
    arch_result (*get_mode)(struct device *dev, uint32_t *width, uint32_t *height, uint32_t *bpp);
    arch_result (*map_framebuffer)(struct device *dev, void **fb_addr, size_t *fb_size);
    arch_result (*write_pixel)(struct device *dev, uint32_t x, uint32_t y, uint32_t color);
    arch_result (*write_char)(struct device *dev, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg);
    arch_result (*blit)(struct device *dev, const void *src, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    arch_result (*clear_screen)(struct device *dev, uint32_t color);
    arch_result (*scroll)(struct device *dev, int lines);
} display_device_ops_t;
 */

typedef struct device {
    char name[32];              // Device name (e.g., "serial0", "vga0")
    device_class_t class;       // Device class
    device_state_t state;       // Current device state
    
    arch_result (*open)(struct device *dev);
    arch_result (*close)(struct device *dev);
    
    union {
        char_device_ops_t char_ops;
        block_device_ops_t block_ops;
    };
    
    void *driver_data;          // Private driver state
    struct device *next;        // Next device in global list
} device_t;

/* Device management API */
arch_result device_init(void);
arch_result device_register(device_t *device);
arch_result device_unregister(device_t *device);
device_t* device_find_by_name(const char *name);
device_t* device_find_by_class(device_class_t class, uint32_t index);
void device_list_all(void);

const char* device_class_name(device_class_t class);
const char* device_state_name(device_state_t state);

#endif