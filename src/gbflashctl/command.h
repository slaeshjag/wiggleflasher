#ifndef __COMMAND_H__
#define	__COMMAND_H__

#include <stdint.h>
#include <stdbool.h>

bool command_sram_enable();
bool command_sram_disable();
uint8_t command_read_one();
bool command_write_one(uint8_t byte);
bool command_read_256(uint8_t *buff);
bool command_write_256(uint8_t *buff);
bool command_flash_one(uint8_t byte);
bool command_flash_256(uint8_t *buff);
bool command_set_addr(uint16_t addr);
bool command_set_addr_low(uint8_t addr);
bool command_set_addr_high(uint8_t addr);
bool command_set_bank(uint8_t bank);
bool command_check_valid();
bool command_erase_chip();
bool command_set_lock_led(bool on);
bool command_identify();
bool command_echo();


#endif
