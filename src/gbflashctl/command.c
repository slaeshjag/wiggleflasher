#include "../cmd.h"
#include "serial.h"

static bool command_8bit_arg(uint8_t cmd, uint8_t arg) {
	serial_send(cmd);
	serial_send(arg);
	return serial_recv() == 0xD0;
}

bool command_sram_enable() {
	serial_send(CMD_SRAM_ENABLE);
	return serial_recv() == 0xD0;
}

bool command_sram_disable() {
	serial_send(CMD_SRAM_DISABLE);
	return serial_recv() == 0xD0;
}

uint8_t command_read_one() {
	uint8_t data;
	serial_send(CMD_READ_ONE);
	data = serial_recv();
	return serial_recv() == 0xD0 ? data : 0xFF;
}

bool command_write_one(uint8_t byte) {
	return command_8bit_arg(CMD_WRITE_ONE, byte);
}

bool command_read_256(uint8_t *buff) {
	int i;

	serial_send(CMD_READ_256);
	for (i = 0; i < 256; i++)
		buff[i] = serial_recv();
	return serial_recv() == 0xD0;
}

bool command_write_256(uint8_t *buff) {
	int i;

	serial_send(CMD_WRITE_256);
	for (i = 0; i < 256; i++)
		serial_send(buff[i]);
	return serial_recv() == 0xD0;
}

bool command_flash_one(uint8_t byte) {
	return command_8bit_arg(CMD_FLASH_ONE, byte);
}

bool command_flash_256(uint8_t *buff) {
	int i;

	serial_send(CMD_FLASH_256);
	for (i = 0; i < 256; i++)
		serial_send(buff[i]);
	return serial_recv() == 0xD0;
}

bool command_set_addr(uint16_t addr) {
	serial_send(CMD_SET_ADDR);
	serial_send(addr);
	serial_send(addr >> 8);
	return serial_recv() == 0xD0;
}

bool command_set_addr_low(uint8_t addr) {
	return command_8bit_arg(CMD_SET_ADDR_LOW, addr);
}

bool command_set_addr_high(uint8_t addr) {
	return command_8bit_arg(CMD_SET_ADDR_HIGH, addr);
}

bool command_set_bank(uint8_t bank) {
	return command_8bit_arg(CMD_SET_BANK, bank);
}

bool command_check_valid() {
	serial_send(CMD_CHECK_VALID);
	return serial_recv() == 0xD0;
}

bool command_erase_chip() {
	serial_send(CMD_ERASE_CHIP);
	return serial_recv() == 0xD0;
}

bool command_set_lock_led(bool on) {
	serial_send(on?CMD_SET_LOCK_LED:CMD_UNSET_LOCK_LED);
	return serial_recv() == 0xD0;
}

bool command_identify() {
	serial_send(CMD_IDENTIFY);
	return (serial_recv() == 'H' && serial_recv() == 'A' && serial_recv() == 'I' && serial_recv() == 0xD0);
}

bool command_echo() {
	serial_send(CMD_ECHO);
	return (serial_recv() != CMD_ECHO && serial_recv() == 0xD0);
}
