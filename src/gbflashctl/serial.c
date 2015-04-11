/* serial.c - Steven Arnow <s@rdw.se>,  2015 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef _WIN32
	#include <unistd.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <sys/types.h>

	static int fd;
#else
	#include <windows.h>
	static HANDLE commport;
#endif


bool serial_init(const char *port) {
	#ifndef _WIN32
	{
		struct termios term;

		if ((fd = open(port, O_RDWR)) < 0) {
			fprintf(stderr, "Unable to open serial port %s\n", port);
			return false;
		}

		memset(&term, 0, sizeof(term));
		term.c_cflag = CS8 | CREAD | CLOCAL;
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 5;

		cfmakeraw(&term);
		cfsetospeed(&term, B115200);
		cfsetispeed(&term, B115200);
		if (tcsetattr(fd, TCSANOW, &term) < 0) {
			fprintf(stderr, "Unable to apply serial port attributes\n");
			return false;
		}

		return true;
	}
	#else
	{
		DCB dcb;
		FillMemory(&dcb, sizeof(dcb), 0);
		dcb.DCBlength = sizeof(dcb);
		if (!BuildCommDCB("115200,n,8,1", &dcb)) {
			fprintf(stderr, "Unable to build COM settings descriptor\n");
		}
		if ((commport = CreateFile(port, 0, 0, NULL, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Unable to open COM port %s\n", port);
			return false;
		}

		if (!SetCommState(commport, &dcb)) {
			fprintf(stderr, "Unable to set COM port settings\n");
			return false;
		}

		return true;
	}
	#endif
}


uint8_t serial_recv() {
	/* TODO: implement timeout */
	uint8_t data = 0xFF;
	#ifndef _WIN32
		read(fd, &data, 1);
		return data;
	#else
		
		ReadFile(commport, &data, 1, NULL, NULL); 
		return data;
	#endif
}


void serial_send(uint8_t data) {
	#ifndef _WIN32
		write(fd, &data, 1);
		return;
	#else
		WriteFile(commport, &data, 1, NULL, NULL);
		return;
	#endif
}
