#include "board/board.h"
#include "board/pc/serial.h"
#include "board/pc/vga.h"

arch_result board_init(void)
{
    vga_init();
    
    return ARCH_OK;
}