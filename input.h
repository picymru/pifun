#ifndef INPUT_H
#define INPUT_H

int input_open(void);

int input_close(void);

int send_key(int key, int release);

int sync_input(void);

#endif