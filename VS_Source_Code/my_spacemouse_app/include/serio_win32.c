/*
libspnavdev - RS-232 access with Win32 API
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

#ifdef _WIN32

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include "serio.h"

#define checkerror(funccall, msg) \
        if(!funccall) { \
            print_error(msg); \
            serclose((int)h); \
            return -1; \
        }

static void print_error(const char* context);

/* Spacetec SpaceBall: 9600 8n1 XON/XOFF */
int stty_sball(int h, struct sball* sb) {
    DCB dcb;

    // https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-R2-and-2012/cc732236(v=ws.11)
    // BuildCommDCBAndTimeouts("baud=9600 parity=N data=8 stop=1 xon=on to=on", &dcb, &CommTimeouts);  // does not do what I need
    dcb.DCBlength = sizeof(DCB);
    checkerror(GetCommState((HANDLE)h, &dcb), "Failed to get serial state");
    BuildCommDCB("baud=9600 parity=N data=8 stop=1 xon=on dtr=on rts=on", &dcb); //     RTS_CONTROL_ENABLE
    checkerror(SetCommState((HANDLE)h, &dcb), "Failed to set serial state");

    return 0;
}

/* LogiCad3D GmbH Magellan SpaceMouse: 9600 8n2 CTS/RTS */
int stty_mag(int h, struct sball* sb) {
    DCB dcb;

    // https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-R2-and-2012/cc732236(v=ws.11)
    // BuildCommDCBAndTimeouts("baud=9600 parity=N data=8 stop=1 xon=on to=on", &dcb, &CommTimeouts);  // does not do what I need
    dcb.DCBlength = sizeof(DCB);
    checkerror(GetCommState((HANDLE)h, &dcb), "Failed to get serial state");
    BuildCommDCB("baud=9600 parity=N data=8 stop=2 xon=off dtr=on rts=on octs=on", &dcb); //     RTS_CONTROL_ENABLE
    checkerror(SetCommState((HANDLE)h, &dcb), "Failed to set serial state");

    return 0;
}

int seropen(char const* devstr) {
    HANDLE h;
    COMMTIMEOUTS CommTimeouts;
    DCB dcb;

    h = CreateFile(devstr /* lpFileName */, GENERIC_READ | GENERIC_WRITE /* dwDesiredAccess */,
                   0 /* dwShareMode */, NULL /* lpSecurityAttributes */,
                   OPEN_EXISTING /* dwCreationDisposition */,
                   FILE_ATTRIBUTE_NORMAL /* dwFlagsAndAttributes */, NULL /* hTemplateFile */
    );

    if(INVALID_HANDLE_VALUE == h) {
        print_error("Could not open serial device");
        return -1;
    }

    // Flush away any bytes previously read or written.
    checkerror(FlushFileBuffers(h), "Failed to flush serial port");

    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddser/ns-ntddser-_serial_timeouts
    // https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/serports/setting-read-and-write-timeouts-for-a-serial-device
    CommTimeouts.ReadIntervalTimeout      = 2 * 1000 * (1+8+1) / 9600; // Inter byte time out in ms. 2 for margin * (1 start 8 data 1 stop bits) / 9600bps    /* = MAXDWORD; for non blocking */
    CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeouts.ReadTotalTimeoutConstant    = 0;

    CommTimeouts.WriteTotalTimeoutMultiplier = 2 * 1000 * (1+8+1)/ 9600;
    CommTimeouts.WriteTotalTimeoutConstant   = 0;
    checkerror(SetCommTimeouts(h, &CommTimeouts), "Failed to set serial timeouts");

    return (int)h;
}

int serread(int h, void* buf, unsigned int len) {
    DWORD read;
    if(ReadFile((HANDLE)h, buf, len, &read, NULL)) {
        return read;
    } else {
        print_error("Read failed");
        return -1;
    };
}

// Only used in the spndev_ser_open function after opening the serial purt (and thus powering the device)
// to allow for the first bytes to arrive. Since on Win32 I configure the serial port "blocking" anyway this is not
// really needed. Kept in case in Linux/Unix or some other platform it is harder to achive.
int serread_timeout(int h, void* buf, unsigned int len, long tm_usec) {
    int ret;
    COMMTIMEOUTS CommTimeouts;
    DWORD ReadIntervalTimeout;
    DWORD ReadTotalTimeoutMultiplier;
    DWORD ReadTotalTimeoutConstant;

    checkerror(GetCommTimeouts((HANDLE)h, &CommTimeouts), "Failed to get serial timeouts");
    ReadIntervalTimeout = CommTimeouts.ReadIntervalTimeout;
    ReadTotalTimeoutMultiplier = CommTimeouts.ReadTotalTimeoutMultiplier;
    ReadTotalTimeoutConstant = CommTimeouts.ReadTotalTimeoutConstant;

    CommTimeouts.ReadIntervalTimeout = tm_usec / 1000;
//    CommTimeouts.ReadTotalTimeoutMultiplier = tm_usec/(1000*len);
    CommTimeouts.ReadTotalTimeoutConstant = tm_usec / 1000;
    checkerror(SetCommTimeouts((HANDLE)h, &CommTimeouts), "Failed to set serial timeouts");

    ret = serread(h, buf, len);

    CommTimeouts.ReadIntervalTimeout = ReadIntervalTimeout;
    CommTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;
    CommTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;
    checkerror(SetCommTimeouts((HANDLE)h, &CommTimeouts), "Failed to set serial timeouts");

    return ret;
}


int serwrite(int h, void const* buf, unsigned int len) {
    DWORD written;
    if(WriteFile((HANDLE)h, buf, len, &written, NULL)) {
        if(written != len) {
            print_error("Not all written");
        }
        return written;
    } else {
        print_error("Write failed");
        return -1;
    };
}

int serclose(int h) {
    if(CloseHandle((void*)h)) {
        return 0;
    } else {
        print_error("Close failed");
        return -1;
    }
}

static void print_error(const char* context) {
    DWORD error_code = GetLastError();
    char buffer[256];
    DWORD size =
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, error_code,
                       MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), buffer, sizeof(buffer), NULL);
    if(size == 0) {
        buffer[0] = 0;
    }
    fprintf(stderr, "%s: %s\n", context, buffer);
}

#endif