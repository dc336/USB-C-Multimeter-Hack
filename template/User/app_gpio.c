#include "app_gpio.h"
#include "py32f0xx_hal.h"
#include "py32f0xx_hal_adc.h"

const GpioPin LCD_PIN_DC     = { GPIOA, GPIO_PIN_6 };
const GpioPin LCD_PIN_SCL    = { GPIOA, GPIO_PIN_5 };
const GpioPin LCD_PIN_SDA    = { GPIOF, GPIO_PIN_0 };
const GpioPin LCD_PIN_RST    = { GPIOB, GPIO_PIN_3 };
const GpioPin LCD_PIN_ENABLE = { GPIOB, GPIO_PIN_6 };
const GpioPin BUTTON_PIN     = { GPIOB, GPIO_PIN_2 };

void gpio_write(GpioPin pin, GPIO_PinState state)
{
    HAL_GPIO_WritePin(pin.port, pin.pin, state);
}

GPIO_PinState gpio_read(GpioPin pin)
{
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}

void pin_high(GpioPin pin)
{
    gpio_write(pin, GPIO_PIN_SET);
}

void pin_low(GpioPin pin)
{
    gpio_write(pin, GPIO_PIN_RESET);
}

void app_gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /*
     * PA0 PA1 PA2 PA3 = ADC analog inputs
     */
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOA, &gpio);

    /*
     * Button
     */
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = BUTTON_PIN.pin;
    HAL_GPIO_Init(BUTTON_PIN.port, &gpio);

    /*
     * LCD pins
     */
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    gpio.Pin = LCD_PIN_SCL.pin | LCD_PIN_DC.pin;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = LCD_PIN_SDA.pin;
    HAL_GPIO_Init(LCD_PIN_SDA.port, &gpio);

    gpio.Pin = LCD_PIN_RST.pin | LCD_PIN_ENABLE.pin;
    HAL_GPIO_Init(GPIOB, &gpio);

    LCD_SCL_LOW_FAST();
    LCD_SDA_LOW_FAST();
    LCD_DC_LOW_FAST();
    LCD_RST_HIGH_FAST();
    LCD_ENABLE_HIGH_FAST();
}


int adc_enable_raw(void)
{
    uint32_t timeout;

    if (ADC1->CR & ADC_CR_ADEN)
    {
        return 1;
    }

    ADC1->CR |= ADC_CR_ADEN;

    timeout = 100000;
    while (((ADC1->CR & ADC_CR_ADEN) == 0) && timeout)
    {
        timeout--;
    }

    return 1;
}

void adc_init_raw(void)
{
    __HAL_RCC_ADC_CLK_ENABLE();

    ADC1->CR = 0;

#ifdef ADC_CFGR1_CONT
    ADC1->CFGR1 &= ~ADC_CFGR1_CONT;
#endif

#ifdef ADC_CFGR1_DISCEN
    ADC1->CFGR1 &= ~ADC_CFGR1_DISCEN;
#endif

#ifdef ADC_CFGR1_EXTEN
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTEN;
#endif

#ifdef ADC_CFGR1_ALIGN
    ADC1->CFGR1 &= ~ADC_CFGR1_ALIGN;
#endif

#ifdef ADC_CFGR1_SCANDIR
    ADC1->CFGR1 &= ~ADC_CFGR1_SCANDIR;
#endif

#ifdef ADC_CFGR1_AUTOFF
    ADC1->CFGR1 &= ~ADC_CFGR1_AUTOFF;
#endif

#ifdef ADC_CFGR1_WAIT
    ADC1->CFGR1 &= ~ADC_CFGR1_WAIT;
#endif

#ifdef ADC_CFGR1_AUTDLY
    ADC1->CFGR1 &= ~ADC_CFGR1_AUTDLY;
#endif

#ifdef ADC_CFGR1_OVRMOD
    ADC1->CFGR1 |= ADC_CFGR1_OVRMOD;
#endif

#ifdef ADC_SMPR_SMP
    ADC1->SMPR = ADC_SMPR_SMP;
#endif

#ifdef ADC_CR_ADCAL
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
    {
    }
#endif

    HAL_Delay(1);

    ADC1->ISR = 0xFFFFFFFFu;

    adc_enable_raw();
}

uint32_t adc_read_raw(uint32_t channel)
{
    uint32_t timeout;
    uint32_t value;

    timeout = 100000;
    while ((ADC1->CR & ADC_CR_ADSTART) && timeout)
    {
        timeout--;
    }

    ADC1->CR &= ~ADC_CR_ADEN;
    HAL_Delay(1);

    ADC1->ISR = 0xFFFFFFFFu;

    ADC1->CHSELR = (1UL << channel);

    ADC1->CR |= ADC_CR_ADEN;

    timeout = 100000;
    while (((ADC1->CR & ADC_CR_ADEN) == 0) && timeout)
    {
        timeout--;
    }

    for (volatile int i = 0; i < 1000; i++)
    {
    }

    ADC1->ISR = 0xFFFFFFFFu;

    ADC1->CR |= ADC_CR_ADSTART;

    timeout = 100000;
    while (((ADC1->ISR & ADC_ISR_EOC) == 0) && timeout)
    {
        timeout--;
    }

    value = ADC1->DR;

    ADC1->ISR = 0xFFFFFFFFu;

    return value;
}