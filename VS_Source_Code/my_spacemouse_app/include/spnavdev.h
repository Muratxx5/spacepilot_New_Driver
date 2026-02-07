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
#ifndef SPNAVDEV_H_
#define SPNAVDEV_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spndev;

enum {
	SPNDEV_NONE,
	SPNDEV_MOTION,
	SPNDEV_BUTTON
};

struct spndev_event_motion {
	int type;
	int v[6];
	unsigned int period;
};

struct spndev_event_button {
	int type;
	int num, press;
};

union spndev_event {
	int type;
	struct spndev_event_motion mot;
	struct spndev_event_button bn;
};

/* possible devstr parameters:
 * - null: auto-detect (USB only)
 * - device file (e.g. COM6 or /dev/ttyS0 or /dev/input/event42)
 * - usb id (046d:c625)
 */
struct spndev *spndev_open(const char *devstr);
void spndev_close(struct spndev *dev);

void spndev_set_userptr(struct spndev *dev, void *uptr);
void *spndev_get_userptr(struct spndev *dev);

/* device information */
const char *spndev_name(struct spndev *dev);
const char *spndev_path(struct spndev *dev);
int spndev_usbid(struct spndev *dev, uint16_t *vend, uint16_t *prod);

int spndev_num_axes(struct spndev *dev);
const char *spndev_axis_name(struct spndev *dev, int axis);
int spndev_axis_min(struct spndev *dev, int axis);
int spndev_axis_max(struct spndev *dev, int axis);

int spndev_num_buttons(struct spndev *dev);
const char *spndev_button_name(struct spndev *dev, int bidx);

int spndev_fd(struct spndev *dev);
void *spndev_handle(struct spndev *dev);

/* device operations */

/* returns event type or 0, and fills the event structure */
int spndev_pending(struct spndev *dev);
int spndev_process(struct spndev *dev, union spndev_event *ev);

int spndev_set_led(struct spndev *dev, int state);
int spndev_get_led(struct spndev *dev);

/* axis == -1: set all deadzones */
int spndev_set_deadzone(struct spndev *dev, int axis, int dead);
int spndev_get_deadzone(struct spndev *dev, int axis);

int spndev_set_lcd_bl(struct spndev *dev, int state);
int spndev_get_lcd_bl(struct spndev *dev);
int spndev_write_lcd(struct spndev *dev, int state);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* SPNAVDEV_H_ */
