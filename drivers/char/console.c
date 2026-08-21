#include "kernel/device.h"
#include "arch/arch.h"

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

static int console_read(device_t *dev, void *buf, size_t len)
{
    return -1;
}

static int console_write(device_t *dev, const void *buf, size_t len)
{
    console_driver_data_t *data = dev->data;
    const char *str = buf;
    
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

static result_t console_flush(device_t *dev)
{
    return RESULT_OK;
}

static result_t console_init(device_t *device)
{
    console_driver_data_t *data = device->data;

    data->display_device = device_find_by_class(DEVICE_CLASS_DISPLAY, 0);
    if (!data->display_device) {
        return RESULT_ERROR;
    }

    data->display_device->display_ops.get_mode(
        data->display_device, &data->width, &data->height, NULL
    );

    data->cursor_x = 0;
    data->cursor_y = 0;
    data->fg_color = 15;
    data->bg_color = 0;

    return data->display_device->display_ops.clear_screen(
        data->display_device, data->fg_color, data->bg_color
    );
}

static device_t console_device = {
    .name = "console0",
    .class = DEVICE_CLASS_CHAR,
    .init = console_init,
    .char_ops = {
        .read = console_read,
        .write = console_write,
        .flush = console_flush,
    },
    .data = &console_data,
};

result_t console_register()
{
    return device_register(&console_device);
}