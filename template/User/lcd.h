#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* Screen size */
#define LCD_WIDTH   160u
#define LCD_HEIGHT  80u

/* RGB565 colors */
#define LCD_BLACK   0x0000u
#define LCD_WHITE   0xFFFFu
#define LCD_RED     0x001Fu
#define LCD_GREEN   0x07E0u
#define LCD_BLUE    0xF800u
#define LCD_YELLOW  0x07FFu
#define LCD_CYAN    0xFFE0u
#define LCD_MAGENTA 0xF81Fu
#define LCD_GRAY    0x8410u
#define LCD_ORANGE  0x02FFu

#define LCD_X_OFFSET 1
#define LCD_Y_OFFSET 26

void lcd_init(void);
void lcd_clear(unsigned int clearColor);
void u32_to_dec(char *out, uint32_t value);

void lcd_fill_rect(int x, int y, int w, int h, int color);

void lcd_draw_text(int x, int y, const char *text, int fg, int bg, int scale);

#endif /* LCD_H */