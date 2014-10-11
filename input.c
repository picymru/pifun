#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>

#include <linux/input.h>
#include <linux/uinput.h>

static int input_fd;

static int input_enable_key(int key)
{
	if (ioctl(input_fd, UI_SET_KEYBIT, key) < 0)
	{
		fprintf(stderr, "Failed to enable key: %i\n", key);
		return -1;
	}

	return 0;
}

int input_open(void)
{
	struct uinput_user_dev uidev;

	//input_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	input_fd = open("/dev/uinput", O_RDWR | O_NONBLOCK);

	if(input_fd < 0)
	{
		fprintf(stderr, "Failed to open input device\n");
		return -1;
	}

	// Enable key press/release events
	if (ioctl(input_fd, UI_SET_EVBIT, EV_KEY) < 0)
	{
		fprintf(stderr, "Failed to enable EV_KEY\n");
		return -1;
	}

	if (ioctl(input_fd, UI_SET_EVBIT, EV_SYN) < 0)
	{
		fprintf(stderr, "Failed to enable EV_SYN\n");
		return -1;
	}

	input_enable_key(KEY_UP);
	input_enable_key(KEY_DOWN);
	input_enable_key(KEY_LEFT);
	input_enable_key(KEY_RIGHT);
	input_enable_key(KEY_SPACE);
	input_enable_key(KEY_ENTER);

	memset(&uidev, 0, sizeof(uidev));

	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "pidas-touch");

	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	// Write the config structure
	if (write(input_fd, &uidev, sizeof(uidev)) < 0)
	{
		fprintf(stderr, "Failed to write config structure\n");
		return -1;
	}

	if (ioctl(input_fd, UI_DEV_CREATE) < 0)
	{
		fprintf(stderr, "Failed to create input device\n");
		return -1;
	}

	return 0;
}

int input_close(void)
{
	if (ioctl(input_fd, UI_DEV_DESTROY) < 0)
	{
		fprintf(stderr, "Failed to destroy user input device\n");
		return -1;
	}

	close(input_fd);

	return 0;
}

int send_key(int key, int release)
{
	struct input_event ev;

	memset(&ev, 0, sizeof(ev));

	ev.type = EV_KEY;
	ev.code = key;
	ev.value = release;

	if (write(input_fd, &ev, sizeof(ev)) < 0)
	{
		fprintf(stderr, "Failed to send key\n");
		return -1;
	}

	return 0;
}

int sync_input(void)
{
	struct input_event ev;

    memset(&ev, 0, sizeof(struct input_event));

    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;

    if(write(input_fd, &ev, sizeof(struct input_event)) < 0)
    {
        fprintf(stderr, "Failed to sync input\n");
        return -1;	
    }

    return 0;
}