#include "lcd.h"
#include "app_gpio.h"
#include "py32f0xx_hal.h"
#include <stdint.h>
#include <stddef.h>


// ST7735S Commands  https://github.com/bersch/ST7735S/blob/master/gfx.c
typedef enum
{
    ST7735_NOP       = 0x00,
    ST7735_SWRESET   = 0x01,
    ST7735_SLPOUT    = 0x11,
    ST7735_NORON     = 0x13,
    ST7735_INVON     = 0x21,
    ST7735_DISPOFF   = 0x28,
    ST7735_DISPON    = 0x29,
    ST7735_CASET     = 0x2A,
    ST7735_RASET     = 0x2B,
    ST7735_RAMWR     = 0x2C,
    ST7735_MADCTL    = 0x36,
    ST7735_IDMOFF    = 0x38,
    ST7735_COLMOD    = 0x3A,
    ST7735_FRMCTR1   = 0xB1,
    ST7735_FRMCTR2   = 0xB2,
    ST7735_FRMCTR3   = 0xB3,
    ST7735_INVCTR    = 0xB4,
    ST7735_PWCTR1    = 0xC0,
    ST7735_PWCTR2    = 0xC1,
    ST7735_PWCTR3    = 0xC2,
    ST7735_PWCTR4    = 0xC3,
    ST7735_PWCTR5    = 0xC4,
    ST7735_VMCTR1    = 0xC5,
    ST7735_VMOFCTR   = 0xC7,
    ST7735_NVFCTR1   = 0xD9,
    ST7735_GMCTRP1   = 0xE0,
    ST7735_GMCTRN1   = 0xE1,
    ST7735_GAMSET    = 0x26,
    ST7735_GCV       = 0xFC
} St7735Command;


static const int font5x7[59][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x07,0x00,0x07,0x00,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}  /* Z */
};




void lcd_delay_short(void)
{
    __asm__ volatile ("nop");
}

void lcd_write8(int value)
{
    for (int bit = 0; bit < 8; bit++)
    {
        if (value & 0x80)
            LCD_SDA_HIGH_FAST();
        else
            LCD_SDA_LOW_FAST();

        lcd_delay_short();

        LCD_SCL_HIGH_FAST();
        lcd_delay_short();

        LCD_SCL_LOW_FAST();
        lcd_delay_short();

        value <<= 1;
    }
}

static void lcd_write_color_repeat(int color, int count)
{
    int hi = (color >> 8) & 0xFF;
    int lo = color & 0xFF;

    pin_high(LCD_PIN_DC);

    while (count-- > 0)
    {
        lcd_write8(hi);
        lcd_write8(lo);
    }
}
void lcd_write_command(int command)
{
    LCD_SCL_LOW_FAST();
    LCD_DC_LOW_FAST();
    lcd_delay_short();
    lcd_write8(command);
}

void lcd_write_data8(int data)
{
    LCD_SCL_LOW_FAST();
    LCD_DC_HIGH_FAST();
    lcd_delay_short();
    lcd_write8(data);
}

void lcd_write_data16(int data)
{
    LCD_DC_HIGH_FAST();
    lcd_write8((data >> 8) & 0xFF);
    lcd_write8(data & 0xFF);
}

static void lcd_push_color(int color, int count)
{
    while (count-- > 0)
    {
        lcd_write_data16(color);
    }
}

 void lcd_write_bytes(const int *data, size_t length)
{
    while (length-- > 0)
    {
        lcd_write_data8(*data++);
    }
}

void lcd_reset(void)
{
    pin_low(LCD_PIN_RST);
    HAL_Delay(10);

    pin_high(LCD_PIN_RST);
    HAL_Delay(150);
}

void lcd_command_with_data(int command, const int *data, size_t length)
{
    lcd_write_command(command);
    lcd_write_bytes(data, length);
}


