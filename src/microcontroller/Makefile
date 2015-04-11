all:
	avr-gcc -mmcu=attiny2313 -Os flasher.c -o flasher.o
	avr-objcopy -j .text -j .data -O ihex flasher.o flasher.hex

clean:
	rm -f flasher.o
	rm -f flasher.hex
