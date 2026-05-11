#ifndef APP_GPIO_H
#define APP_GPIO_H

#include "py32f0xx_hal.h"
#include <stdint.h>

#define GPIOA_BASE_ADDR 0x50000000UL
#define GPIOB_BASE_ADDR 0x50000400UL
#define GPIOF_BASE_ADDR 0x50001400UL

#define GPIO_BSRR_OFFSET 0x18
#define GPIO_BRR_OFFSET  0x28

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define GPIOA_BSRR REG32(GPIOA_BASE_ADDR + GPIO_BSRR_OFFSET)
#define GPIOA_BRR  REG32(GPIOA_BASE_ADDR + GPIO_BRR_OFFSET)

#define GPIOB_BSRR REG32(GPIOB_BASE_ADDR + GPIO_BSRR_OFFSET)
#define GPIOB_BRR  REG32(GPIOB_BASE_ADDR + GPIO_BRR_OFFSET)

#define GPIOF_BSRR REG32(GPIOF_BASE_ADDR + GPIO_BSRR_OFFSET)
#define GPIOF_BRR  REG32(GPIOF_BASE_ADDR + GPIO_BRR_OFFSET)

#define LCD_SCL_HIGH_FAST()  (GPIOA_BSRR = (1u << 5))
#define LCD_SCL_LOW_FAST()   (GPIOA_BRR  = (1u << 5))

#define LCD_DC_HIGH_FAST()   (GPIOA_BSRR = (1u << 6))
#define LCD_DC_LOW_FAST()    (GPIOA_BRR  = (1u << 6))

#define LCD_SDA_HIGH_FAST()  (GPIOF_BSRR = (1u << 0))
#define LCD_SDA_LOW_FAST()   (GPIOF_BRR  = (1u << 0))

#define LCD_RST_HIGH_FAST()  (GPIOB_BSRR = (1u << 3))
#define LCD_RST_LOW_FAST()   (GPIOB_BRR  = (1u << 3))

#define LCD_ENABLE_HIGH_FAST() (GPIOB_BSRR = (1u << 6))
#define LCD_ENABLE_LOW_FAST()  (GPIOB_BRR  = (1u << 6))

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} GpioPin;

/* Public pin declarations */
extern const GpioPin LCD_PIN_DC;
extern const GpioPin LCD_PIN_SCL;
extern const GpioPin LCD_PIN_SDA;
extern const GpioPin LCD_PIN_RST;
extern const GpioPin LCD_PIN_ENABLE;
extern const GpioPin BUTTON_PIN;

void app_gpio_init(void);

void gpio_write(GpioPin pin, GPIO_PinState state);
GPIO_PinState gpio_read(GpioPin pin);
void pin_high(GpioPin pin);
void pin_low(GpioPin pin);


uint32_t adc_read_raw(uint32_t channel);
void adc_init_raw(void);
int adc_enable_raw(void);




#endif /* APP_GPIO_H */