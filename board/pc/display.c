#include "arch/arch.h"
#include "arch/x86_64/io.h"
#include "arch/x86_64/memory.h"

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  ((uint16_t*)virtual_address(0xB8000))

#define VGA_CURSOR_CMD  0x3D4
#define VGA_CURSOR_DATA 0x3D5

#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

typedef struct {
    bool initialized;
    uint32_t cursor_x;
    uint32_t cursor_y;
} vga_display_device_t;

static vga_display_device_t vga_device = {0};

static uint8_t vga_make_color(uint8_t fg, uint8_t bg)
{
    return fg | (bg << 4);
}

static uint16_t vga_make_entry(char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void vga_update_hardware_cursor(uint32_t x, uint32_t y)
{
    uint16_t pos = y * VGA_WIDTH + x;
    
    outb(VGA_CURSOR_CMD, 0x0F);
    outb(VGA_CURSOR_DATA, pos & 0xFF);
    outb(VGA_CURSOR_CMD, 0x0E);
    outb(VGA_CURSOR_DATA, (pos >> 8) & 0xFF);
}

int arch_display_get_count(void)
{
    return 1;
}

arch_result arch_display_get_info(int index, arch_display_info_t *info)
{
    if (index != 0 || !info) {
        return ARCH_ERROR;
    }
    
    info->device = (arch_display_device_t *)&vga_device;
    info->name = "vga0";
    info->width = VGA_WIDTH;
    info->height = VGA_HEIGHT;
    info->bpp = 0;  // Text mode
    info->text_mode = true;
    
    return ARCH_OK;
}

arch_result arch_display_init(arch_display_device_t *device)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device) {
        return ARCH_ERROR;
    }
    
    vga->initialized = true;
    vga->cursor_x = 0;
    vga->cursor_y = 0;
    
    uint16_t *vga_mem = VGA_MEMORY;
    
    uint16_t blank = vga_make_entry(' ', vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_mem[i] = blank;
    }
    
    return ARCH_OK;
}

arch_result arch_display_set_cursor(arch_display_device_t *device, uint32_t x, uint32_t y)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device || !vga->initialized) {
        return ARCH_ERROR;
    }
    
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return ARCH_ERROR;
    }
    
    vga->cursor_x = x;
    vga->cursor_y = y;
    vga_update_hardware_cursor(x, y);
    
    return ARCH_OK;
}

arch_result arch_display_get_cursor(arch_display_device_t *device, uint32_t *x, uint32_t *y)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device || !vga->initialized) {
        return ARCH_ERROR;
    }
    
    if (x) *x = vga->cursor_x;
    if (y) *y = vga->cursor_y;
    
    return ARCH_OK;
}

arch_result arch_display_write_char(arch_display_device_t *device, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device || !vga->initialized) {
        return ARCH_ERROR;
    }
    
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return ARCH_ERROR;
    }
    
    uint16_t *vga_mem = VGA_MEMORY;
    uint8_t color = vga_make_color(fg & 0x0F, bg & 0x0F);  // Limit to 4 bits each
    vga_mem[y * VGA_WIDTH + x] = vga_make_entry(c, color);
    
    return ARCH_OK;
}

arch_result arch_display_clear_screen(arch_display_device_t *device, uint8_t fg, uint8_t bg)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device || !vga->initialized) {
        return ARCH_ERROR;
    }
    
    uint16_t *vga_mem = VGA_MEMORY;
    uint8_t color = vga_make_color(fg & 0x0F, bg & 0x0F);
    uint16_t blank = vga_make_entry(' ', color);
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_mem[i] = blank;
    }
    
    // Reset cursor to top-left
    vga->cursor_x = 0;
    vga->cursor_y = 0;
    vga_update_hardware_cursor(0, 0);
    
    return ARCH_OK;
}

arch_result arch_display_scroll_up(arch_display_device_t *device, uint32_t lines)
{
    vga_display_device_t *vga = (vga_display_device_t *)device;
    
    if (vga != &vga_device || !vga->initialized) {
        return ARCH_ERROR;
    }
    
    if (lines == 0 || lines >= VGA_HEIGHT) {
        return ARCH_ERROR;
    }
    
    uint16_t *vga_mem = VGA_MEMORY;
    
    // Move all lines up
    for (uint32_t row = 0; row < VGA_HEIGHT - lines; row++) {
        for (uint32_t col = 0; col < VGA_WIDTH; col++) {
            vga_mem[row * VGA_WIDTH + col] = vga_mem[(row + lines) * VGA_WIDTH + col];
        }
    }
    
    // Clear the bottom lines
    uint16_t blank = vga_make_entry(' ', vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    for (uint32_t row = VGA_HEIGHT - lines; row < VGA_HEIGHT; row++) {
        for (uint32_t col = 0; col < VGA_WIDTH; col++) {
            vga_mem[row * VGA_WIDTH + col] = blank;
        }
    }
    
    // Adjust cursor position if needed
    if (vga->cursor_y >= lines) {
        vga->cursor_y -= lines;
    } else {
        vga->cursor_y = 0;
    }
    
    vga_update_hardware_cursor(vga->cursor_x, vga->cursor_y);
    
    return ARCH_OK;
}