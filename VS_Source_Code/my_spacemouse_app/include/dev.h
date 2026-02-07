/*
libspnavdev - direct 6dof device handling library
Copyright (C) 2020 John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SPNAVDEV_DEVICE_H_
#define SPNAVDEV_DEVICE_H_

#include "spnavdev.h"

struct axisprop {
	const char *name;
	int minval, maxval, deadz;
};

struct spndev {
	char *name, *path;
	uint16_t usb_vendor, usb_product;
	unsigned internal_id;

	int fd;			/* UNIX file descriptor */
	void *handle;	/* Win32 handle */

	int num_axes, num_buttons;
	struct axisprop *aprop;
	const char **bn_name;
	int led;
	int lcdbl;

	void *uptr, *drvdata;

	void (*close)(struct spndev*);
	int (*read)(struct spndev*, union spndev_event*);

	void (*setled)(struct spndev*, int led);
	int (*getled)(struct spndev*);

	void (*setlcdbl)(struct spndev *, int bl);
	int (*getlcdbl)(struct spndev *);

	void (*writelcd)(struct spndev *, int state);
};


int spndev_usb_open(struct spndev *dev, const char *devstr, uint16_t vend, uint16_t prod);
int spndev_ser_open(struct spndev *dev, const char *devstr);


#endif	/* SPNAVDEV_DEVICE_H_ */
