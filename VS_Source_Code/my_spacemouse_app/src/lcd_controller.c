#include "lcd_controller.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>  // snprintf için gerekli baþlýk

hid_device* hid = NULL;

static unsigned char frame_buffer[FBWIDTH * FBHEIGHT]; // Global frame buffer

static int set_start_pos(int row, int col) {
    unsigned char buf[4] = { REP_LCD_POS, row, col, 0 };
    return hid_send_feature_report(hid, buf, sizeof(buf));
}

static int send_unpacked(const unsigned char* data) {
    unsigned char buf[8];
    buf[0] = REP_LCD_DATA;
    memcpy(&buf[1], data, 7);
    return hid_send_feature_report(hid, buf, sizeof(buf));
}

// Geniþletilmiþ font tanýmý (tüm harfler, sayýlar ve temel karakterler)
unsigned char font(char ch, int row) {
    switch (ch) {
        // Büyük harfler
    case 'A': return (unsigned char[]) { 0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00 }[row];
    case 'B': return (unsigned char[]) { 0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00 }[row];
    case 'C': return (unsigned char[]) { 0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00 }[row];
    case 'D': return (unsigned char[]) { 0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00 }[row];
    case 'E': return (unsigned char[]) { 0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00 }[row];
    case 'F': return (unsigned char[]) { 0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00 }[row];
    case 'G': return (unsigned char[]) { 0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'H': return (unsigned char[]) { 0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00 }[row];
    case 'I': return (unsigned char[]) { 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00 }[row];
    case 'J': return (unsigned char[]) { 0x1E, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00 }[row];
    case 'K': return (unsigned char[]) { 0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00 }[row];
    case 'L': return (unsigned char[]) { 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00 }[row];
    case 'M': return (unsigned char[]) { 0x42, 0x66, 0x5A, 0x5A, 0x42, 0x42, 0x42, 0x00 }[row];
    case 'N': return (unsigned char[]) { 0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00 }[row];
    case 'O': return (unsigned char[]) { 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'P': return (unsigned char[]) { 0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00 }[row];
    case 'Q': return (unsigned char[]) { 0x3C, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A, 0x00 }[row];
    case 'R': return (unsigned char[]) { 0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00 }[row];
    case 'S': return (unsigned char[]) { 0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00 }[row];
    case 'T': return (unsigned char[]) { 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }[row];
    case 'U': return (unsigned char[]) { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'V': return (unsigned char[]) { 0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00 }[row];
    case 'W': return (unsigned char[]) { 0x42, 0x42, 0x42, 0x5A, 0x5A, 0x66, 0x42, 0x00 }[row];
    case 'X': return (unsigned char[]) { 0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x00 }[row];
    case 'Y': return (unsigned char[]) { 0x42, 0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x00 }[row];
    case 'Z': return (unsigned char[]) { 0x7E, 0x02, 0x04, 0x18, 0x20, 0x40, 0x7E, 0x00 }[row];
    case 'Ç': return (unsigned char[]) { 0x1C, 0x22, 0x40, 0x40, 0x40, 0x22, 0x1C, 0x00 }[row];
    case 'Ð': return (unsigned char[]) { 0x42, 0x42, 0x7E, 0x42, 0x42, 0x24, 0x18, 0x00 }[row];
    case 'Ý': return (unsigned char[]) { 0x00, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00 }[row];
    case 'Ö': return (unsigned char[]) { 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'Þ': return (unsigned char[]) { 0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00 }[row];
    case 'Ü': return (unsigned char[]) { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];

        // Küçük harfler
    case 'a': return (unsigned char[]) { 0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00 }[row];
    case 'b': return (unsigned char[]) { 0x40, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x7C, 0x00 }[row];
    case 'c': return (unsigned char[]) { 0x00, 0x00, 0x3C, 0x42, 0x40, 0x42, 0x3C, 0x00 }[row];
    case 'd': return (unsigned char[]) { 0x02, 0x02, 0x3E, 0x42, 0x42, 0x42, 0x3E, 0x00 }[row];
    case 'e': return (unsigned char[]) { 0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00 }[row];
    case 'f': return (unsigned char[]) { 0x0C, 0x12, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x00 }[row];
    case 'g': return (unsigned char[]) { 0x00, 0x00, 0x3E, 0x42, 0x42, 0x3E, 0x02, 0x7C }[row];
    case 'h': return (unsigned char[]) { 0x40, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x42, 0x00 }[row];
    case 'i': return (unsigned char[]) { 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x1C, 0x00 }[row];
    case 'j': return (unsigned char[]) { 0x04, 0x00, 0x0C, 0x04, 0x04, 0x44, 0x38, 0x00 }[row];
    case 'k': return (unsigned char[]) { 0x40, 0x40, 0x42, 0x44, 0x78, 0x44, 0x42, 0x00 }[row];
    case 'l': return (unsigned char[]) { 0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00 }[row];
    case 'm': return (unsigned char[]) { 0x00, 0x00, 0x76, 0x49, 0x49, 0x49, 0x49, 0x00 }[row];
    case 'n': return (unsigned char[]) { 0x00, 0x00, 0x7C, 0x42, 0x42, 0x42, 0x42, 0x00 }[row];
    case 'o': return (unsigned char[]) { 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'p': return (unsigned char[]) { 0x00, 0x00, 0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40 }[row];
    case 'q': return (unsigned char[]) { 0x00, 0x00, 0x3E, 0x42, 0x42, 0x3E, 0x02, 0x02 }[row];
    case 'r': return (unsigned char[]) { 0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00 }[row];
    case 's': return (unsigned char[]) { 0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C, 0x00 }[row];
    case 't': return (unsigned char[]) { 0x10, 0x10, 0x7C, 0x10, 0x10, 0x12, 0x0C, 0x00 }[row];
    case 'u': return (unsigned char[]) { 0x00, 0x00, 0x42, 0x42, 0x42, 0x46, 0x3A, 0x00 }[row];
    case 'v': return (unsigned char[]) { 0x00, 0x00, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00 }[row];
    case 'w': return (unsigned char[]) { 0x00, 0x00, 0x41, 0x49, 0x49, 0x49, 0x36, 0x00 }[row];
    case 'x': return (unsigned char[]) { 0x00, 0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00 }[row];
    case 'y': return (unsigned char[]) { 0x00, 0x00, 0x42, 0x42, 0x42, 0x3E, 0x02, 0x7C }[row];
    case 'z': return (unsigned char[]) { 0x00, 0x00, 0x7E, 0x04, 0x18, 0x20, 0x7E, 0x00 }[row];
    case 'ç': return (unsigned char[]) { 0x00, 0x1C, 0x22, 0x20, 0x20, 0x22, 0x1C, 0x08 }[row];
    case 'ð': return (unsigned char[]) { 0x00, 0x42, 0x42, 0x42, 0x3E, 0x02, 0x3C, 0x00 }[row];
    case 'ý': return (unsigned char[]) { 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x3E, 0x00 }[row];
    case 'ö': return (unsigned char[]) { 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00 }[row];
    case 'þ': return (unsigned char[]) { 0x00, 0x00, 0x7C, 0x42, 0x7C, 0x40, 0x40, 0x00 }[row];
    case 'ü': return (unsigned char[]) { 0x00, 0x00, 0x42, 0x42, 0x42, 0x46, 0x3A, 0x00 }[row];


        // Sayýlar (0-9)
    case '0': return (unsigned char[]) { 0x3C, 0x42, 0x46, 0x4A, 0x52, 0x62, 0x3C, 0x00 }[row];
    case '1': return (unsigned char[]) { 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x3E, 0x00 }[row];
    case '2': return (unsigned char[]) { 0x3C, 0x42, 0x02, 0x0C, 0x30, 0x40, 0x7E, 0x00 }[row];
    case '3': return (unsigned char[]) { 0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00 }[row];
    case '4': return (unsigned char[]) { 0x04, 0x0C, 0x14, 0x24, 0x7E, 0x04, 0x04, 0x00 }[row];
    case '5': return (unsigned char[]) { 0x7E, 0x40, 0x7C, 0x02, 0x02, 0x42, 0x3C, 0x00 }[row];
    case '6': return (unsigned char[]) { 0x1C, 0x20, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00 }[row];
    case '7': return (unsigned char[]) { 0x7E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00 }[row];
    case '8': return (unsigned char[]) { 0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00 }[row];
    case '9': return (unsigned char[]) { 0x3C, 0x42, 0x42, 0x3E, 0x02, 0x04, 0x38, 0x00 }[row];

        // Özel karakterler
    case ' ': return 0x00; // Boþluk
    case '!': return (unsigned char[]) { 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 }[row];
    case '?': return (unsigned char[]) { 0x3C, 0x42, 0x04, 0x08, 0x08, 0x00, 0x08, 0x00 }[row];
    case '.': return (unsigned char[]) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00 }[row];
    case ',': return (unsigned char[]) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 }[row];
    case ':': return (unsigned char[]) { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00 }[row];
    case ';': return (unsigned char[]) { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00 }[row];
    case '-': return (unsigned char[]) { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 }[row];
    case '+': return (unsigned char[]) { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00 }[row];
    case '*': return (unsigned char[]) { 0x00, 0x24, 0x18, 0x7E, 0x18, 0x24, 0x00, 0x00 }[row];
    case '/': return (unsigned char[]) { 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00 }[row];
    case '\\': return (unsigned char[]) { 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 }[row];
    case '|': return (unsigned char[]) { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 }[row];
    case '=': return (unsigned char[]) { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00 }[row];
    case '(': return (unsigned char[]) { 0x0C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0C, 0x00 }[row];
    case ')': return (unsigned char[]) { 0x30, 0x08, 0x08, 0x08, 0x08, 0x08, 0x30, 0x00 }[row];
    case '[': return (unsigned char[]) { 0x3C, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x00 }[row];
    case ']': return (unsigned char[]) { 0x3C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x3C, 0x00 }[row];
    case '{': return (unsigned char[]) { 0x0E, 0x10, 0x10, 0x60, 0x10, 0x10, 0x0E, 0x00 }[row];
    case '}': return (unsigned char[]) { 0x70, 0x08, 0x08, 0x06, 0x08, 0x08, 0x70, 0x00 }[row];
    case '<': return (unsigned char[]) { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 }[row];
    case '>': return (unsigned char[]) { 0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00 }[row];
    case '"': return (unsigned char[]) { 0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00 }[row];
    case '\'': return (unsigned char[]) { 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }[row];
    case '_': return (unsigned char[]) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00 }[row];
    case '$': return (unsigned char[]) { 0x08, 0x3E, 0x48, 0x3E, 0x12, 0x3E, 0x08, 0x00 }[row];
    case '%': return (unsigned char[]) { 0x62, 0x64, 0x08, 0x10, 0x26, 0x46, 0x00, 0x00 }[row];
    case '&': return (unsigned char[]) { 0x30, 0x48, 0x30, 0x50, 0x4A, 0x44, 0x3A, 0x00 }[row];
    case '#': return (unsigned char[]) { 0x14, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x14, 0x00 }[row];
    case '@': return (unsigned char[]) { 0x3C, 0x42, 0x5A, 0x56, 0x5C, 0x40, 0x3C, 0x00 }[row];

    default:  return 0x00;
    }
}

int init_lcd() {
    if (hid_init()) return 0;
    hid = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
    return hid != NULL ? 1 : 0;
}

void draw_char(int x, int y, char ch, unsigned char* fb) {
    for (int row = 0; row < CHAR_HEIGHT; row++) {
        unsigned char byte = font(ch, row);
        for (int col = 0; col < 8; col++) {
            int px = x + col;
            int py = y + row;
            if (px >= FBWIDTH || py >= FBHEIGHT) continue;
            fb[py * FBWIDTH + px] = (byte & (1 << (7 - col))) ? 1 : 0;
        }
    }
}

void update_display(unsigned char* fb) {
    for (int row = 0; row < 8; row++) {
        set_start_pos(row, 0);
        for (int col = 0; col < FBWIDTH; col += 7) {
            unsigned char data[7];
            for (int i = 0; i < 7; i++) {
                unsigned char col_byte = 0;
                for (int bit = 0; bit < 8; bit++) {
                    int idx = row * 8 + bit;
                    col_byte |= (fb[idx * FBWIDTH + (col + i)] ? 1 : 0) << bit;
                }
                data[i] = col_byte;
            }
            send_unpacked(data);
        }
    }
}

void show_message_on_lcd(const char* msg) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };
    int x = 0;
    int y = 0;

    for (int i = 0; msg[i] != '\0'; i++) {
        // Satýr sonu kontrolü (normal veya \n karakteri)
        if (msg[i] == '\n' || x + 8 > FBWIDTH) {
            x = 0;
            y += LINE_HEIGHT; // Satýr yüksekliði + boþluk

            // Ekran sýnýr kontrolü
            if (y + CHAR_HEIGHT > FBHEIGHT) {
                break;
            }

            // \n karakterini atla
            if (msg[i] == '\n') continue;
        }

        // Karakteri çiz
        draw_char(x, y, msg[i], fb);
        x += 8;
    }

    update_display(fb);
}

