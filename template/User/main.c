#include "main.h"
#include "py32f0xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "storage.h"
#include "lcd.h"
#include "app_gpio.h"

#define ADC_CH_PA2_CURRENT 2u
#define ADC_CH_PB1_VOLTAGE 9u

#define PB1_ADC_AT_3300MV 350L
#define PB1_ADC_AT_5000MV 544L

#define PA2_ZERO_ADC 2040L
#define PA2_KNOWN_DELTA 300L
#define PA2_KNOWN_CURRENT_MA 1000L

#define BUTTON_HOLD_MS 500u
#define BARS 3u
#define FRAME_MS 30u
#define FLAPPY_GAP_SIZE 25u
#define BIRD_COLOR LCD_YELLOW
#define BAR_COLOR LCD_GREEN
#define FLAPPY_BACKGROUND LCD_CYAN
#define METER_BACKGROUND LCD_BLACK
#define COUNTER_BACKGROUND LCD_BLACK
#define BIRD_SIZE 10u

#define TIMER_BACKGROUND LCD_BLACK

#define METER_REFRESH_MS 550u

typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESS,
    BUTTON_EVENT_HOLD
} ButtonEvent;

static int32_t calc_vbus_mV(uint32_t pb1_adc);
static int32_t calc_current_mA(uint32_t pa2_adc);
static ButtonEvent handle_button(void);

static int button_is_down(void);
static int get_next_border_color(void);
static int ranNum(void);
static int rects_overlap(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh);
static int flappyScore;
static int meterMode = 0; // start at screen. 0 meter 1 counter 2 flappy

static void drawMeter(void);
static void intro(void);
static void draw_counter_static_screen(void);
static void draw_count_number(int count);
static void draw_highscore_number(int highscore);
static void draw_random_border(void);
static void initBird(void);
static void flappyScreen(void);

typedef struct
{
    int x;
    int y;
    int w;
    int h;
    unsigned int c;
} Bird;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} LastBird;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
    unsigned int c;
} Bars;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} LastBars;

char vBuf[8];
char cBuf[10];

char vBufOld[8];
char cBufOld[10];

char tBuf[6];
char tBufOld[6];

Bars bars[BARS];
Bird bird;
LastBird lastBird;
LastBars lastBars[BARS];

const int rainbow[] = {
    0xF800, 0xF900, 0xFA60, 0xFBE0, 0xFFE0, 0xAFE0,
    0x57E0, 0x07E0, 0x07F0, 0x07FF, 0x041F, 0x001F,
    0x401F, 0x801F, 0xC01F, 0xF81F, 0xF80F, 0xF800};

static int32_t calc_vbus_mV(uint32_t pb1_adc) // AI
{
    int32_t adc = (int32_t)pb1_adc;

    return 3300L +
           ((adc - PB1_ADC_AT_3300MV) * (5000L - 3300L)) /
               (PB1_ADC_AT_5000MV - PB1_ADC_AT_3300MV);
}

static int32_t calc_current_mA(uint32_t pa2_adc) // AI
{
    int32_t delta = PA2_ZERO_ADC - (int32_t)pa2_adc;

    if (delta < 0)
    {
        delta = 0;
    }

    return (delta * PA2_KNOWN_CURRENT_MA) / PA2_KNOWN_DELTA;
}

static void drawMeter(void)
{
    lcd_draw_text(0, 4, "  USB METER", LCD_CYAN, LCD_BLACK, 2);
    lcd_draw_text(0, 26, "V:", LCD_WHITE, LCD_BLACK, 2);
    lcd_draw_text(0, 48, "I:", LCD_WHITE, LCD_BLACK, 2);

    while (meterMode == 0)
    {
        (void)handle_button();

        int current_mA;
        int vbus_mV;

        int adc_current_pa2;
        int adc_voltage_pb1;

        adc_current_pa2 = adc_read_raw(ADC_CH_PA2_CURRENT);
        adc_voltage_pb1 = adc_read_raw(ADC_CH_PB1_VOLTAGE);

        current_mA = calc_current_mA(adc_current_pa2);
        vbus_mV = calc_vbus_mV(adc_voltage_pb1);

        if (vbus_mV < 0)
        {
            vbus_mV = 0;
        }

        if (current_mA < 0)
        {
            current_mA = 0;
        }
        vBuf[0] = (char)('0' + ((vbus_mV / 1000) % 10));
        vBuf[1] = '.';
        vBuf[2] = (char)('0' + ((vbus_mV / 100) % 10));
        vBuf[3] = (char)('0' + ((vbus_mV / 10) % 10));
        vBuf[4] = 'V';
        vBuf[5] = '\0';

        int current_units = current_mA * 10;

        cBuf[0] = (char)('0' + ((current_units / 10000) % 10));
        cBuf[1] = '.';
        cBuf[2] = (char)('0' + ((current_units / 1000) % 10));
        cBuf[3] = (char)('0' + ((current_units / 100) % 10));
        cBuf[4] = (char)('0' + ((current_units / 10) % 10));
        cBuf[5] = (char)('0' + (current_units % 10));
        cBuf[6] = 'A';
        cBuf[7] = '\0';

        lcd_draw_text(36, 26, vBufOld, METER_BACKGROUND, METER_BACKGROUND, 2);
        lcd_draw_text(36, 48, cBufOld, METER_BACKGROUND, METER_BACKGROUND, 2);

        lcd_draw_text(36, 26, vBuf, LCD_GREEN, METER_BACKGROUND, 2);
        lcd_draw_text(36, 48, cBuf, LCD_YELLOW, METER_BACKGROUND, 2);

        strcpy(vBufOld, vBuf);
        strcpy(cBufOld, cBuf);

        for (int i = 0; i < 25 && meterMode == 0; i++) // Non blocking delay 250 so button-hold feels normal
        {
            handle_button();
            HAL_Delay(10);
        }
    }
}

