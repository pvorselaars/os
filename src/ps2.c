#include "ps2.h"

static const char scancode_table[128] = {
    // 0x00-0x0F
    0,    27,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    // 0x10-0x1F  
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
    // 0x20-0x2F
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    // 0x30-0x3F  
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',  0,    0,    0,    0,    0,    0,
    // 0x40-0x4F
    0,    0,    0,    0,    0,    0,    0,    '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    // 0x50-0x5F
    '2',  '3',  '0',  '.',  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    // 0x60-0x6F
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    // 0x70-0x7F
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

void ps2_keyboard_interrupt() {
    // Read scan code from data port
    uint8_t scan_code = inb(PS2_DATA_PORT);

    if (scan_code & 0x80) {
    } else {
        uint8_t key_pressed = scan_code;
        char c = scancode_table[key_pressed];
        put(c);
    }
}

void ps2_init() {

    // Disable devices
    outb(PS2_COMMAND_PORT, 0xAD); // Disable first PS/2 port (keyboard)
    outb(PS2_COMMAND_PORT, 0xA7); // Disable second PS/2 port (mouse)

    // Flush output buffer
    while (inb(PS2_STATUS_PORT) & 0x01) {
        inb(PS2_DATA_PORT);
    }

    // Set controller configuration byte
    outb(PS2_COMMAND_PORT, 0x20);
    uint8_t config = inb(PS2_DATA_PORT);
    outb(PS2_COMMAND_PORT, 0x60);
    outb(PS2_DATA_PORT, 0b01010101);

    // Determine if second PS/2 port is present
    outb(PS2_COMMAND_PORT, 0xA8);
    outb(PS2_COMMAND_PORT, 0x20);
    config = inb(PS2_DATA_PORT);
    bool second_port_present = config & 0x20;
    if (second_port_present) {
        outb(PS2_COMMAND_PORT, 0xA7);
    }

    // Perform controller self-test
    outb(PS2_COMMAND_PORT, 0xAA);
    uint8_t self_test_result = inb(PS2_DATA_PORT);
    if (self_test_result != 0x55) {
        // Self-test failed, TODO: handle error
    }

    if (second_port_present) {
        // Test second PS/2 port
        outb(PS2_COMMAND_PORT, 0xA9);
        uint8_t port_test_result = inb(PS2_DATA_PORT);
        if (port_test_result != 0x00) {
            // Second port test failed, TODO: handle error
        }
    }

    // Enable devices
    outb(PS2_COMMAND_PORT, 0xAE);
    if (second_port_present) {
        outb(PS2_COMMAND_PORT, 0xA8);
    }

    // Enable interrupts for devices
    outb(PS2_COMMAND_PORT, 0x20);
    config = inb(PS2_DATA_PORT);
    config |= 0x01;
    if (second_port_present) {
        config |= 0x02;
    }
    outb(PS2_COMMAND_PORT, 0x60);
    outb(PS2_DATA_PORT, config);    

    // Reset devices
    outb(PS2_DATA_PORT, 0xFF);
    if (second_port_present) {
        outb(PS2_COMMAND_PORT, 0xD4);
        outb(PS2_DATA_PORT, 0xFF); 
    }

}
