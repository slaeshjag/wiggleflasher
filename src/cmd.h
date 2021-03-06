#ifndef __CMD_H__
#define	__CMD_H__

enum Commands {
	CMD_SRAM_ENABLE		= 0x0,
	CMD_SRAM_DISABLE	= 0x1,
	CMD_READ_ONE		= 0x2,
	CMD_WRITE_ONE		= 0x3,
	CMD_READ_256		= 0x4,
	CMD_WRITE_256		= 0x5,
	CMD_FLASH_ONE		= 0x6,
	CMD_FLASH_256		= 0x7,
	CMD_SET_ADDR		= 0x8,
	CMD_SET_ADDR_LOW	= 0x9,
	CMD_SET_ADDR_HIGH	= 0xA,
	CMD_SET_BANK		= 0xB,
	CMD_CHECK_VALID		= 0xC,
	CMD_ERASE_CHIP		= 0xD,
	CMD_SET_LOCK_LED	= 0xE,
	CMD_UNSET_LOCK_LED	= 0xF,
	CMD_IDENTIFY		= 0x10,
	CMD_ECHO		= 0x55,

};

#endif
