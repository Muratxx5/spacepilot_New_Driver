/*
libspnavdev - direct 6dof device handling library
Modified version for exclusive 3Dconnexion SpacePilot support
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spnavdev.h"
#include "dev.h"

#define SPACEPILOT_VID 0x046D
#define SPACEPILOT_PID 0xC625

/* Yeni yardımcı fonksiyon: SpacePilot kontrolü */
static int is_spacepilot(struct spndev *dev)
{
    return dev->usb_vendor == SPACEPILOT_VID && 
           dev->usb_product == SPACEPILOT_PID;
}

struct spndev *spndev_open(const char *devstr)
{
    struct spndev *dev;
    uint16_t vendor = SPACEPILOT_VID, product = SPACEPILOT_PID;
    const int max_attempts = 5;
    int attempt = 0;

    if(!(dev = malloc(sizeof *dev))) {
        perror("spndev_open: failed to allocate device structure");
        return NULL;
    }

    /* Manuel cihaz belirtilmişse */
    if(devstr) {
        /* Seri cihaz isteği */
        if(strncmp(devstr, "/dev/", 5) == 0 || strstr(devstr, "COM")) {
            if(spndev_ser_open(dev, devstr) == -1) {
                free(dev);
                return NULL;
            }
            return dev;
        }
        
        /* VID:PID formatı kontrolü */
        if(sscanf(devstr, "%hx:%hx", &vendor, &product) != 2) {
            fprintf(stderr, "Invalid device format. Use VID:PID or device path\n");
            free(dev);
            return NULL;
        }
    }

    /* Sadece SpacePilot cihazını kabul et */
    if(vendor != SPACEPILOT_VID || product != SPACEPILOT_PID) {
        fprintf(stderr, "Only SpacePilot devices (046D:C625) are supported\n");
        free(dev);
        return NULL;
    }

    /* SpacePilot'i bulana kadar birden fazla deneme yap */
    while(attempt++ < max_attempts) {
        if(spndev_usb_open(dev, NULL, vendor, product) == 0) {
            if(is_spacepilot(dev)) {
                return dev; /* Doğru cihaz bulundu */
            }
            dev->close(dev);
        }
        
        #ifdef _WIN32
        Sleep(200);
        #else
        usleep(200000);
        #endif
    }

    fprintf(stderr, "SpacePilot device not found after %d attempts\n", max_attempts);
    free(dev);
    return NULL;
}

void spndev_close(struct spndev *dev)
{
	if(dev) {
		dev->close(dev);
		free(dev);
	}
}

void spndev_set_userptr(struct spndev *dev, void *uptr)
{
	dev->uptr = uptr;
}

void *spndev_get_userptr(struct spndev *dev)
{
	return dev->uptr;
}

/* device information */
const char *spndev_name(struct spndev *dev)
{
	return dev->name;
}

const char *spndev_path(struct spndev *dev)
{
	return dev->path;
}

int spndev_usbid(struct spndev *dev, uint16_t*vend, uint16_t*prod)
{
	if( (dev->usb_vendor == 0) || (dev->usb_vendor == 0xFFFF) ) return -1;
	*vend = dev->usb_vendor;
	*prod = dev->usb_product;
	return 0;
}


int spndev_num_axes(struct spndev *dev)
{
	return dev->num_axes;
}

const char *spndev_axis_name(struct spndev *dev, int axis)
{
	if(axis < 0 || axis >= dev->num_axes) {
		return 0;
	}
	return dev->aprop[axis].name;
}

int spndev_axis_min(struct spndev *dev, int axis)
{
	if(axis < 0 || axis >= dev->num_axes) {
		return 0;
	}
	return dev->aprop[axis].minval;
}

int spndev_axis_max(struct spndev *dev, int axis)
{
	if(axis < 0 || axis >= dev->num_axes) {
		return 0;
	}
	return dev->aprop[axis].maxval;
}

int spndev_num_buttons(struct spndev *dev)
{
	return dev->num_buttons;
}

const char *spndev_button_name(struct spndev *dev, int bidx)
{
	if(bidx < 0 || bidx >= dev->num_buttons) {
		return 0;
	}
	return dev->bn_name[bidx];
}

int spndev_fd(struct spndev *dev)
{
	return dev->fd;
}

void *spndev_handle(struct spndev *dev)
{
	return dev->handle;
}

/* device operations */
int spndev_process(struct spndev *dev, union spndev_event *ev)
{
	return dev->read(dev, ev);
}

int spndev_set_led(struct spndev *dev, int state)
{
	if(!dev->setled) {
		return -1;
	}
	dev->setled(dev, state);
	return 0;
}

int spndev_get_led(struct spndev *dev)
{
	if(!dev->getled) {
		return 0;
	}
	return dev->getled(dev);
}

/* axis == -1: set all deadzones */
int spndev_set_deadzone(struct spndev *dev, int axis, int dead)
{
	return -1;	/* TODO */
}

int spndev_get_deadzone(struct spndev *dev, int axis)
{
	return -1;	/* TODO */
}

int spndev_set_lcd_bl(struct spndev *dev, int bl)
{
    if(!dev->setlcdbl) {
        return -1;
    }
    dev->setlcdbl(dev, bl);
    return 0;
}

int spndev_get_lcd_bl(struct spndev *dev) {
    if(!dev->getlcdbl) {
        return 0;
    }
    return dev->getlcdbl(dev);
}

int spndev_write_lcd(struct spndev *dev, int state) {
    if(!dev->writelcd) {
        return -1;
    }
    dev->writelcd(dev, state);
    return 0;
}
