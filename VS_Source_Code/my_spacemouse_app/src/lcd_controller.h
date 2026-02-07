#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H
// lcd_controller.h dosyasýna bu tanýmlarý ekleyin
#define CHAR_HEIGHT 8       // Her karakterin piksel yüksekliði
#define CHAR_WIDTH 8        // Her karakterin piksel geniþliði
#define LINE_SPACING 2      // Satýrlar arasý boþluk
#define LINE_HEIGHT (CHAR_HEIGHT + LINE_SPACING)  // Toplam satýr yüksekliði

#include <windows.h>
#include <hidapi.h>

#define FBWIDTH   240
#define FBHEIGHT  64
#define VENDOR_ID  0x046d
#define PRODUCT_ID 0xc625

#define REP_LCD_POS         0x0c
#define REP_LCD_DATA        0x0d
#define REP_LCD_DATA_PACKED 0x0e

extern hid_device* hid;

int init_lcd();
void draw_char(int x, int y, char ch, unsigned char* fb);
void update_display(unsigned char* fb);
void show_custom_message(const char* top_text, const char* center_text, const char* bottom_text);
unsigned char font(char ch, int row);
void draw_rectangle(int x, int y, int width, int height, int thickness);
void draw_4row_table();
void draw_text_in_cell(int row, int col, const char* text);
void draw_table_with_content(const char* cells[4][2]);
// Tablo çizimi için
void draw_table_frame();

// Hücre içerik güncelleme için
void update_cell_content(int row, int col, const char* text);
void draw_full_table(const char* header, const char* cell_contents[3][2]);
#endif