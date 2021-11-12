//kernel IO behaviour
#include "schismKernelIO.h"

 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) 
{
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}
 
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
	{
		if(data[i] != '\n')
			terminal_putchar(data[i]);
		else
			terminal_handle_newline();
	}
}
 
void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void terminal_writeint(int data)
{
	//we need an array of characters.  Its an integer, which in a 32 bit OS is
	//max 10 characters.  +1 for the zero character and we're good to go
	char outString[INT_OUTPUT_SIZE] = {0}; //the way we are doing this gives LSD first, so we need to reverse it later
	char outStringReversed[INT_OUTPUT_SIZE] = {0}; //this is the actual output string
	int inputSize = 0;
	if(data < 0)
	{
		terminal_putchar('-');
		data = -data;
	}
	if(data == 0)
	{
		terminal_putchar(ZERO_CHAR);
		return;
	}
	
	while(data!= 0)
	{
		outString[inputSize] = data%10 + ZERO_CHAR;
		inputSize++;
		data /= 10;
	}
	
	//now reverse the array
	int maxSize = inputSize-1; //inputSize has been ++'d one too many times
	for(int i = inputSize-1; i >= 0; i--)
	{
		outStringReversed[maxSize-i] = outString[i];
	}
	terminal_writestring(outStringReversed);
	return;
}

//almost identical to writeInt except what is being sent in and no check for negative
void terminal_writeuint32(uint32_t data)
{
	//we need an array of characters.  Its an integer, which in a 32 bit OS is
	//max 10 characters.  +1 for the zero character and we're good to go
	char outString[INT_OUTPUT_SIZE] = {0}; //the way we are doing this gives LSD first, so we need to reverse it later
	char outStringReversed[INT_OUTPUT_SIZE] = {0}; //this is the actual output string
	int inputSize = 0;
	if(data == 0)
	{
		terminal_putchar(ZERO_CHAR);
		return;
	}
	
	while(data!= 0)
	{
		outString[inputSize] = data%10 + ZERO_CHAR;
		inputSize++;
		data /= 10;
	}
	
	//now reverse the array
	int maxSize = inputSize-1; //inputSize has been ++'d one too many times
	for(int i = inputSize-1; i >= 0; i--)
	{
		outStringReversed[maxSize-i] = outString[i];
	}
	terminal_writestring(outStringReversed);
	return;
}

//a VERY basic printf for debug and basic talk with the komputermachin
void kernel_printf(const char* data,...)
{
	//create the variable argument list
	va_list vargin;
	
	//start it
	va_start(vargin,data);
	
	int i = 0;
	while(data[i])
	{
		if(data[i] != '%' && data[i] != '\n' && data[i] != '\t')
			terminal_putchar(data[i]);
		else
		{
			//handle the backslash
			if(data[i] == '\n')
			{
				terminal_handle_newline();
			}
			else if(data[i] == '\t')
			{
				for(int i = 0; i < TAB_WIDTH; i++)
					terminal_putchar(' ');
			}
			else //this is a format string
			{
				//this is a SUPER simple printf = it's only integers for now
				//first, increment i
				i++;
				//now check it = later we'll handle different things than int
				if(data[i] == 'd')
				{
					int nextOut = va_arg(vargin,int);
					terminal_writeint(nextOut);
				}
				else if(data[i] == 'u')
				{
					uint32_t nextOut = va_arg(vargin,uint32_t);
					terminal_writeuint32(nextOut);
				}
			}
		}
		i++;
	}
	
	//release the arguments
	va_end(vargin);
	return;

}

//In general it just increments the row counter, but if we are at the maximum height
//this will move the entire buffer up by 1 and blank out the last line
void terminal_handle_newline()
{
	terminal_column = 0;
	//reset the terminal after advancing the row
	if(++terminal_row == VGA_HEIGHT)
	{
		terminal_row--;
		//now copy everything
		for (size_t y = 1; y < VGA_HEIGHT; y++) {
			for (size_t x = 0; x < VGA_WIDTH; x++) {
				size_t index = y * VGA_WIDTH + x;
				size_t prevRowIndx = (y-1)*VGA_WIDTH + x;
				terminal_buffer[prevRowIndx] = terminal_buffer[index];
			}
		}
		//blank out the last line
		for(size_t i = 0; i < VGA_WIDTH; i++)
		{
			size_t index = (VGA_HEIGHT-1)*VGA_WIDTH + i;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