static void drawTimer(void)
{
    lcd_draw_text(0, 4, "    TIMER", LCD_GREEN, LCD_BLACK, 2);

    tBufOld[0] = '\0';

    uint32_t start = HAL_GetTick();
    uint32_t pause_start = 0;
    int paused = 0;

    while (meterMode == 3)
    {
        ButtonEvent button_event = handle_button();

        if (button_event == BUTTON_EVENT_PRESS)
        {
            paused = !paused;

            if (paused)
            {
                pause_start = HAL_GetTick();

                // Draw pause icon
                lcd_fill_rect(140, 5, 5, 15, LCD_WHITE);
                lcd_fill_rect(150, 5, 5, 15, LCD_WHITE);
            }
            else
            {
                // Shift start forward by the amount of time spent paused
                start += HAL_GetTick() - pause_start;

                // Clear pause icon
                lcd_fill_rect(140, 5, 5, 15, TIMER_BACKGROUND);
                lcd_fill_rect(150, 5, 5, 15, TIMER_BACKGROUND);
            }
        }

        if (!paused)
        {
            uint32_t now = HAL_GetTick();
            uint32_t elapsed_secs = (now - start) / 1000;

            uint32_t mins = elapsed_secs / 60;
            uint32_t secs = elapsed_secs % 60;

            tBuf[0] = (char)('0' + ((mins / 10) % 10));
            tBuf[1] = (char)('0' + (mins % 10));
            tBuf[2] = ':';
            tBuf[3] = (char)('0' + ((secs / 10) % 10));
            tBuf[4] = (char)('0' + (secs % 10));
            tBuf[5] = '\0';

            if (strcmp(tBuf, tBufOld) != 0)
            {
                lcd_draw_text(50, 48, tBufOld, TIMER_BACKGROUND, TIMER_BACKGROUND, 2);
                lcd_draw_text(50, 48, tBuf, LCD_YELLOW, TIMER_BACKGROUND, 2);
                strcpy(tBufOld, tBuf);
            }
        }

        HAL_Delay(10);
    }
}

static ButtonEvent handle_button(void) // AI
{
    static int was_down = 0;
    static int hold_done = 0;
    static int down_start_ms = 0;

    int is_down = button_is_down();
    int now = HAL_GetTick();

    if (is_down && !was_down)
    {
        down_start_ms = now;
        hold_done = 0;
    }

    if (is_down && !hold_done)
    {
        if ((int)(now - down_start_ms) >= BUTTON_HOLD_MS)
        {
            hold_done = 1;

            meterMode++;
            if (meterMode >= 4)
            {
                meterMode = 0;
            }

            was_down = is_down;
            return BUTTON_EVENT_HOLD;
        }
    }

    if (!is_down && was_down)
    {
        was_down = is_down;

        if (!hold_done)
        {
            return BUTTON_EVENT_PRESS;
        }

        return BUTTON_EVENT_NONE;
    }

    was_down = is_down;
    return BUTTON_EVENT_NONE;
}

static void initBird(void)
{
    bars[0] = (Bars){150, ranNum(), 5, LCD_HEIGHT, BAR_COLOR};
    bars[1] = (Bars){110, ranNum(), 5, LCD_HEIGHT, BAR_COLOR};
    bars[2] = (Bars){70, ranNum(), 5, LCD_HEIGHT, BAR_COLOR};

    lastBars[0] = (LastBars){150, 0, 5, LCD_HEIGHT};
    lastBars[1] = (LastBars){110, 0, 5, LCD_HEIGHT};
    lastBars[2] = (LastBars){70, 0, 5, LCD_HEIGHT};

    bird.x = 20;
    bird.y = 38;
    bird.w = BIRD_SIZE;
    bird.h = BIRD_SIZE;
    bird.c = BIRD_COLOR;

    lastBird.x = bird.x;
    lastBird.y = bird.y;
    lastBird.w = bird.w;
    lastBird.h = bird.h;
    flappyScore = 0;
}