void clear_lcd() {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };
    update_display(fb);
}
void show_button_message(int button_num, const char* header_text) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };

    // Baþlýk yazýsý
    int header_len = (int)strlen(header_text);
    int x_center = (FBWIDTH - header_len * 8) / 2;
    int y_center = (FBHEIGHT - 8) / 2;

    for (int i = 0; header_text[i]; i++) {
        draw_char(x_center + i * 8, y_center, header_text[i], fb);
    }

    // Buton mesajý (düzeltilmiþ kýsým)
    char button_text[64] = { 0 };  // Baþlatýldý
    snprintf(button_text, sizeof(button_text), " Basilan Buton: %d ", button_num);

    int x = 0, y = 0;
    for (int i = 0; button_text[i] && y < FBHEIGHT; i++) {
        draw_char(x, y, button_text[i], fb);
        x += 8;
        if (x + 8 > FBWIDTH) {
            x = 0;
            y += 8;
        }
    }

    update_display(fb);
}
static void draw_text_centered(const char* text, int y, unsigned char* fb) {
    if (!text) return;
    int x = (FBWIDTH - strlen(text) * CHAR_WIDTH) / 2;
    for (int i = 0; text[i]; i++) {
        draw_char(x + i * CHAR_WIDTH, y, text[i], fb);
    }
}

