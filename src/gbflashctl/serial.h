#ifndef __SERIAL_H__
#define	__SERIAL_H__

#include <stdint.h>
#include <stdbool.h>

bool serial_init(const char *port);
uint8_t serial_recv();
void serial_send(uint8_t data);

#endif