static int ranNum(void)
{
    static unsigned int rng_state = 123456789;

    rng_state = (rng_state * 1664525) + 1013904223;

    return 5 + (int)(rng_state % 46);
}

static int rects_overlap(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh)
{
    return (ax < bx + bw) &&
           (ax + aw > bx) &&
           (ay < by + bh) &&
           (ay + ah > by);
}

static void flappyScreen(void)
{
    int last_frame_tick = HAL_GetTick();

    char buf[3];

    while (meterMode == 2)
    {
        ButtonEvent button_event = handle_button();

        if (button_event == BUTTON_EVENT_PRESS)
        {
            bird.y -= 10;
        }

        int now = HAL_GetTick();

        if ((int)(now - last_frame_tick) >= FRAME_MS)
        {
            last_frame_tick = now;

            lcd_fill_rect(lastBird.x, lastBird.y, lastBird.w, lastBird.h, FLAPPY_BACKGROUND);
            lcd_fill_rect(lastBird.x + lastBird.w, lastBird.y, 3, 3, FLAPPY_BACKGROUND); // Beak

            for (int i = 0; i < BARS; i++)
            {
                lcd_fill_rect(lastBars[i].x, 0, lastBars[i].w, LCD_HEIGHT, FLAPPY_BACKGROUND);
            }

            for (int i = 0; i < BARS; i++)
            {
                bars[i].x -= 2;

                if ((bars[i].x + bars[i].w) <= 0)
                {
                    bars[i].x = LCD_WIDTH;
                    bars[i].y = ranNum();
                }
            }

            bird.y += 1;

            if (bird.y < 0)
            {
                bird.y = 0;
            }

            if ((bird.y + bird.h) >= LCD_HEIGHT)
            {
                bird.y = LCD_HEIGHT - bird.h;
            }

            lcd_fill_rect(bird.x, bird.y, bird.w, bird.h, bird.c);
            lcd_fill_rect(bird.x + bird.w, bird.y, 3, 3, LCD_ORANGE);

            for (int i = 0; i < BARS; i++)
            {
                int gap_top = bars[i].y;
                int gap_bottom = gap_top + FLAPPY_GAP_SIZE;

                if (gap_bottom > LCD_HEIGHT)
                {
                    gap_bottom = LCD_HEIGHT;
                }

                if (gap_top > 0)
                {
                    lcd_fill_rect(
                        bars[i].x,
                        0,
                        bars[i].w,
                        gap_top,
                        BAR_COLOR);

                    if (rects_overlap(bird.x, bird.y, bird.w, bird.h,
                                      bars[i].x, 0, bars[i].w, gap_top))
                    {
                        lcd_clear(FLAPPY_BACKGROUND);
                        initBird();
                        break;
                    }
                }

                if (gap_bottom < LCD_HEIGHT)
                {
                    lcd_fill_rect(
                        bars[i].x,
                        gap_bottom,
                        bars[i].w,
                        LCD_HEIGHT - gap_bottom,
                        BAR_COLOR);

                    if (rects_overlap(bird.x, bird.y, bird.w, bird.h,
                                      bars[i].x, gap_bottom, bars[i].w, LCD_HEIGHT - gap_bottom))
                    {
                        lcd_clear(FLAPPY_BACKGROUND);
                        initBird();
                        break;
                    }
                    else if (bird.x == bars[i].x)
                    {
                        flappyScore++;
                    }
                }
            }

            lastBird.x = bird.x;
            lastBird.y = bird.y;
            lastBird.w = bird.w;
            lastBird.h = bird.h;

            for (int i = 0; i < BARS; i++)
            {
                lastBars[i].x = bars[i].x;
                lastBars[i].y = bars[i].y;
                lastBars[i].w = bars[i].w;
                lastBars[i].h = LCD_HEIGHT;
            }

            buf[0] = (char)('0' + ((flappyScore / 10) % 10)); // tens
            buf[1] = (char)('0' + (flappyScore % 10));        // ones
            buf[2] = '\0';
            lcd_draw_text(2, 2, buf, LCD_WHITE, LCD_BLACK, 2);
        }
    }
}

static int button_is_down(void)
{
    return gpio_read(BUTTON_PIN) == GPIO_PIN_RESET;
}

