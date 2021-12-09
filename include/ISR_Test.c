#include "ISR_Test.h"
extern int test;
//__attribute__((interrupt)) void test_interrupt_handler(struct interrupt_frame* frame)
//void test_interrupt_handler(struct interrupt_frame* frame)
//void test_interrupt_handler()
//__attribute__((interrupt)) void test_interrupt_handler(struct interrupt_frame* frame)

void keyboard_event()
{
//	kernel_printf("A keypress was detected!!\n");
	test++;
	//terminal_setcolor(VGA_COLOR_WHITE);
	//PIC_sendEOI(1);
}

/*
void generic_event()
{
	kernel_printf("Generic interrupt\n");
}*/