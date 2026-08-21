#include "arch/x86_64/io.h"
#include "arch/x86_64/memory.h"
#include "board/pc/display.h"
#include "kernel/device.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t *)virtual_address(0xB8000))

#define VGA_CURSOR_CMD  0x3D4
#define VGA_CURSOR_DATA 0x3D5

typedef struct {
    bool initialized;
    uint32_t cursor_x;
    uint32_t cursor_y;
} vga_display_t;

static vga_display_t vga_display;

static uint8_t vga_make_color(uint8_t foreground, uint8_t background)
{
    return foreground | (background << 4);
}

static uint16_t vga_make_entry(char character, uint8_t color)
{
    return (uint16_t)character | ((uint16_t)color << 8);
}

static void vga_update_hardware_cursor(uint32_t x, uint32_t y)
{
    uint16_t position = y * VGA_WIDTH + x;

    outb(VGA_CURSOR_CMD, 0x0F);
    outb(VGA_CURSOR_DATA, (uint8_t)position);
    outb(VGA_CURSOR_CMD, 0x0E);
    outb(VGA_CURSOR_DATA, (uint8_t)(position >> 8));
}

static result_t vga_display_init(device_t *device)
{
    vga_display_t *display = device->data;
    uint16_t blank = vga_make_entry(' ', vga_make_color(15, 0));

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = blank;
    }

    display->initialized = true;
    display->cursor_x = 0;
    display->cursor_y = 0;
    vga_update_hardware_cursor(0, 0);
    return RESULT_OK;
}

static result_t vga_display_get_mode(
    device_t *device,
    uint32_t *width,
    uint32_t *height,
    uint32_t *bpp
) {
    (void)device;

    if (width) {
        *width = VGA_WIDTH;
    }
    if (height) {
        *height = VGA_HEIGHT;
    }
    if (bpp) {
        *bpp = 0;
    }

    return RESULT_OK;
}

static result_t vga_display_set_cursor(device_t *device, uint32_t x, uint32_t y)
{
    vga_display_t *display = device->data;

    if (!display->initialized || x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return RESULT_ERROR;
    }

    display->cursor_x = x;
    display->cursor_y = y;
    vga_update_hardware_cursor(x, y);
    return RESULT_OK;
}

static result_t vga_display_get_cursor(device_t *device, uint32_t *x, uint32_t *y)
{
    vga_display_t *display = device->data;

    if (!display->initialized) {
        return RESULT_ERROR;
    }
    if (x) {
        *x = display->cursor_x;
    }
    if (y) {
        *y = display->cursor_y;
    }

    return RESULT_OK;
}

static result_t vga_display_write_char(
    device_t *device,
    uint32_t x,
    uint32_t y,
    char character,
    uint8_t foreground,
    uint8_t background
) {
    vga_display_t *display = device->data;

    if (!display->initialized || x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return RESULT_ERROR;
    }

    VGA_MEMORY[y * VGA_WIDTH + x] = vga_make_entry(
        character,
        vga_make_color(foreground & 0x0F, background & 0x0F)
    );
    return RESULT_OK;
}

static result_t vga_display_clear_screen(
    device_t *device,
    uint8_t foreground,
    uint8_t background
) {
    vga_display_t *display = device->data;

    if (!display->initialized) {
        return RESULT_ERROR;
    }

    uint16_t blank = vga_make_entry(
        ' ',
        vga_make_color(foreground & 0x0F, background & 0x0F)
    );

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = blank;
    }

    display->cursor_x = 0;
    display->cursor_y = 0;
    vga_update_hardware_cursor(0, 0);
    return RESULT_OK;
}

static result_t vga_display_scroll_up(device_t *device, uint32_t lines)
{
    vga_display_t *display = device->data;

    if (!display->initialized || lines == 0 || lines >= VGA_HEIGHT) {
        return RESULT_ERROR;
    }

    for (uint32_t row = 0; row < VGA_HEIGHT - lines; row++) {
        for (uint32_t column = 0; column < VGA_WIDTH; column++) {
            VGA_MEMORY[row * VGA_WIDTH + column] =
                VGA_MEMORY[(row + lines) * VGA_WIDTH + column];
        }
    }

    uint16_t blank = vga_make_entry(' ', vga_make_color(15, 0));
    for (uint32_t row = VGA_HEIGHT - lines; row < VGA_HEIGHT; row++) {
        for (uint32_t column = 0; column < VGA_WIDTH; column++) {
            VGA_MEMORY[row * VGA_WIDTH + column] = blank;
        }
    }

    display->cursor_y = display->cursor_y >= lines
        ? display->cursor_y - lines
        : 0;
    vga_update_hardware_cursor(display->cursor_x, display->cursor_y);
    return RESULT_OK;
}

static device_t vga_device = {
    .name = "vga0",
    .class = DEVICE_CLASS_DISPLAY,
    .init = vga_display_init,
    .display_ops = {
        .get_mode = vga_display_get_mode,
        .set_cursor = vga_display_set_cursor,
        .get_cursor = vga_display_get_cursor,
        .write_char = vga_display_write_char,
        .clear_screen = vga_display_clear_screen,
        .scroll_up = vga_display_scroll_up,
    },
    .data = &vga_display,
};

result_t pc_display_register(void)
{
    return device_register(&vga_device);
}
