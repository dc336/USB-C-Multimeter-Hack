#include "storage.h"
#include "py32f0xx_hal.h"

#define HIGH_ADDR   (USER_FLASH_PAGE_ADDR + 4)

int highscore_load(void)
{
    uint32_t magic = *(volatile uint32_t *)USER_FLASH_PAGE_ADDR;
    uint32_t score = *(volatile uint32_t *)HIGH_ADDR;

    if (magic != SAVE_MAGIC)
    {
        return 0;
    }

    return (int)score;
}

void highscore_save(int highscore)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0;
    uint32_t data[2];

    data[0] = SAVE_MAGIC;
    data[1] = (uint32_t)highscore;

    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_PAGEERASE;
    erase.PageAddress = USER_FLASH_PAGE_ADDR;
    erase.NbPages = 1;

    if (HAL_FLASHEx_Erase(&erase, &page_error) == HAL_OK)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, USER_FLASH_PAGE_ADDR, data);
    }

    HAL_FLASH_Lock();
}