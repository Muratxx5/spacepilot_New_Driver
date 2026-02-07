
#if defined(_WIN32)
#define _CRT_NONSTDC_NO_WARNINGS	/* Avoid error with strdup on Win32 */
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <ctype.h>
#if defined(_WIN32)
#undef _CRT_NONSTDC_NO_WARNINGS
#endif
#include "dev.h"
#include "serio.h"

#if  defined(__i386__) || defined(__ia64__) || defined(_WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define SPNDEV_LITTLE_ENDIAN
#else
#define SPNDEV_BIG_ENDIAN
#endif

#define TURBO_MAGELLAN_COMPRESS

enum {
	SB4000	= 1,
	FLIPXY	= 2
};

enum {
	DEV_UNKNOWN,
	DEV_SB2003,
	DEV_SB2003C,
	DEV_SB3003,
	DEV_SB4000,
	DEV_SMOUSE,
	DEV_SB5000,
	DEV_CADMAN,
	DEV_SPEXP
};

static struct {
	const char *name;
	int nbuttons;
	const char *bnames[16];
} devinfo[] = {
	{"Unknown serial device", 16, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}},
	{"Spaceball 1003/2003", 8, {"1", "2", "3", "4", "5", "6", "7", "pick"}},
	{"Spaceball 2003C", 8, {"1", "2", "3", "4", "5", "6", "7", "8"}},
	{"Spaceball 3003/3003C", 2, {"R", "L"}},
	{"Spaceball 4000FLX/5000FLX-A", 12, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C"}},
	{"Magellan SpaceMouse", 11, {"1", "2", "3", "4", "5", "6", "7", "8", "*", "+", "-"}},
	{"Spaceball 5000FLX", 12, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C"}},
	{"CadMan", 4, {"1", "2", "3", "4"}},
	{"Space Explorer", 14, {"1", "2", "T", "L", "R", "F", "ALT", "ESC", "SHIFT", "CTRL", "Fit", "Panel", "+", "-", "2D"}},
	{0, 0, {0}}
};

static void close_dev_serial(struct spndev* dev);
static int read_dev_serial(struct spndev* dev, union spndev_event* evt);
static int init_dev(struct spndev *dev, int type);

static void stty_save(int fd, struct sball *sb);
static void stty_restore(int fd, struct sball *sb);

static int proc_input(struct spndev* dev, union spndev_event* evt);

static int mag_parsepkt(struct spndev* dev, union spndev_event* evt, int id, char* data, int len);
static int sball_parsepkt(struct spndev* dev, union spndev_event* evt, int id, char* data, int len);

static int guess_device(const char *verstr);

static void make_printable(char *buf, int len);
static void gen_button_events(struct sball* sb, unsigned int prev, union spndev_event* evt);


int spndev_ser_open(struct spndev *dev, const char *devstr)
{
	int fd, sz;
	char buf[128];
	struct sball *sb = 0;

	if(-1 == (fd = seropen(devstr))) {
		fprintf(stderr, "spndev_open: failed to open device: %s: %s\n", devstr, strerror(errno));
		return -1;
	}
	dev->fd = fd;
	dev->path = strdup(devstr);
	dev->usb_vendor = dev->usb_product = 0xFFFF;
	dev->handle = 0;

	if(!(sb = calloc(1, sizeof *sb))) {
		fprintf(stderr, "spndev_open: failed to allocate sball object\n");
		goto err;
	}
	dev->drvdata = sb;
	dev->close = close_dev_serial;
	dev->read = read_dev_serial;
	dev->setled = 0;
	dev->getled = 0;
	dev->setlcdbl = 0;
	dev->getlcdbl = 0;
	dev->writelcd = 0;

	stty_save(fd, sb);

	if(stty_sball(fd, sb) == -1) {
		goto err;
	}
	serwrite(fd, "\r@RESET\r", 8);

	if((sz = serread_timeout(fd, buf, sizeof buf - 1, 2000000)) > 0 && strstr(buf, "\r@1")) {
		/* we got a response, so it's a spaceball */
		make_printable(buf, sz);
		if(init_dev(dev, guess_device(buf)) == -1) {
			fprintf(stderr, "spndev_open: failed to initialize device structure\n");
			goto err;
		}

		printf("Spaceball detected: %s\n", dev->name);

		/* set binary mode and enable automatic data packet sending. also request
		 * a key event to find out as soon as possible if this is a 4000flx with
		 * 12 buttons
		*/
		serwrite(fd, "\rCB\rMSSV\rk\r", 11);

		sb->parse = sball_parsepkt;
		return 0;
	}

	/* try as a magellan spacemouse */
	if(stty_mag(fd, sb) == -1) {
		goto err;
	}
	serwrite(fd, "\rvQ\r", 4);

	if((sz = serread_timeout(fd, buf, sizeof buf - 1, 250000)) > 0 && buf[0] == 'v') {
		make_printable(buf, sz);
		if(init_dev(dev, guess_device(buf)) == -1) {
			fprintf(stderr, "spndev_open: failed to initialize device structure\n");
			goto err;
		}

		printf("Magellan SpaceMouse detected: %s\n", dev->name);

		/* set 3D mode, not-dominant-axis, pass through motion and button packets */
//		serwrite(fd, "m3\r", 3);
#ifdef TURBO_MAGELLAN_COMPRESS
		/* set 3D mode, not-dominant-axis, pass through motion and button packets, extended key
		   mode, turbo/compressed Magellan mode */
		serwrite(fd, "c33\r", 4);
#else
		/* set 3D mode, not-dominant-axis, pass through motion and button packets, extended key
		   mode */
		serwrite(fd, "c32\r", 4);
#endif
		sz = serread_timeout(fd, buf, sizeof buf - 1, 250000);
		sb->parse = mag_parsepkt;
		return 0;
	}

err:
	close_dev_serial(dev);
	return -1;
}

static void close_dev_serial(struct spndev *dev)
{
	if(dev->drvdata) {
		stty_restore(dev->fd, dev->drvdata);
		serclose(dev->fd);
		free(dev->drvdata);
	}
	dev->drvdata = 0;
}

static int read_dev_serial(struct spndev* dev, union spndev_event* evt)
{
	int sz;
	struct sball* sb = (struct sball*)dev->drvdata;

	if (!sb) return -1;

	evt->type = SPNDEV_NONE;

//	while ((sz = serread(dev->fd, sb->buf + sb->len, INP_BUF_SZ - sb->len - 1)) > 0) {
	{
		sz = serread(dev->fd, sb->buf + sb->len, (16 > INP_BUF_SZ - sb->len - 1) ? INP_BUF_SZ - sb->len - 1 : 16);
		sb->len += sz;
		proc_input(dev, evt);
	}

	/* if we fill the input buffer, make a last attempt to parse it, and discard
	 * it so we can receive more
	 */
	if (sb->len >= INP_BUF_SZ) {
		proc_input(dev, evt);
		sb->len = 0;
	}

	return evt->type;
}

static int init_dev(struct spndev *dev, int type)
{
	int i;
	struct sball *sb = dev->drvdata;
	static const char *axnames[] = {"Tx", "Ty", "Tz", "Rx", "Ry", "Rz"};

	if(!(dev->name = strdup(devinfo[type].name))) {
		return -1;
	}
	dev->num_axes = 6;
	dev->num_buttons = devinfo[type].nbuttons;
	sb->keymask      = 0xffff >> (16 - dev->num_buttons);

	if(!(dev->aprop = malloc(dev->num_axes * sizeof *dev->aprop))) {
		free(dev->name);
		return -1;
	}
	if(!(dev->bn_name = malloc(dev->num_buttons * sizeof *dev->bn_name))) {
		free(dev->aprop);
		free(dev->name);
	}

	for(i = 0; i < dev->num_axes; i++) {
		dev->aprop[i].name = axnames[i];
		/* TODO min/max/deadz */
	}
	for(i=0; i<dev->num_buttons; i++) {
		dev->bn_name[i] = devinfo[type].bnames[i];
	}

	/* TODO setup callbacks */
	return 0;
}

static int guess_device(const char *verstr)
{
	int major, minor;
	const char *s;

	if((s = strstr(verstr, "Firmware version"))) {	/* spaceball */
		/* try to guess based on firmware number */
		if(sscanf(s + 17, "%d.%d", &major, &minor) == 2 && major == 2) {
			switch(minor) {
			case 35:
			case 62:
			case 63:
				return DEV_SB3003;

			case 43:
			case 45:
				return DEV_SB4000;

			case 2:
			case 13:
			case 15:
				return DEV_SB2003;

			case 42:
				/* 2.42 is also used by spaceball 4000flx. we'll guess 2003c for
				 * now, and change the buttons to 12 first time we get a '.'
				 * packet. I'll also request a key report during init to make
				 * sure this happens as soon as possible, before clients have a
				 * chance to connect.
				 */
				return DEV_SB2003C;
			}
		}
	}

	if(strstr(verstr, "MAGELLAN")) {
		return DEV_SMOUSE;
	}

	if(strstr(verstr, "SPACEBALL")) {
		return DEV_SB5000;
	}

	if(strstr(verstr, "CadMan")) {
		return DEV_CADMAN;
	}

	if(strstr(verstr, "SpaceExplorer")) {	/* no idea what the ver string is, this is just a guess */
		return DEV_SPEXP;
	}

	fprintf(stderr, "Unknown serial device. Even if it works, please report this "
			"as a bug at https://github.com/ruevs/libspnavdev/issues\n");
	fprintf(stderr, "Please include the following version string in your bug report: \"%s\"\n", verstr);
	return DEV_UNKNOWN;
}

static void stty_save(int fd, struct sball* sb) {}
static void stty_restore(int fd, struct sball* sb) {}

static int proc_input(struct spndev* dev, union spndev_event* evt)
{
	struct sball* sb = (struct sball*)dev->drvdata;
	int sz;
	char* bptr = sb->buf;
	char* start = sb->buf;
	char* end = sb->buf + sb->len;

	/* see if we have a CR in the buffer */
	while (bptr < end) {
		if (*bptr == '\r') {
			*bptr = 0;
			sb->parse(dev, evt, *start, start + 1, bptr - start - 1);
			start = ++bptr;
		} else {
			bptr++;
		}
	}

	sz = start - sb->buf;
	if (sz > 0) {
		memmove(sb->buf, start, sz);	// PAR@@@ hmmmm.... reading from memory beyond buf ?
		sb->len -= sz;
	}
	return 0;
}

static int mag_parsepkt(struct spndev* dev, union spndev_event* evt, int id, char *data, int len)
{
	struct sball* sb = (struct sball*)dev->drvdata;
	int i, prev, motion_pending = 0;
	unsigned int prev_key;
	unsigned check_sum=0;

	/*printf("magellan packet: %c - %s (%d bytes)\n", (char)id, data, len);*/

	switch(id) {
	case 'd':
#ifdef TURBO_MAGELLAN_COMPRESS
		if(len != 14) {
			fprintf(stderr, "magellan: invalid data packet, expected 14 bytes, got: %d\n", len);
			return -1;
		}
#else
		if(len != 24) {
			fprintf(stderr, "magellan: invalid data packet, expected 24 bytes, got: %d\n", len);
			return -1;
		}
#endif
//		evt->type = SPNDEV_MOTION;
		for(i = 0; i < dev->num_axes; i++) {
			prev = evt->mot.v[i];

#ifdef TURBO_MAGELLAN_COMPRESS
			evt->mot.v[i] = ((((int)data[0] & 0x3f) << 6) | (data[1] & 0x3f)) - 0x800;
			check_sum += (unsigned char)(data[0]);
			check_sum += (unsigned char)(data[1]);
			data += 2;
#else
			evt->mot.v[i] = ((((int)data[0] & 0xf) << 12) | (((int)data[1] & 0xf) << 8) |
					(((int)data[2] & 0xf) << 4) | (data[3] & 0xf)) - 0x8000;
			data += 4;
#endif

			if(evt->mot.v[i] != prev) {
// PAR@@@				enqueue_motion(sb, i, evt->mot.v[i]);
				motion_pending++;
			}
		}
#ifdef TURBO_MAGELLAN_COMPRESS
		{
			/* Verify checksum */
			unsigned checsum_rec =  ((((unsigned)data[0] & 0x3f) << 6) | (data[1] & 0x3f));
			if(check_sum != checsum_rec) {
				fprintf(stderr, "magellan: invalid check sum, expected %u, got: %u\n",
				        checsum_rec, check_sum);
			};
			data += 2;
		}
#endif

		if(motion_pending) {
// PAR@@@			enqueue_motion(sb, -1, 0);
			evt->type = SPNDEV_MOTION;
		}
		break;

	case 'k':
		if(len < 3) {
			fprintf(stderr, "magellan: invalid keyboard pakcet, expected 3 bytes, got: %d\n", len);
			return -1;
		}
		prev_key = sb->keystate;
		sb->keystate = (data[0] & 0xf) | ((data[1] & 0xf) << 4) | (((unsigned int)data[2] & 0xf) << 8);
		if(len > 3) {
			sb->keystate |= ((unsigned int)data[3] & 0xf) << 12;
		}

		if(sb->keystate != prev_key) {
			gen_button_events(sb, prev_key, evt);
		}
		break;

	case 'e':
		if(data[0] == 1) {
			fprintf(stderr, "magellan error: illegal command: %c%c\n", data[1], data[2]);
		} else if(data[0] == 2) {
			fprintf(stderr, "magellan error: framing error\n");
		} else {
			fprintf(stderr, "magellan error: unknown device error\n");
		}
		return -1;

	default:
		break;
	}
	return 0;
}

static int sball_parsepkt(struct spndev* dev, union spndev_event* evt, int id, char *data, int len)
{
	struct sball* sb = (struct sball*)dev->drvdata;
	int i, prev, motion_pending = 0;
	char c, *rd, *wr;
	unsigned int prev_key;

	/* decode data packet, replacing escaped values with the correct ones */
	rd = wr = data;
	while(rd < data + len) {
		if((c = *rd++) == '^') {
			switch(*rd++) {
			case 'Q':
				*wr++ = 0x11;	/* XON */
				break;
			case 'S':
				*wr++ = 0x13;	/* XOFF */
				break;
			case 'M':
				*wr++ = 13;		/* CR */
				break;
			case '^':
				*wr++ = '^';
				break;
			default:
				fprintf(stderr, "sball decode: ignoring invalid escape code: %xh\n", (unsigned int)c);
			}
		} else {
			*wr++ = c;
		}
	}
	len = wr - data;	/* update the decoded length */

	switch(id) {
	case 'D':
		if(len != 14) {
			fprintf(stderr, "sball: invalid data packet, expected 14 bytes, got: %d\n", len);
			return -1;
		}

		for(i = 0; i < dev->num_axes; i++) {
			data += 2;
			prev = evt->mot.v[i];
#ifdef SBALL_BIG_ENDIAN
			evt->mot.v[i] = data[0] | data[1] << 8;
#else
			evt->mot.v[i] = data[1] | data[0] << 8;
#endif

			if(evt->mot.v[i] != prev) {
// PAR@@@				enqueue_motion(sb, i, evt->mot.v[i]);
				motion_pending++;
			}
		}
		if(motion_pending) {
// PAR@@@			enqueue_motion(sb, -1, 0);
			evt->type = SPNDEV_MOTION;
		}
		break;

	case 'K':
		if(len != 2) {
			fprintf(stderr, "sball: invalid key packet, expected 2 bytes, got: %d\n", len);
			return -1;
		}
		if(sb->flags & SB4000) break;	/* ignore K packets from spaceball 4000 devices */

		prev_key = sb->keystate;
		/* data[1] bits 0-3 -> buttons 0,1,2,3
		 * data[1] bits 4,5 (3003 L/R) -> buttons 0, 1
		 * data[0] bits 0-2 -> buttons 4,5,6
		 * data[0] bit 4 is (2003 pick) -> button 7
		 */
		sb->keystate = ((data[1] & 0xf) | ((data[1] >> 4) & 3) | ((data[0] & 7) << 4) |
			((data[0] & 0x10) << 3)) & sb->keymask;

		if(sb->keystate != prev_key) {
			gen_button_events(sb, prev_key, evt);
		}
		break;

	case '.':
		if(len != 2) {
			fprintf(stderr, "sball: invalid sb4k key packet, expected 2 bytes, got: %d\n", len);
			return -1;
		}
		/* spaceball 4000 key packet */
		if(!(sb->flags & SB4000)) {
			printf("Switching to spaceball 4000flx/5000flx-a mode (12 buttons)            \n");
			sb->flags |= SB4000;
			dev->num_buttons = 12;	/* might have guessed 8 before */	// PAR@@@@@@ FIIIIX reallocate the button names array at the very least!
			sb->keymask      = 0xffff >> (16 - dev->num_buttons);
		}
		/* update orientation flag (actually don't bother) */
		/*
		if(data[0] & 0x20) {
			sb->flags |= FLIPXY;
		} else {
			sb->flags &= ~FLIPXY;
		}
		*/

		prev_key = sb->keystate;
		/* data[1] bits 0-5 -> buttons 0,1,2,3,4,5
		 * data[1] bit 7 -> button 6
		 * data[0] bits 0-4 -> buttons 7,8,9,10,11
		 */
		sb->keystate = (data[1] & 0x3f) | ((data[1] & 0x80) >> 1) | ((data[0] & 0x1f) << 7);
		if(sb->keystate != prev_key) {
			gen_button_events(sb, prev_key, evt);
		}
		break;

	case 'E':
		fprintf(stderr, "sball: error:");
		for(i=0; i<len; i++) {
			if(isprint((int)data[i])) {
				fprintf(stderr, " %c", data[i]);
			} else {
				fprintf(stderr, " %02xh", (unsigned int)data[i]);
			}
		}
		break;

	case 'M':	/* ignore MSS responses */
	case '?':	/* ignore unrecognized command errors */
		break;

	default:
		/* DEBUG */
		fprintf(stderr, "sball: got '%c' packet:", (char)id);
		for(i=0; i<len; i++) {
			fprintf(stderr, " %02x", (unsigned int)data[i]);
		}
		fputc('\n', stderr);
	}
	return 0;
}

// Probably not needed used in the original dev_serial.c only to make sure that an unknown message can be printf-ed
static void make_printable(char* buf, int len) {}

static void gen_button_events(struct sball* sb, unsigned int prev, union spndev_event* evt) {
	int i;
	unsigned int bit  = 1;
	unsigned int diff = sb->keystate ^ prev;

	for(i = 0; i < 16; i++) {
		if(diff & bit) {
			evt->type     = SPNDEV_BUTTON;
			evt->bn.press = sb->keystate & bit ? 1 : 0;
			evt->bn.num   = i;
		}
		bit <<= 1;
	}
}
