#include "serial.h"
#include "command.h"
#include "dump.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool erase_chip(bool force) {
	if (!force) {
		if (!command_check_valid()) {
			fprintf(stderr, "Unsupported FlashPak\n");
			return false;
		}
	}

	if (!command_erase_chip()) {
		fprintf(stderr, "Chip erase failed\n");
		return false;
	}
	return true;
}

bool flash_rom(const char *path, bool force) {
	FILE *fp;
	uint32_t size;
	int banks, i, j;
	uint8_t buff[256];
	
	if (!(fp = fopen(path, "rb"))) {
		fprintf(stderr, "Unable to open %s\n", path);
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	banks = size >> 14;
	/* Size is not in whole banks */
	if (size & 0x3FFF)
		banks++;
	if (banks > 32 && !force) {
		fprintf(stderr, "ROM is too big (%i banks, WigglePak can only hold 32)\n", banks);
		goto error;
	}

	for (i = 0; i < banks; i++) {
		fprintf(stdout, "Writing bank %.3i/%.3i...\r", i + 1, banks);
		fflush(stdout);
		command_set_addr_high(0);
		command_set_bank(i);
		for (j = 0; j < 64; j++) {
			fread(buff, 256, 1, fp);
			if (!command_flash_256(buff))
				goto flash_error;
		}
	}

	fprintf(stdout, "\nFlash completed\n");
	fclose(fp);
	return true;

	flash_error:
	fprintf(stdout, "\n");
	fflush(stdout);
	fprintf(stderr, "Error while writing bank %i, segment %i\n", i, j);
	error:
	fclose(fp);
	return false;

}

void usage(char *name) {
	fprintf(stderr, "gbflashctl v. 0.0.1\n");
	fprintf(stderr, "CLI utility for WiggleFlasher <https://github.com/slaeshjag/wiggleflasher>\n");
	fprintf(stderr, "Usage: %s <COM port> <command> [ROM-file]\n", name);
	fprintf(stderr, "If no ROM-file is given, default.gb is used\n");
	fprintf(stderr, "Commands:\n");
	fprintf(stderr, "\tdetect      - Checks if a GamePak interface is present on COM port\n");
	fprintf(stderr, "\tidentify    - Identifies the GamePak type from cartridge header\n");
	fprintf(stderr, "\tverify      - Compares the data on a cartridge with a supplied ROM image\n");
	fprintf(stderr, "\tdump        - Dumps a cartridge to file\n");
	fprintf(stderr, "\terase       - Erases a WigglePak\n");
	fprintf(stderr, "\tforce-erase - Erases a cartridge without checking that it's compatible\n");
	fprintf(stderr, "\tflash       - Flashes a ROM onto a WigglePak\n");
	fprintf(stderr, "\tforce-flash - Flashes a ROM onto a cartridge without checking that it's compatible\n");
	fprintf(stderr, "\terase-flash - Erases a WigglePak and flashes a ROM to it\n");
	exit(1);
}

void detect() {
	if (!command_identify()) {
		fprintf(stderr, "No flash device detected on COM port\n");
		exit(1);;
	}
}

int main(int argc, char **argv) {
	const char *file;
	FILE *fp = NULL;

	if (argc <3) {
		usage(argv[0]);
	}

	if (argc == 4)
		file = "default.gb";

	if (!serial_init(argv[1]))
		return 1;
	
	detect();
	if (!strcmp(argv[2], "detect")) {
		fprintf(stderr, "Interface detected\n");
		return 0;
	}
	
	command_set_lock_led(true);

	if (!strcmp(argv[2], "identify")) {
		mbc_identify();
	} else if (!strcmp(argv[2], "dump")) {
		if (!(fp = fopen(file, "wb"))) {
			fprintf(stderr, "Unable to open %s\n", file);
			goto end;
		}

		mbc_identify();
		mbc_dump_cart(fp, NULL);
	} else if (!strcmp(argv[2], "verify")) {
		if (!(fopen(file, "rb"))) {
			fprintf(stderr, "Unable to open %s\n", file);
			goto end;
		}

		mbc_identify();
		mbc_dump_cart(NULL, fp);
	} else if (!strcmp(argv[2], "erase")) {
		erase_chip(false);
	} else if (!strcmp(argv[2], "force-erase")) {
		erase_chip(true);
	} else if (!strcmp(argv[2], "flash")) {
		flash_rom(file, false);
	} else if (!strcmp(argv[2], "force-flash")) {
		flash_rom(file, true);
	} else if (!strcmp(argv[2], "erase-flash")) {
		if (!erase_chip(false))
			goto end;
		flash_rom(file, false);
	} else {
		fprintf(stderr, "Bad command %s\n", argv[2]);
	}

	if (fp)
		fclose(fp);

	end:
	command_set_lock_led(false);
	return 0;
}
