#ifndef STORAGE_H
#define STORAGE_H

// Flash has 20KB flash, use last 128 byte page for persistent data
#define USER_FLASH_PAGE_ADDR   0x08004F80
#define USER_FLASH_PAGE_SIZE   128
#define SAVE_MAGIC             0xDEADBEEF // Magic signature to see if data high score is saved

void highscore_save(int highscore);
int highscore_load(void);

#endif