void show_custom_message(const char* top_text, const char* center_text, const char* bottom_text) {
    static unsigned char fb[FBWIDTH * FBHEIGHT];
    memset(fb, 0, sizeof(fb));

    draw_text_centered(top_text, 0, fb);
    draw_text_centered(center_text, (FBHEIGHT - CHAR_HEIGHT) / 2, fb);
    draw_text_centered(bottom_text, FBHEIGHT - CHAR_HEIGHT, fb);

    update_display(fb);
}
void draw_rectangle(int x, int y, int width, int height, int thickness) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };

    // Üst kenar
    for (int i = x; i < x + width; i++) {
        for (int t = 0; t < thickness; t++) {
            if (i < FBWIDTH && (y + t) < FBHEIGHT) {
                fb[(y + t) * FBWIDTH + i] = 1;
            }
        }
    }

    // Alt kenar
    for (int i = x; i < x + width; i++) {
        for (int t = 0; t < thickness; t++) {
            if (i < FBWIDTH && (y + height - 1 - t) < FBHEIGHT) {
                fb[(y + height - 1 - t) * FBWIDTH + i] = 1;
            }
        }
    }

    // Sol kenar
    for (int j = y; j < y + height; j++) {
        for (int t = 0; t < thickness; t++) {
            if ((x + t) < FBWIDTH && j < FBHEIGHT) {
                fb[j * FBWIDTH + (x + t)] = 1;
            }
        }
    }

    // Sað kenar
    for (int j = y; j < y + height; j++) {
        for (int t = 0; t < thickness; t++) {
            if ((x + width - 1 - t) < FBWIDTH && j < FBHEIGHT) {
                fb[j * FBWIDTH + (x + width - 1 - t)] = 1;
            }
        }
    }

   update_display(fb);
}
void draw_4row_table() {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };

    // Çizgi kalýnlýðý
    const int line_thickness = 2;

    // Satýr yükseklikleri (piksel cinsinden)
    int row_heights[] = {
        FBHEIGHT / 4,    // 1. satýr
        FBHEIGHT / 4,    // 2. satýr 
        FBHEIGHT / 4,    // 3. satýr
        FBHEIGHT / 4     // 4. satýr
    };

    // Yatay çizgiler (sadece 2. ve 3. satýrlarýn üstüne)
    for (int row = 1; row < 4; row++) {
        int y = 0;
        for (int i = 0; i < row; i++) {
            y += row_heights[i];
        }

        for (int x = 0; x < FBWIDTH; x++) {
            for (int t = 0; t < line_thickness; t++) {
                if (y + t < FBHEIGHT) {
                    fb[(y + t) * FBWIDTH + x] = 1;
                }
            }
        }
    }

    // Dikey çizgiler (2. satýrdan itibaren)
    int vertical_line = FBWIDTH / 2;
    for (int y = row_heights[0]; y < FBHEIGHT; y++) {
        for (int t = 0; t < line_thickness; t++) {
            if (vertical_line + t < FBWIDTH) {
                fb[y * FBWIDTH + (vertical_line + t)] = 1;
            }
        }
    }

    update_display(fb);
}

