#include "spacemouse_reader.h"
#include <stdio.h>

static hid_device* device = NULL;

hid_device* open_spacemouse(void) {
    device = hid_open(0x046d, 0xc626, NULL);  // Vendor ID & Product ID örnek
    return device;
}

void close_spacemouse(void) {
    if (device) {
        hid_close(device);
        device = NULL;
    }
}

int is_spacemouse_connected(void) {
    hid_device* test = hid_open(0x046d, 0xc626, NULL);
    if (test) {
        hid_close(test);
        return 1;
    }
    return 0;
}
