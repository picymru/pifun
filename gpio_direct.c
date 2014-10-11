#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "gpio.h"

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

static void *gpio_map;

// I/O access
volatile unsigned *gpio;


int gpio_open(void)
{
	int mem_fd;

  	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
	{
		printf("Can't open /dev/mem\n");
		return -1;
	}

	/* mmap GPIO */
	gpio_map = mmap(
		NULL,             //Any adddress in our space will do
		BLOCK_SIZE,       //Map length
		PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
		MAP_SHARED,       //Shared with other processes
		mem_fd,           //File to map
		GPIO_BASE         //Offset to GPIO peripheral
	);

	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED)
	{
		printf("mmap error %d\n", (int)gpio_map);//errno also set!
		return -1;
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;

	return 0;
}

int gpio_close(void)
{
	return 0;
}

int gpio_export(int pin)
{
	return 0;
}

int gpio_unexport(int pin)
{
	return 0;
}

int gpio_direction(int pin, int dir)
{
	INP_GPIO(pin);

	if (OUT == dir)
	{
		OUT_GPIO(pin);
	}

	return 0;
}

inline int gpio_read(int pin)
{
	if (GET_GPIO(pin))
	{
		return 1;
	}
	
	return 0;
}

inline void gpio_write(int pin, int value)
{
	if (value)
	{
		GPIO_SET = 1 << pin;
	}
	else
	{
		GPIO_CLR = 1 << pin;
	}
}
