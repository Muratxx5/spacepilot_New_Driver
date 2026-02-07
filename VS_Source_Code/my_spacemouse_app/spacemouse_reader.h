#ifndef SPACEMOUSE_READER_H
#define SPACEMOUSE_READER_H

#include "hidapi.h"

#ifdef __cplusplus
extern "C" {
#endif

	hid_device* open_spacemouse(void);
	void close_spacemouse(void);
	int is_spacemouse_connected(void);

#ifdef __cplusplus
}
#endif

#endif
