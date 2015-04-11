#include <avr/io.h>
#include <stdint.h>

#include "../cmd.h"

#define	SYSCLK			8000000
#define	BAUDRATE		115200
#define	BAUD_DIV		(SYSCLK/(BAUDRATE * 8) - 1)
#define	PIN_ALE1		(0x10)
#define	PIN_ALE0		(0x8)
#define	PIN_CS			(0x4)
#define	PIN_RD			(0x40)
#define	PIN_WR			(0x20)


unsigned char addr_low;
unsigned char addr_high;
unsigned char addr_bank;


void init_uart(void) {
	//UBRRL = BAUD_DIV;
	//UBRRH = (BAUD_DIV >> 8);
	UBRRL = 8;
	UBRRH = 0;
	/* Double speed uart */
	UCSRA |= 2;

	UCSRB |= (1 << TXEN) | (1 << RXEN);
	UCSRC |= (1 << 7) | (1 << UCSZ0) | (1 << UCSZ1);
}

void uart_send(unsigned char byte) {
	while (!(UCSRA & (1 << UDRE)));
	UDR = byte;
}

unsigned char uart_recv(void) {
	while (!(UCSRA & (1 << RXC)));
	return UDR;
}

unsigned char gbus_read() {
	unsigned char data;

	/* Made data port input */
	DDRB = 0;

	PORTD &= (~PIN_RD);
	data = PORTB;
	data = PORTB;
	data = PORTB;
	data = PORTB;
	PORTD |= PIN_RD;
	return data;
}

void gbus_write(unsigned char data, unsigned char ale) {
	/* Make data port output */
	DDRB = 0xFF;

	PORTB = data;
	PORTD |= ale;

	PORTD &= ~PIN_WR;
	PORTD &= ~PIN_WR;
	PORTD &= ~PIN_WR;
	PORTD &= ~PIN_WR;
	PORTD |= PIN_WR;
	PORTD |= PIN_WR;
	PORTD |= PIN_WR;
	PORTD |= PIN_WR;
	PORTD &= (~ale);

	/* Set data port as input */
	DDRB = 0;
}

void gbus_write_addr_low(unsigned char addr) {
	gbus_write(addr, PIN_ALE0);
}

void gbus_write_addr_high(unsigned char addr) {
	/* A15 must never go low when SRAM is enabled, or VeryBad™ things will happen */
	if (!(PORTD & PIN_CS))
		addr |= 0x80;
	gbus_write(addr, PIN_ALE1);
}

void gbus_set_bank(unsigned char bank) {
	gbus_write_addr_high(0x20);
	gbus_write_addr_low(0x0);
	gbus_write(bank, 0);
}

void gbus_enable_sram() {
	PORTD &= (~PIN_CS);
	gbus_write_addr_high(0xFF);
}

void gbus_disable_sram() {
	PORTD |= PIN_CS;
	gbus_write_addr_high(0x0);
}

void flash_write_command(unsigned char cmd) {
	gbus_set_bank(1);
	gbus_write_addr_high(0x40 | 0x15);
	gbus_write_addr_low(0x55);
	gbus_write(0xAA, 0);
	gbus_set_bank(0);
	gbus_write_addr_high(0x40 | 0x2A);
	gbus_write_addr_low(0xAA);
	gbus_write(0x55, 0);
	gbus_set_bank(1);
	gbus_write_addr_high(0x40 | 0x15);
	gbus_write_addr_low(0x55);
	gbus_write(cmd, 0);
}

void flash_write_byte(unsigned char byte) {
	flash_write_command(0xA0);
	gbus_set_bank(addr_bank);
	gbus_write_addr_high(0x40 | addr_high);
	gbus_write_addr_low(addr_low);
	gbus_write(byte, 0);
	while ((gbus_read() & 0x80) != (byte & 0x80));
	return;
}

int main(void) {
	unsigned char data;
	uint16_t i;

	init_uart();
	/* D port is always output */
	DDRD = 0x7C;
	PORTD = 0x64;
	/* Bit 0 used for busy LED, bit 1 for locked LED */
	DDRA = 0x3;
	PORTA = 0;

	for (data = uart_recv();; data = uart_recv()) {
		/* Make LED glow when we're busy */
		PORTA |= 1;

		switch (data) {
			case CMD_SRAM_ENABLE:
				gbus_enable_sram();
				break;
			case CMD_SRAM_DISABLE:
				gbus_disable_sram();
				break;
			case CMD_READ_ONE:
				uart_send(gbus_read());
				break;
			case CMD_WRITE_ONE:
				gbus_write(uart_recv(), 0);
				break;
			case CMD_READ_256:
				for (i = 0; i < 256; i++) {
					gbus_write_addr_low(i);
					uart_send(gbus_read());
				}
				addr_high++;
				gbus_write_addr_high(addr_high);
				break;
			case CMD_WRITE_256:
				for (i = 0; i < 256; i++) {
					gbus_write_addr_low(i);
					gbus_write(uart_recv(), 0);
				}
				addr_high++;
				gbus_write_addr_high(addr_high);
				break;
			case CMD_FLASH_ONE:
				flash_write_byte(uart_recv());
				break;
			case CMD_FLASH_256:
				for (i = 0; i < 256; i++) {
					addr_low = i;
					flash_write_byte(uart_recv());
				}
				addr_high++;
				gbus_write_addr_high(addr_high);
				addr_low = 0;
				break;
			case CMD_SET_ADDR:
				addr_low = uart_recv();
				gbus_write_addr_low(addr_low);
				addr_high = uart_recv();
				gbus_write_addr_high(addr_high);
				break;
			case CMD_SET_ADDR_LOW:
				addr_low = uart_recv();
				gbus_write_addr_low(addr_low);
				break;
			case CMD_SET_ADDR_HIGH:
				addr_high = uart_recv();
				gbus_write_addr_high(addr_high);
				break;
			case CMD_SET_BANK:
				addr_bank = uart_recv();
				gbus_set_bank(addr_bank);
				break;
			case CMD_CHECK_VALID:
				flash_write_command(0x90);
				gbus_set_bank(0);
				gbus_write_addr_high(0);
				gbus_write_addr_low(0);
				if (gbus_read() != 0xBF)
					goto valid_error;
				gbus_write_addr_low(1);
				if (gbus_read() != 0xB7)
					goto valid_error;
				gbus_write(0xF0, 0);
				break;

				valid_error:
				gbus_write(0xF0, 0);
				uart_send(0xFE);
				goto done;
			case CMD_ERASE_CHIP:
				flash_write_command(0x80);
				flash_write_command(0x10);

				while (!(gbus_read() & 0x80));
				break;
			case CMD_SET_LOCK_LED:
				PORTA |= 0x2;
				break;
			case CMD_UNSET_LOCK_LED:
				PORTA &= (~0x2);
				break;
			case CMD_ECHO:
				uart_send(CMD_ECHO);
				break;
			case CMD_IDENTIFY:
				uart_send('H');
				uart_send('A');
				uart_send('I');
				break;
			default:
				/* Bad command */
				uart_send(0xFF);
				goto done;
		}
		
		/* Indicate that we're ready for next command */
		uart_send(0xD0);
		
		done:
		PORTA &= (~1);
	}
}
