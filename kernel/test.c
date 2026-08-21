#include "kernel/test.h"

#include "drivers/console.h"
#include "kernel/device.h"
#include "kernel/time.h"
#include "lib/utils.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            debug_printf("Test failed: %s\n", #condition); \
            return RESULT_ERROR; \
        } \
    } while (0)

static device_t *require_device(const char *name, device_class_t class) {
    device_t *device = device_find_by_name(name);

    if (!device || device_class(device) != class) {
        return NULL;
    }

    return device;
}

result_t test_run(void) {
    debug_printf("Running tests...\n");

    debug_printf("Timer test\n");
    uint64_t start = time_now_ns();
    sleep(20);
    CHECK(time_now_ns() > start);

    device_t *serial = require_device("serial0", DEVICE_CLASS_CHAR);
    device_t *parallel = require_device("parallel0", DEVICE_CLASS_CHAR);
    device_t *speaker = require_device("pcspk0", DEVICE_CLASS_CHAR);
    device_t *keyboard = require_device("kb0", DEVICE_CLASS_CHAR);
    device_t *disk = require_device("ata0", DEVICE_CLASS_BLOCK);
    device_t *display = require_device("vga0", DEVICE_CLASS_DISPLAY);
    device_t *console = require_device("console0", DEVICE_CLASS_CHAR);

    CHECK(serial);
    CHECK(parallel);
    CHECK(speaker);
    CHECK(keyboard);
    CHECK(disk);
    CHECK(display);
    CHECK(console);
    CHECK(device_state(console) == DEVICE_STATE_READY);
    CHECK(device_char_write(disk, NULL, 0) == -1);

    static const char console_message[] = "Console test\n";
    CHECK(device_char_write(
        console, console_message, sizeof(console_message) - 1
    ) == sizeof(console_message) - 1);

    static const char serial_message[] = "Serial test\n";
    CHECK(device_char_write(
        serial, serial_message, sizeof(serial_message) - 1
    ) == sizeof(serial_message) - 1);

    debug_printf("Parallel test\n");
    static const char parallel_message[] = "Parallel test\n";
    CHECK(device_char_write(parallel,
        parallel_message, sizeof(parallel_message) - 1
    ) == sizeof(parallel_message) - 1);

    debug_printf("Audio test\n");
    uint8_t tone[] = {0xB8, 0x01, 0x00, 0x00}; // 440 Hz, little-endian
    uint8_t silence[] = {0, 0, 0, 0};
    CHECK(device_char_write(speaker, tone, sizeof(tone)) == sizeof(tone));
    sleep(50);
    CHECK(device_char_write(speaker, silence, sizeof(silence)) == sizeof(silence));

    debug_printf("Disk test\n");
    static uint8_t sector[512];
    CHECK(device_block_read(disk, sector, 0, 1) == 1);


    debug_printf("Tests passed\n");
    return RESULT_OK;
}
