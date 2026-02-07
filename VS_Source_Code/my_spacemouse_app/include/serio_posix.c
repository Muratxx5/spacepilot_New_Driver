/*
libspnavdev - RS-232 access with standard POSIX file IO
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

#ifndef _WIN32

#if defined(_WIN32)
#    define _CRT_NONSTDC_NO_WARNINGS /* Avoid errors with open, read, write, close on Win32*/
#endif

#include <fcntl.h>
#include <unistd.h>
#include "serio.h"

// http://unixwiz.net/techtips/termios-vmin-vtime.html

int seropen(char const* devstr) {
    return open(devstr, O_RDWR | O_NOCTTY | O_NONBLOCK);
}

int serread(int h, void* buf, unsigned int len) {
    return read(h, buf, len);
}

int serread_timeout(int h, void* buf, unsigned int len, long tm_usec) {
    return serread(h, buf, len);
}

int serwrite(int h, void const* buf, unsigned int len) {
    return write(h, buf, len);
}

int stty_sball(int h, struct sball* sb) { return 0; }
int stty_mag(int h, struct sball* sb) { return 0; }

int serclose(int h) {
    return close(h);
}

#endif