void lcd_init(void)
{
    static const int frmctr1[] = { 0x00, 0x3F, 0x3F };
    static const int frmctr2[] = { 0x0F, 0x01, 0x01 };
    static const int frmctr3[] = { 0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C };
    static const int pwctr1[]  = { 0xFC, 0x08, 0x02 };
    static const int pwctr3[]  = { 0x0D, 0x00 };
    static const int pwctr4[]  = { 0x8D, 0x2A };
    static const int pwctr5[]  = { 0x8D, 0xEE };
    static const int gmctrp1[] = {
        0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2C,
        0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10
    };
    static const int gmctrn1[] = {
        0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2C,
        0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10
    };

    app_gpio_init();
    lcd_reset();

    lcd_write_command(ST7735_SWRESET);
    HAL_Delay(150);

    lcd_write_command(ST7735_SLPOUT);
    HAL_Delay(120);

    lcd_write_command(ST7735_DISPOFF);

    lcd_command_with_data(ST7735_FRMCTR1, frmctr1, sizeof(frmctr1) / sizeof(frmctr1[0]));
    lcd_command_with_data(ST7735_FRMCTR2, frmctr2, sizeof(frmctr2) / sizeof(frmctr2[0]));
    lcd_command_with_data(ST7735_FRMCTR3, frmctr3, sizeof(frmctr3) / sizeof(frmctr3[0]));

    lcd_write_command(ST7735_INVCTR);
    lcd_write_data8(0x03);

    lcd_command_with_data(ST7735_PWCTR1,  pwctr1,  sizeof(pwctr1)  / sizeof(pwctr1[0]));

	lcd_write_command(ST7735_PWCTR2);
	lcd_write_data8(0xC0);

	lcd_command_with_data(ST7735_PWCTR3, pwctr3, sizeof(pwctr3) / sizeof(pwctr3[0]));
	lcd_command_with_data(ST7735_PWCTR4, pwctr4, sizeof(pwctr4) / sizeof(pwctr4[0]));
	lcd_command_with_data(ST7735_PWCTR5, pwctr5, sizeof(pwctr5) / sizeof(pwctr5[0]));

    lcd_write_command(ST7735_GCV);
    lcd_write_data8(0xD8);

    lcd_write_command(ST7735_NVFCTR1);
    lcd_write_data8(0x40);

    lcd_write_command(ST7735_VMCTR1);
    lcd_write_data8(0x0F);

    lcd_write_command(ST7735_VMOFCTR);
    lcd_write_data8(0x10);

    lcd_write_command(ST7735_GAMSET);
    lcd_write_data8(0x08);

    lcd_write_command(ST7735_MADCTL);
    lcd_write_data8(0x60);

    lcd_write_command(ST7735_COLMOD);
    lcd_write_data8(0x05); /* 16-bit RGB565 */

    lcd_command_with_data(ST7735_GMCTRP1, gmctrp1, sizeof(gmctrp1) / sizeof(gmctrp1[0]));
    lcd_command_with_data(ST7735_GMCTRN1, gmctrn1, sizeof(gmctrn1) / sizeof(gmctrn1[0]));

    lcd_write_command(ST7735_INVON);
    lcd_write_command(ST7735_IDMOFF);
    lcd_write_command(ST7735_NORON);

    lcd_clear(LCD_BLACK);

    lcd_write_command(ST7735_DISPON);
    HAL_Delay(100);
}

void lcd_set_window(int x0, int y0, int x1, int y1)
{
    x0 += LCD_X_OFFSET;
    x1 += LCD_X_OFFSET;
    y0 += LCD_Y_OFFSET;
    y1 += LCD_Y_OFFSET;

    lcd_write_command(ST7735_CASET);
    lcd_write_data16(x0);
    lcd_write_data16(x1);

    lcd_write_command(ST7735_RASET);
    lcd_write_data16(y0);
    lcd_write_data16(y1);

    lcd_write_command(ST7735_RAMWR);
}

void lcd_fill_rect(int x, int y, int w, int h, int color)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT) || (w <= 0) || (h <= 0))
    {
        return;
    }

    if (x < 0)
    {
        w += x;
        x = 0;
    }

    if (y < 0)
    {
        h += y;
        y = 0;
    }

    if ((w <= 0) || (h <= 0))
    {
        return;
    }

    if ((x + w) > LCD_WIDTH)
    {
        w = LCD_WIDTH - x;
    }

    if ((y + h) > LCD_HEIGHT)
    {
        h = LCD_HEIGHT - y;
    }

    lcd_set_window(x, y, x + w - 1, y + h - 1);
    lcd_write_color_repeat(color, w * h);
}

 void lcd_clear(unsigned int clearColor)
{
    lcd_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, clearColor);
}

/* -------------------------------------------------------------------------- */
/* Text drawing                                                                */
/* -------------------------------------------------------------------------- */

char normalize_font_char(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        c = (char)(c - ('a' - 'A'));
    }

    if ((c < 0x20) || (c > 'Z'))
    {
        c = '?';
    }

    return c;
}

void lcd_draw_char(int x, int y, char c, int fg, int bg, int scale)
{
    if (scale <= 0)
    {
        scale = 1;
    }

    c = normalize_font_char(c);
    const int *glyph = font5x7[(int)c - 0x20];

    int char_w = 6 * scale;
    int char_h = 7 * scale;

    lcd_set_window(x, y, x + char_w - 1, y + char_h - 1);

    for (int row = 0; row < 7; row++)
    {
        for (int sy = 0; sy < scale; sy++)
        {
            for (int col = 0; col < 6; col++)
            {
                int color = bg;

                if (col < 5)
                {
                    int bits = glyph[col];
                    if (bits & (1 << row))
                    {
                        color = fg;
                    }
                }

                lcd_push_color(color, scale);
            }
        }
    }
}


void lcd_draw_text(int x, int y, const char *text, int fg, int bg, int scale)
{
    int start_x = x;

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            x = start_x;
            y += (int)(8 * scale);
            text++;
            continue;
        }

        lcd_draw_char(x, y, *text, fg, bg, scale);
        x += (int)(6 * scale);
        text++;
    }
}

void u32_to_dec(char *out, uint32_t value)
{
    char temp[11];
    int i = 0;
    int j = 0;

    if (value == 0)
    {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while ((value > 0) && (i < sizeof(temp)))
    {
        temp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0)
    {
        out[j++] = temp[--i];
    }

    out[j] = '\0';
}