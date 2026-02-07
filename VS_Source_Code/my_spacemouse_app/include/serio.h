/*
libspnavdev - RS-232 serial port HAL
Copyright (C) 2021 Peter Ruevski <dpr@ruevs.com>

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

#ifndef SERIO_H_
#define SERIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#include <termios.h>
#endif

#include "dev.h"

#define INP_BUF_SZ	256

struct sball {
	int type;
	unsigned int flags;

	char buf[INP_BUF_SZ];
	int len;

	unsigned int keystate, keymask;

#ifndef _WIN32
	struct termios saved_term;
#endif
	int saved_mstat;

	int (*parse)(struct spndev*, union spndev_event*, int, char*, int);
};

int seropen(char const* devstr);
int serread(int h, void* buf, unsigned int len);
int serread_timeout(int h, void* buf, unsigned int len, long tm_usec);
int serwrite(int h, void const* buf, unsigned int len);
int stty_sball(int h, struct sball* sb);
int stty_mag(int h, struct sball* sb);
int serclose(int h);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* SERIO_H_ */