static void intro(void)
{
    lcd_draw_text(0, 10, "HACK", LCD_GREEN, LCD_BLACK, 3);
    HAL_Delay(10);
    lcd_draw_text(84, 26, "ALL", LCD_GREEN, LCD_BLACK, 3);
    HAL_Delay(10);
    lcd_draw_text(0, 42, "THE", LCD_GREEN, LCD_BLACK, 3);
    HAL_Delay(10);
    lcd_draw_text(54, 58, "THINGS", LCD_GREEN, LCD_BLACK, 3);
    HAL_Delay(100);

    for (int y = 0; y < LCD_HEIGHT; y += 14)
    {
        int color = rainbow[(y / 14) % (sizeof(rainbow) / sizeof(rainbow[0]))];
        lcd_fill_rect(0, y, LCD_WIDTH, 14, color);
    }

    lcd_clear(LCD_BLACK);
}

static void draw_counter_static_screen(void)
{
    lcd_draw_text(30, 18, "COUNT", LCD_WHITE, LCD_BLACK, 2);
    lcd_draw_text(30, 48, "HIGH", LCD_GREEN, LCD_BLACK, 2);
}

static void draw_count_number(int count)
{
    char count_str[12];

    u32_to_dec(count_str, count);
    lcd_fill_rect(102, 18, 40, 16, LCD_BLACK);
    lcd_draw_text(102, 18, count_str, LCD_WHITE, LCD_BLACK, 2);
}

static void draw_highscore_number(int highscore)
{
    char high_str[12];

    u32_to_dec(high_str, highscore);
    lcd_fill_rect(102, 48, 40, 16, LCD_BLACK);
    lcd_draw_text(102, 48, high_str, LCD_GREEN, LCD_BLACK, 2);
}

static void draw_random_border(void)
{
    const int border_width = 5;

    lcd_fill_rect(0, 0, LCD_WIDTH, border_width, get_next_border_color());
    lcd_fill_rect(LCD_WIDTH - border_width, border_width, border_width, LCD_HEIGHT - border_width, get_next_border_color());
    lcd_fill_rect(0, LCD_HEIGHT - border_width, LCD_WIDTH, border_width, get_next_border_color());
    lcd_fill_rect(0, border_width, border_width, LCD_HEIGHT - border_width, get_next_border_color());
}

static void shuffle_colors(int *colors, int count, unsigned int *rng_state)
{
    for (int i = (int)(count - 1); i > 0; i--)
    {
        *rng_state = (*rng_state * 1664525) + 1013904223;
        int j = (int)(*rng_state % (i + 1));

        int temp = colors[i];
        colors[i] = colors[j];
        colors[j] = temp;
    }
}

static int get_next_border_color(void)
{
    static unsigned int rng_state = 123456789;
    static int colors[] = {
        LCD_RED,
        LCD_GREEN,
        LCD_BLUE,
        LCD_YELLOW,
        LCD_CYAN,
        LCD_MAGENTA};
    static int initialized = 0;
    static int index = 0;
    static int last_color = 0;

    const int color_count = (int)(sizeof(colors) / sizeof(colors[0]));

    if (!initialized)
    {
        shuffle_colors(colors, color_count, &rng_state);
        initialized = 1;
    }

    if (index >= color_count)
    {
        int previous = last_color;

        shuffle_colors(colors, color_count, &rng_state);
        index = 0;

        if (colors[0] == previous)
        {
            int temp = colors[0];
            colors[0] = colors[1];
            colors[1] = temp;
        }
    }

    last_color = colors[index];
    return colors[index++];
}

int main(void)
{
    HAL_Init();
    adc_init_raw();
    lcd_init();
    intro();

    int count = 0;
    int highscore = highscore_load();

    int lastMode = -1;

    while (1)
    {
        ButtonEvent button_event = handle_button();

        if (meterMode != lastMode)
        {
            lastMode = meterMode;

            if (meterMode == 0)
            {
                lcd_clear(METER_BACKGROUND);
                drawMeter();
                lastMode = -1;
            }
            else if (meterMode == 1)
            {
                lcd_clear(COUNTER_BACKGROUND);
                draw_counter_static_screen();
                draw_count_number(count);
                draw_highscore_number(highscore);
                draw_random_border();
            }
            else if (meterMode == 2)
            {
                lcd_clear(FLAPPY_BACKGROUND);
                initBird();
                flappyScreen();
                lastMode = -1;
            }
            else if (meterMode == 3)
            {
                lcd_clear(LCD_BLACK);
                drawTimer();
                lastMode = -1;
            }
        }

        if (meterMode == 1)
        {
            if (button_event == BUTTON_EVENT_PRESS)
            {
                count++;
                draw_count_number(count);
                draw_random_border();

                if (count > highscore)
                {
                    highscore = count;
                    highscore_save(highscore);
                    draw_highscore_number(highscore);
                }
            }
        }

        HAL_Delay(10);
    }
}