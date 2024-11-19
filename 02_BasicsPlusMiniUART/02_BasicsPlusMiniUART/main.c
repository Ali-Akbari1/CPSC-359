// This is an example of a program that demonstrates communication with the
// Raspberry Pi using the Mini UART peripheral.
//
// Make sure the host machine is running Minicom (or an equivalent serial
// communications program) in a second terminal window, and is connected
// (using a USB cable) to the U of C Pi4 Hat's top USB port (the cable also
// handles JTAG communications). Minicom should be set so that it connects to
// Port 2 of the Dual RS232-HS device. On a macOS host machine, Minicom can be
// started at the command line on the host machine using:
//
//     minicom -b 115200 -D tty.usbserial-142401
// 
// On a Linux lab machine, you would instead use:
//
//     minicom -b 115200 -D ???
//
// No matter which serial communications program that you use, make sure the
// baud rate is 115200 bits per second, the data size is 8 bits, odd parity,
// and 1 stop bit. This is summarized as: 115200 8N1. Also make sure hardware
// and software flow control are both set to "No".
//
// Once Minicom has been started, boot the Pi by applying power. A message
// should be printed in the Minicom terminal, and you should be able to send
// keystrokes to the Pi, which are echoed back to the terminal.

// Include header file
#include "uart.h"


void main()
{
    char c;
  
    // Set up the UART serial port
    uart_init();
    
    // Print out a message to the console
    uart_puts("Goodbye cruel world!\n");
    
    // Loop forever, echoing characters received from the console on a separate
    // line with : : around the character
    while (1) {
        // Wait for a character input from the console
      	c = uart_getc();

        // Output the character
        uart_puts(":");
        uart_putc(c);
        uart_puts(":\n");
    }
}
