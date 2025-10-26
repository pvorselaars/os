#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

static arch_result console_open(device_t *dev);
static arch_result console_close(device_t *dev);
static int console_read(device_t *dev, void *buf, size_t len);
static int console_write(device_t *dev, const void *buf, size_t len);
static arch_result console_flush(device_t *dev);

typedef struct {
    device_t *display_device;  // Backend display device
    uint32_t width;            // Console width (from display)
    uint32_t height;           // Console height (from display)
    uint32_t cursor_x;         // Current cursor column
    uint32_t cursor_y;         // Current cursor row
    uint8_t fg_color;          // Foreground color
    uint8_t bg_color;          // Background color
} console_driver_data_t;

static device_t console_device;
static console_driver_data_t console_data;

static void console_scroll_if_needed(console_driver_data_t *data)
{
    if (data->cursor_y >= data->height) {
        if (data->display_device) {
            data->display_device->display_ops.scroll_up(data->display_device, 1);
        }
        data->cursor_y = data->height - 1;
    }
}

static void console_update_cursor(console_driver_data_t *data)
{
    if (data->display_device) {
        data->display_device->display_ops.set_cursor(data->display_device, 
                                                     data->cursor_x, data->cursor_y);
    }
}

static arch_result console_open(device_t *dev)
{
    console_driver_data_t *data = (console_driver_data_t *)dev->driver_data;
    
    data->display_device = device_find_by_name("vga0");
    if (!data->display_device) {
        return ARCH_ERROR;
    }
    
    arch_result result = data->display_device->open(data->display_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    data->display_device->display_ops.get_mode(data->display_device, 
                                               &data->width, &data->height, NULL);
    
    data->cursor_x = 0;
    data->cursor_y = 0;
    data->fg_color = 15;
    data->bg_color = 0;

    data->display_device->display_ops.clear_screen(data->display_device,
                                                   data->fg_color, data->bg_color);
    console_update_cursor(data);
    
    return ARCH_OK;
}

static arch_result console_close(device_t *dev)
{
    console_driver_data_t *data = (console_driver_data_t *)dev->driver_data;
    
    if (data->display_device) {
        data->display_device->close(data->display_device);
        data->display_device = NULL;
    }
    
    return ARCH_OK;
}

static int console_read(device_t *dev, void *buf, size_t len)
{
    return -1;
}

static int console_write(device_t *dev, const void *buf, size_t len)
{
    console_driver_data_t *data = (console_driver_data_t *)dev->driver_data;
    const char *str = (const char *)buf;
    
    if (!data->display_device || !buf) {
        return -1;
    }
    
    size_t written = 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        
        switch (c) {
        case '\n':
            data->cursor_x = 0;
            data->cursor_y++;
            console_scroll_if_needed(data);
            break;
            
        case '\r':
            data->cursor_x = 0;
            break;
            
        case '\t':
            data->cursor_x = (data->cursor_x + 8) & ~7;
            if (data->cursor_x >= data->width) {
                data->cursor_x = 0;
                data->cursor_y++;
                console_scroll_if_needed(data);
            }
            break;
            
        case '\b':
            if (data->cursor_x > 0) {
                data->cursor_x--;
            }
            break;
            
        default:
            if (c >= 32 && c < 127) {
                data->display_device->display_ops.write_char(data->display_device,
                                                             data->cursor_x, data->cursor_y,
                                                             c, data->fg_color, data->bg_color);
                data->cursor_x++;
                
                if (data->cursor_x >= data->width) {
                    data->cursor_x = 0;
                    data->cursor_y++;
                    console_scroll_if_needed(data);
                }
            }
            break;
        }
        
        written++;
    }
    
    console_update_cursor(data);
    return written;
}

static arch_result console_flush(device_t *dev)
{
    return ARCH_OK;
}

arch_result console_driver_init(void)
{
    strncpy(console_device.name, "console0", sizeof(console_device.name) - 1);
    console_device.name[sizeof(console_device.name) - 1] = '\0';
    
    console_device.class = DEVICE_CLASS_CHAR;
    console_device.state = DEVICE_STATE_UNINITIALIZED;
    console_device.open = console_open;
    console_device.close = console_close;
    console_device.char_ops.read = console_read;
    console_device.char_ops.write = console_write;
    console_device.char_ops.flush = console_flush;
    console_device.driver_data = &console_data;
    console_device.next = NULL;
    
    console_data.display_device = NULL;
    console_data.width = 0;
    console_data.height = 0;
    console_data.cursor_x = 0;
    console_data.cursor_y = 0;
    console_data.fg_color = 15;
    console_data.bg_color = 0;
    
    return device_register(&console_device);
}