#ifndef __DUMP_H__
#define	__DUMP_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool mbc_identify();
bool mbc_dump_cart(FILE *fp, FILE *compare);

#endif
