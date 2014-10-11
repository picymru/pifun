/* blink.c
 *
 * Raspberry Pi GPIO example using sysfs interface.
 * Guillermo A. Amaral B. <g@maral.me>
 *
 * This file blinks GPIO 4 (P1-07) while reading GPIO 24 (P1_18).
 */

 /*
 	RPi Model B GPIOs (Rev 1 / Rev 2):
 	1 - 0 (SDA) / 2 (Rev 2)
 	2 - 1 (SCL) / 3 (Rev 2)
 	3 - 4
 	4 - 7 (CE1)
 	5 - 8 (CE0)
 	6 - 9 (MISO)
 	7 - 10 (MOSI)
 	8 - 11 (SCLK)
 	9 - 14 (TXD)
 	10 - 15 (RXD)
 	11 - 17
 	12 - 18 (PCM_CLK)
 	13 - 21 / 27 (Rev 2)
 	14 - 22
 	15 - 23
 	16 - 24
 	17 - 25
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <memory.h>

#include "gpio.h"
#include "input.h"

//#define KEY_DOWN_THRESHOLD 9
//#define KEY_DOWN_THRESHOLD 4
#define KEY_DOWN_THRESHOLD 12

//#define KEY_UP_THRESHOLD 18
#define KEY_UP_THRESHOLD 16

#define MOVING_AVG_DATA_POINTS 24

#define GPIO_PIN_COUNT 6

static int gpioPins[GPIO_PIN_COUNT] = { 4, 17, 18, 21, 22, 23 };
static int gpioKeys[GPIO_PIN_COUNT] = { KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_SPACE, KEY_ENTER };

typedef struct
{
	uint32_t moving_sum_data[MOVING_AVG_DATA_POINTS];
	uint32_t moving_sum_index;
	uint32_t moving_sum;
	
	uint32_t key_state_prev;
	uint32_t key_down;
} inputDataStruct;

inputDataStruct inputData[GPIO_PIN_COUNT];

int
main(int argc, char *argv[])
{

	uint32_t sum_index;
	uint32_t gpio_value;
	uint32_t old_gpio_value;

	uint32_t pin_index = 0;

	uint32_t lowest = 0xFFFF;
	uint32_t highest = 0;

	printf("PiFun Driver v0.1 - (c)2014 Douglas Gore / PiCymru\n");

	memset(&inputData, 0, sizeof(inputData));

	if (-1 == input_open())
	{
		fprintf(stderr, "Failed to open input device\n");
		return 0;
	}

	if (-1 == gpio_open())
	{
		fprintf(stderr, "Failed to open access to GPIO\n");
		return 0;
	}

	for (pin_index = 0; pin_index < GPIO_PIN_COUNT; pin_index++)
	{
		/*
		 * Enable GPIO pins
		 */
		if (-1 == gpio_export(gpioPins[pin_index]))
		{
			fprintf(stderr, "Failed to export GPIO\n");
			return 0;
		}

		/*
		 * Set GPIO directions
		 */
		if (-1 == gpio_direction(gpioPins[pin_index], IN))
		{
			fprintf(stderr, "Failed to set GPIO direction\n");
			return 0;
		}

		inputData[pin_index].key_down = 1;
	}

	while(1) {
		/*
		 * Read GPIO value
		 */

		for (pin_index = 0; pin_index < GPIO_PIN_COUNT; pin_index++)
		{
			inputDataStruct* inDat = &inputData[pin_index];

			gpio_value = gpio_read(gpioPins[pin_index]);

			old_gpio_value = inDat->moving_sum_data[inDat->moving_sum_index];

			// Store GPIO value
	        inDat->moving_sum_data[inDat->moving_sum_index++] = gpio_value;

	        if (inDat->moving_sum_index == MOVING_AVG_DATA_POINTS)
	        {
            	inDat->moving_sum_index = 0;
            }

	        //printf("Moving sum index = %d\n", inDat->moving_sum_index);

	        if (old_gpio_value)
	        {
	        	inDat->moving_sum--;
	        }

	        if (gpio_value)
	        {
	        	inDat->moving_sum++;
	        }

	        /*
	        inDat->moving_sum = 0;

	        // Calculate the sum of the current data set
	        for (sum_index = 0; sum_index < MOVING_AVG_DATA_POINTS; sum_index++)
	        {
	            inDat->moving_sum += inDat->moving_sum_data[sum_index];
	        }
	        */

	        // If we are below the key down theshold
	        if (inDat->moving_sum <= KEY_DOWN_THRESHOLD) // sofa apple value = 12 / floor pepper 5
	        {
	        	inDat->key_down = 1;
	        }
	        // If we are above the key up theshold
	        else if (inDat->moving_sum >= KEY_UP_THRESHOLD)
	        {
	        	inDat->key_down = 0;
	        }

	        // Has the key state changed?
	        if (inDat->key_state_prev != inDat->key_down)
	        {
	        	printf("Key %d stated changed to %i, sum: %d, low: %d, high: %d\n", pin_index, inDat->key_down, inDat->moving_sum, lowest, highest);

	        	if (inDat->key_down)
	        	{
	        		send_key(gpioKeys[pin_index], 1);
	        	}
	        	else
	        	{
	        		send_key(gpioKeys[pin_index], 0);
	        	}

	        	sync_input();

	        	inDat->key_state_prev = inDat->key_down;
	        }

/*
	        if (inDat->moving_sum_index == 0)
	        {
	    	    if (inDat->moving_sum < lowest) lowest = inDat->moving_sum;
		        if (inDat->moving_sum > highest) highest = inDat->moving_sum;

	        	//printf("pin %d: moving sum: %d, lowest: %d, highest: %d\n", pin_index, inDat->moving_sum, lowest, highest);
	        	//printf("pin %d: moving sum: %d, key down: %d\n", pin_index, inDat->moving_sum, inDat->key_down);
	        	//printf("%d\n", moving_sum);
	        }
*/
	    }

		usleep(744);
	}

	if (-1 == input_close())
	{
		return 0;
	}

	return 0;
}