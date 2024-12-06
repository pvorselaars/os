#include "memory.h"

void kernel()
{
        char msg[10] = {'H', 0x1f, 'E', 0x2f, 'L', 0x3f, 'L', 0x4f, 'O', 0x5f};
        char msg2[10] = {'W', 0x1f, 'O', 0x2f, 'R', 0x3f, 'L', 0x4f, 'D', 0x5f};
        char *video = (char*)0xb8000;

        memsetw(video, 0x0f20, 80*25);

        if(memcmp(msg, msg, 10)) {
                memcpy(video, msg, 10);
        } else {
                memcpy(video, msg2, 10);
        }

        while(1);
}
