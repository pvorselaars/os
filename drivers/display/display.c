#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

static arch_result display_open(device_t *dev);
static arch_result display_close(device_t *dev);
static arch_result display_get_mode(device_t *dev, uint32_t *width, uint32_t *height, uint32_t *bpp);
static arch_result display_set_cursor(device_t *dev, uint32_t x, uint32_t y);
static arch_result display_get_cursor(device_t *dev, uint32_t *x, uint32_t *y);
static arch_result display_write_char(device_t *dev, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg);
static arch_result display_clear_screen(device_t *dev, uint8_t fg, uint8_t bg);
static arch_result display_scroll_up(device_t *dev, uint32_t lines);

typedef struct {
    arch_display_device_t *arch_device;  // Opaque arch-specific device handle
    uint32_t width;                       // Display width in characters/pixels
    uint32_t height;                      // Display height in characters/pixels
    uint32_t bpp;                         // Bits per pixel (0 for text mode)
    bool text_mode;                       // Text mode vs graphics mode
} display_driver_data_t;

static device_t *display_devices = NULL;
static display_driver_data_t *display_data = NULL;
static int display_device_count = 0;

static arch_result display_open(device_t *dev)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    
    arch_result result = arch_display_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    return ARCH_OK;
}

static arch_result display_close(device_t *dev)
{
    return ARCH_OK;
}

static arch_result display_get_mode(device_t *dev, uint32_t *width, uint32_t *height, uint32_t *bpp)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    
    if (width) *width = data->width;
    if (height) *height = data->height;
    if (bpp) *bpp = data->bpp;
    
    return ARCH_OK;
}

static arch_result display_set_cursor(device_t *dev, uint32_t x, uint32_t y)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    
    if (x >= data->width || y >= data->height) {
        return ARCH_ERROR;
    }
    
    return arch_display_set_cursor(data->arch_device, x, y);
}

static arch_result display_get_cursor(device_t *dev, uint32_t *x, uint32_t *y)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    return arch_display_get_cursor(data->arch_device, x, y);
}

static arch_result display_write_char(device_t *dev, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    
    if (x >= data->width || y >= data->height) {
        return ARCH_ERROR;
    }
    
    return arch_display_write_char(data->arch_device, x, y, c, fg, bg);
}

static arch_result display_clear_screen(device_t *dev, uint8_t fg, uint8_t bg)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    return arch_display_clear_screen(data->arch_device, fg, bg);
}

static arch_result display_scroll_up(device_t *dev, uint32_t lines)
{
    display_driver_data_t *data = (display_driver_data_t *)dev->driver_data;
    
    if (lines == 0 || lines >= data->height) {
        return ARCH_ERROR;
    }
    
    return arch_display_scroll_up(data->arch_device, lines);
}

arch_result display_driver_init(void)
{
    display_device_count = arch_display_get_count();
    
    if (display_device_count == 0) {
        return ARCH_OK;
    }
    
    #define MAX_DISPLAY_DEVICES 4
    static device_t static_devices[MAX_DISPLAY_DEVICES];
    static display_driver_data_t static_data[MAX_DISPLAY_DEVICES];
    
    if (display_device_count > MAX_DISPLAY_DEVICES) {
        display_device_count = MAX_DISPLAY_DEVICES;
    }
    
    display_devices = static_devices;
    display_data = static_data;
    
    for (int i = 0; i < display_device_count; i++) {
        arch_display_info_t info;
        arch_result result = arch_display_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        device_t *device = &display_devices[i];
        display_driver_data_t *data = &display_data[i];
        
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_DISPLAY;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = display_open;
        device->close = display_close;
        device->display_ops.get_mode = display_get_mode;
        device->display_ops.set_cursor = display_set_cursor;
        device->display_ops.get_cursor = display_get_cursor;
        device->display_ops.write_char = display_write_char;
        device->display_ops.clear_screen = display_clear_screen;
        device->display_ops.scroll_up = display_scroll_up;
        device->driver_data = data;
        device->next = NULL;
        
        data->arch_device = info.device;
        data->width = info.width;
        data->height = info.height;
        data->bpp = info.bpp;
        data->text_mode = info.text_mode;
        
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}