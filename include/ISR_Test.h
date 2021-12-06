#ifndef ISR_TEST
#define ISR_TEST
#include "schismKernelIO.h"
#include "schism_PIC.h"


struct interrupt_frame {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} __attribute__((packed));
 
void keyboard_event();
//void generic_event();
//__attribute__((interrupt)) void test_interrupt_handler(struct interrupt_frame* frame);
//void test_interrupt_handler();
#endif
