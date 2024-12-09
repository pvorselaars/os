#include "console.h"

void kernel()
{
        console_init();
        print("Hello,\nworld!");
        while(1);
}

