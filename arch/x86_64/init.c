#include "arch/arch.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "board/board.h"

arch_result arch_init(void)
{
    arch_result result;

    x86_64_gdt_init();

    TEST_CASE("Memory initialization") {
        result = arch_memory_init();
        TEST_ASSERT_EQUAL(ARCH_OK, result);
        if (result != ARCH_OK) {
            return result;
        }
    }

    TEST_CASE("Interrupt initialization") {
        result = arch_interrupt_init();
        TEST_ASSERT_EQUAL(ARCH_OK, result);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    TEST_CASE("Timer initialization") {
        result = arch_timer_init(100);
        TEST_ASSERT_EQUAL(ARCH_OK, result);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}