void draw_text_in_cell(int row, int col, const char* text) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };
    draw_4row_table();

    int x, y;
    int text_width = strlen(text) * 8;
    int row_height = FBHEIGHT / 4;

    // 1. satýr (tek hücre)
    if (row == 0) {
        x = (FBWIDTH - text_width) / 2;
        y = (row_height - 8) / 2;
    }
    // Diðer satýrlar (2 hücreli)
    else {
        int y_offset = row * row_height;
        if (col == 0) {
            x = (FBWIDTH / 2 - text_width) / 2;
        }
        else {
            x = FBWIDTH / 2 + (FBWIDTH / 2 - text_width) / 2;
        }
        y = y_offset + (row_height - 8) / 2;
    }

    for (int i = 0; text[i]; i++) {
        draw_char(x + i * 8, y, text[i], fb);
    }

    update_display(fb);
}
void draw_table_with_content(const char* cells[4][2]) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };

    // Çizgi kalýnlýðý ve stil
    const int line_thickness = 2;

    // Satýr yükseklikleri
    int row_heights[] = { FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4 };

    // 1. Yatay çizgiler (2. 3. ve 4. satýr üstleri)
    int current_y = row_heights[0];
    for (int row = 1; row < 4; row++) {
        for (int x = 0; x < FBWIDTH; x++) {
            for (int t = 0; t < line_thickness; t++) {
                if (current_y + t < FBHEIGHT) {
                    fb[(current_y + t) * FBWIDTH + x] = 1;
                }
            }
        }
        current_y += row_heights[row];
    }

    // 2. Dikey çizgiler (2. satýrdan itibaren)
    int vertical_line = FBWIDTH / 2;
    for (int y = row_heights[0]; y < FBHEIGHT; y++) {
        for (int t = 0; t < line_thickness; t++) {
            if (vertical_line + t < FBWIDTH) {
                fb[y * FBWIDTH + (vertical_line + t)] = 1;
            }
        }
    }

    // 3. Tüm metinleri çiz
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < (row == 0 ? 1 : 2); col++) {
            if (cells[row][col]) {
                int x, y;
                int text_width = strlen(cells[row][col]) * 8;

                if (row == 0) { // 1. satýr (tek hücre)
                    x = (FBWIDTH - text_width) / 2;
                    y = (row_heights[0] - 8) / 2;
                }
                else { // Diðer satýrlar
                    int y_offset = 0;
                    for (int i = 0; i < row; i++) y_offset += row_heights[i];

                    if (col == 0) {
                        x = (FBWIDTH / 2 - text_width) / 2;
                    }
                    else {
                        x = FBWIDTH / 2 + (FBWIDTH / 2 - text_width) / 2;
                    }
                    y = y_offset + (row_heights[row] - 8) / 2;
                }

                for (int i = 0; cells[row][col][i]; i++) {
                    draw_char(x + i * 8, y, cells[row][col][i], fb);
                }
            }
        }
    }

    update_display(fb);
}
static unsigned char frame_buffer[FBWIDTH * FBHEIGHT]; // Global frame buffer

