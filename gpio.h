#ifndef GPIO_H
#define GPIO_H

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

int gpio_export(int pin);

int gpio_unexport(int pin);

int gpio_direction(int pin, int dir);

inline int gpio_read(int pin);

inline void gpio_write(int pin, int value);

#endif