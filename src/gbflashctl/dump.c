#include "dump.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum MBCRomStrategy {
	MBC_STRATEGY_UNHANDLED,
	MBC_STRATEGY_NO_BANKS,
	MBC_STRATEGY_MBC1,
	MBC_STRATEGY_MBC2,
};

static const char *mbc_name[] = {
	[0x00] = "none",
	[0x01] = "MBC1",
	[0x02] = "MBC1 /w RAM",
	[0x03] = "MBC1 /w NVRAM",
	[0x05] = "MBC2",
	[0x06] = "MBC2 /w Battery",
	[0x08] = "none, /w RAM",
	[0x09] = "none, /w NVRAM",
	[0x0B] = "MMM01",
	[0x0C] = "MMM01 /w RAM",
	[0x0D] = "MMM01 /w NVRAM",
	[0x0F] = "MBC3 /w RTC",
	[0x10] = "MBC3 /w RTC, NVRAM",
	[0x11] = "MBC3",
	[0x12] = "MBC3 /w RAM",
	[0x13] = "MBC3 /w NVRAM",
	[0x15] = "MBC4",
	[0x16] = "MBC4 /w RAM",
	[0x17] = "MBC4 /w NVRAM",
	[0x19] = "MBC5",
	[0x1A] = "MBC5 /w RAM",
	[0x1B] = "MBC5 /w NVRAM",
	[0x1C] = "MBC5 /w Rumble",
	[0x1D] = "MBC5 /w RAM, Rumble",
	[0x1E] = "MBC5 /w NVRAM, Rumble",
	[0x51] = "WigglePak",
	[0x52] = "WigglePak /w Accelerometer",
	[0xFC] = "Pocket Camera",
	[0xFE] = "HuC3",
	[0xFF] = "HuC1 /w NVRAM",
};


static const enum MBCRomStrategy mbc_rom_strategy[] = {
	[0x00] = MBC_STRATEGY_NO_BANKS,
	[0x01] = MBC_STRATEGY_MBC1,
	[0x02] = MBC_STRATEGY_MBC1,
	[0x03] = MBC_STRATEGY_MBC1,
	[0x05] = MBC_STRATEGY_MBC2,
	[0x06] = MBC_STRATEGY_MBC2,
	[0x08] = MBC_STRATEGY_NO_BANKS,
	[0x09] = MBC_STRATEGY_NO_BANKS,
	[0x0B] = MBC_STRATEGY_MBC2,
	[0x0C] = MBC_STRATEGY_MBC2,
	[0x0D] = MBC_STRATEGY_MBC2,
	[0x0F] = MBC_STRATEGY_MBC2,
	[0x10] = MBC_STRATEGY_MBC2,
	[0x11] = MBC_STRATEGY_MBC2,
	[0x12] = MBC_STRATEGY_MBC2,
	[0x13] = MBC_STRATEGY_MBC2,
	[0x15] = MBC_STRATEGY_MBC2,
	[0x16] = MBC_STRATEGY_MBC2,
	[0x17] = MBC_STRATEGY_MBC2,
	[0x19] = MBC_STRATEGY_MBC2,
	[0x1A] = MBC_STRATEGY_MBC2,
	[0x1B] = MBC_STRATEGY_MBC2,
	[0x1C] = MBC_STRATEGY_MBC2,
	[0x1D] = MBC_STRATEGY_MBC2,
	[0x1E] = MBC_STRATEGY_MBC2,
	[0x51] = MBC_STRATEGY_MBC2,
	[0x52] = MBC_STRATEGY_MBC2,
	[0xFE] = MBC_STRATEGY_MBC2,
	[0xFF] = MBC_STRATEGY_MBC2,
};


static struct {
	int		mbc;
	int		banks;
	int		ram_size;
} mbc_state;


bool mbc_identify() {
	uint8_t buff[256];
	command_set_addr_high(0x1);
	if (!command_read_256(buff))
		goto read_error;

	mbc_state.mbc = buff[0x47];
	if (buff[0x48] < 0xA)
		mbc_state.banks = 2 << buff[0x48];
	else if (buff[0x48] == 0x52)
		mbc_state.banks = 72;
	else if (buff[0x48] == 0x53)
		mbc_state.banks = 80;
	else if (buff[0x48] == 0x54)
		mbc_state.banks = 96;
	else
		mbc_state.banks = 0;

	mbc_state.ram_size = buff[0x49] ? (512 << (buff[0x49] * 2)) : 0;
	
	fprintf(stdout, "MBC:\t\t%s\n", mbc_name[mbc_state.mbc] ? mbc_name[mbc_state.mbc] : "Unknown");
	fprintf(stdout, "ROM Size:\t%.4i kiB\n", mbc_state.banks * 16);
	fprintf(stdout, "RAM Size:\t%.4i kiB\n", mbc_state.ram_size >> 10);
	return true;

	read_error:
	fprintf(stderr, "Read error - Link to GamePak interface broken?\n");
	exit(1);
}


static void mbc_set_bank(int bank) {
	int strategy = mbc_rom_strategy[mbc_state.mbc];
	if (strategy == MBC_STRATEGY_NO_BANKS || strategy == MBC_STRATEGY_UNHANDLED)
		return;
	if (strategy == MBC_STRATEGY_MBC1) {
		command_set_addr(0x4000);
		command_write_one((bank >> 5) & 0x3);
		command_set_addr(0x2100);
		command_write_one(bank & 0x1F);
	} else if (strategy == MBC_STRATEGY_MBC2) {
		command_set_addr(0x3100);
		command_write_one((bank >> 8) & 1);
		command_set_addr(0x2100);
		command_write_one(bank & 0xFF);
	}

	return;
}


bool mbc_dump_cart(FILE *fp, FILE *compare) {
	int i, j;
	uint8_t buff[256], comp_buff[256];

	if (!mbc_rom_strategy[mbc_state.mbc]) {
		fprintf(stderr, "MBC 0x%X unsupported\n", mbc_state.mbc);
		exit(1);
	}

	for (i = 0; i < mbc_state.banks; i++) {
		fprintf(stdout, "Reading bank %.3i/%.3i...\r", i + 1, mbc_state.banks);
		fflush(stdout);
		mbc_set_bank(i);
		if (!i)
			command_set_addr_high(0);
		else {
			command_set_addr_high(0x40);
		}

		command_set_addr_low(0);
		for (j = 0; j < 64; j++) {
			if (!command_read_256(buff))
				goto read_error;
			if (fp)
				fwrite(buff, 256, 1, fp);
			if (compare) {
				if (fread(comp_buff, 256, 1, compare) > 0) {
					if (memcmp(comp_buff, buff, 256)) {
						fprintf(stderr, "\nCheck mismatch at bank #%i, segment #%i\n", i, j);
						return false;
					}
				} else {
					fprintf(stderr, "\nInput file is shorter than cartridge data\n");
					return false;
				}
			}
		}
	}

	if (fp)
		fprintf(stdout, "\nROM dump complete!\n");
	if (compare) {
		fprintf(stdout, "\nROM file matches cartridge data\n");
		if (ftell(compare) != (fseek(compare, 0, SEEK_END), ftell(compare)))
			fprintf(stdout, "Warning: Input file contains additional data\n");
	}

	return true;

	read_error:
	fprintf(stderr, "\nRead error - Link to GamePak interface broken?\n");
	exit(1);
}