void draw_table_frame() {
    memset(frame_buffer, 0, sizeof(frame_buffer));

    // Çizgi kalýnlýðý
    const int line_thickness = 2;

    // Satýr yükseklikleri
    int row_heights[] = { FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4 };

    // Yatay çizgiler (2., 3. ve 4. satýr üstleri)
    int current_y = row_heights[0];
    for (int row = 1; row < 4; row++) {
        for (int x = 0; x < FBWIDTH; x++) {
            for (int t = 0; t < line_thickness; t++) {
                if (current_y + t < FBHEIGHT) {
                    frame_buffer[(current_y + t) * FBWIDTH + x] = 1;
                }
            }
        }
        current_y += row_heights[row];
    }

    // Dikey çizgiler (2. satýrdan itibaren)
    int vertical_line = FBWIDTH / 2;
    for (int y = row_heights[0]; y < FBHEIGHT; y++) {
        for (int t = 0; t < line_thickness; t++) {
            if (vertical_line + t < FBWIDTH) {
                frame_buffer[y * FBWIDTH + (vertical_line + t)] = 1;
            }
        }
    }

    update_display(frame_buffer);
}

void update_cell_content(int row, int col, const char* text) {
    // Önce tablo çerçevesini koruyarak içeriði temizle
    unsigned char temp_buffer[FBWIDTH * FBHEIGHT];
    memcpy(temp_buffer, frame_buffer, sizeof(temp_buffer));

    // Hücre sýnýrlarýný hesapla
    int cell_top = 0;
    for (int i = 0; i < row; i++) {
        cell_top += FBHEIGHT / 4;
    }
    int cell_bottom = cell_top + FBHEIGHT / 4;
    int cell_left = (row == 0) ? 0 : (col == 0 ? 0 : FBWIDTH / 2);
    int cell_right = (row == 0) ? FBWIDTH : (col == 0 ? FBWIDTH / 2 : FBWIDTH);

    // Hücre içini temizle (çizgiler hariç)
    for (int y = cell_top + 2; y < cell_bottom - 2; y++) {
        for (int x = cell_left + 2; x < cell_right - 2; x++) {
            if (y >= 0 && y < FBHEIGHT && x >= 0 && x < FBWIDTH) {
                temp_buffer[y * FBWIDTH + x] = 0;
            }
        }
    }

    // Metin konumlandýrma (1. satýr için özel ortalama)
    int x_pos, y_pos;
    y_pos = cell_top + (FBHEIGHT / 4 - 8) / 2; // Dikey ortalama

    if (row == 0) {
        // 1. satýr için metni ortala
        int text_width = strlen(text) * 8;
        x_pos = (FBWIDTH - text_width) / 2;
    }
    else {
        // Diðer satýrlar için sola yasla
        x_pos = cell_left + 5; // 5 piksel padding
    }

    // Metni çiz
    for (int i = 0; text[i] && (x_pos + i * 8) < cell_right - 5; i++) {
        draw_char(x_pos + i * 8, y_pos, text[i], temp_buffer);
    }

    update_display(temp_buffer);
}

