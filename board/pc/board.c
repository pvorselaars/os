#include "board/board.h"

#include "board/pc/audio.h"
#include "board/pc/disk.h"
#include "board/pc/display.h"
#include "board/pc/keyboard.h"
#include "board/pc/parallel.h"
#include "board/pc/vga.h"
#include "board/pc/timer.h"
#include "board/pc/pic.h"
#include "board/pc/serial.h"


static result_t board_register_devices(void)
{
    result_t result = pc_serial_register();
    if (result != RESULT_OK) {
        return result;
    }

    result = pc_parallel_register();
    if (result != RESULT_OK) {
        return result;
    }

    result = pc_audio_register();
    if (result != RESULT_OK) {
        return result;
    }

    result = pc_keyboard_register();
    if (result != RESULT_OK) {
        return result;
    }

    result = pc_disk_register();
    if (result != RESULT_OK) {
        return result;
    }

    return pc_display_register();
}

void board_init()
{
    pc_pic_init();
    vga_init();
    pc_timer_init(100);
    board_register_devices();
}
