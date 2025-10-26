#ifndef X86_64_PIC_H
#define X86_64_PIC_H

#include "definitions.h"

void x86_64_pic_remap(void);
void x86_64_pic_eoi(unsigned int irq);
void x86_64_pic_mask_irq(unsigned int irq);
void x86_64_pic_unmask_irq(unsigned int irq);

#endif