void draw_full_table(const char* header, const char* cell_contents[3][2]) {
    unsigned char fb[FBWIDTH * FBHEIGHT] = { 0 };

    // 1. Tablo çerçevesini çiz
    const int line_thickness = 2;
    int row_heights[] = { FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4, FBHEIGHT / 4 };

    // Yatay çizgiler
    int current_y = row_heights[0];
    for (int row = 1; row < 4; row++) {
        for (int x = 0; x < FBWIDTH; x++) {
            for (int t = 0; t < line_thickness; t++) {
                if (current_y + t < FBHEIGHT) {
                    fb[(current_y + t) * FBWIDTH + x] = 1;
                }
            }
        }
        current_y += row_heights[row];
    }

    // Dikey çizgiler
    int vertical_line = FBWIDTH / 2;
    for (int y = row_heights[0]; y < FBHEIGHT; y++) {
        for (int t = 0; t < line_thickness; t++) {
            if (vertical_line + t < FBWIDTH) {
                fb[y * FBWIDTH + (vertical_line + t)] = 1;
            }
        }
    }

    // 2. Tüm içerikleri tek seferde çiz
    // Baþlýk (1. satýr)
    int text_width = strlen(header) * 8;
    int x = (FBWIDTH - text_width) / 2;
    int y = (row_heights[0] - 8) / 2;
    for (int i = 0; header[i]; i++) {
        draw_char(x + i * 8, y, header[i], fb);
    }

    // Diðer hücreler
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 2; col++) {
            if (cell_contents[row][col]) {
                int cell_y = row_heights[0];
                for (int i = 0; i < row; i++) cell_y += row_heights[i + 1];

                x = col == 0 ? 5 : FBWIDTH / 2 + 5; // 5 piksel padding
                y = cell_y + (row_heights[row + 1] - 8) / 2;

                for (int i = 0; cell_contents[row][col][i]; i++) {
                    draw_char(x + i * 8, y, cell_contents[row][col][i], fb);
                }
            }
        }
    }

    update_display(fb);
    memcpy(frame_buffer, fb, sizeof(frame_buffer)); // Frame buffer'ý güncelle
}