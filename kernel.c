#include "memory.h"

void console_init();
void print(const char *str);

static char *vga = (char*)0xb8000;

void kernel()
{
        console_init();
        print("Hello,\nworld!");

        while(1);
}

void console_init()
{
        memsetw(vga, 0x0f20, 4000);
}

void print(const char *str)
{
        static int cursor = 0;

        while (*str) {
                if (*str == '\n') {
                        cursor = (cursor / 160 + 1) * 160;
                } else {
                        vga[cursor] = *str;
                        cursor += 2;
                }

                // scroll screen up
                if (cursor > 4000) {
                        memcpy(vga, vga+160, 3840);
                        memsetw(vga+3840, 0x1f20, 160);
                        cursor = 3840;
                }

                str++;
        }

}
