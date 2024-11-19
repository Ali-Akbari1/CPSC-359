// This program sets up GPIO pin 1 as an input pin, which is assumed to be
// connected to Push Button B on the Traffic Light board when that board is
// connected to the Pi via Port B on the Pi Hat. The jumper for this switch
// should be set to "PULLUP" (that is, the GPIO pin 1 is pulled up externally).
// The program polls this pin in an infinite loop, and writes the current level
// (low or high) to the console terminal. Pushing Button B should result in the
// level going from high to low.


// Include files
#include "uart.h"
#include "gpio.h"

// Function prototypes
void init_GPIO1_to_input();
unsigned int get_GPIO1();



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function initializes GPIO pin 1 to be an input pin
//                  without an internal pull-up or pull-down resistor. It then
//                  polls the pin an infinite loop and prints out the current
//                  state of the pin (low or high).
//
//                  GPIO 1 is connected to Push Button B on the Traffic Light
//                  board when that board is connected to the Pi via Port B on
//                  the Pi Hat. The jumper for this switch should be set to
//                  "PULLUP".
//                  
//                  If a breadboard is used instead of the Traffic Light
//                  board, make sure your circuit uses a pull-up (or pull-down)
//                  resistor, plus a current-limiting resistor. In other words,
//                  do no let the input pin float. Also be sure to use a 3.3V
//                  (NOT 5V) power supply for this circuit. Note that the
//                  Raspberry Pi uses pull-up resistors on GPIO pins 2 and 3,
//                  so a breadboard circuit will have to use some other GPIO
//                  pin (such as GPIO 17).
//           
////////////////////////////////////////////////////////////////////////////////

void main()
{
    register unsigned int r;


    // Set up the UART serial port
    uart_init();
    
    // Set up GPIO pin 1 for input
    init_GPIO1_to_input();
    
    // Print out a message to the console
    uart_puts("External Switch Program starting.\n");
    
    // Loop forever, polling GPIO pin 1 for its current level
    while (1) {
    	// Get the current value of GPIO pin 1
    	r = get_GPIO1();
    	
		// Determine the pin input level and print this to the console
		if (r == 0) {
	    	uart_puts("GPIO pin 1: Low\n");
		} else {
	    	uart_puts("GPIO pin 1: High\n");
		}
        
        // Delay a little using a busy loop
        r = 0x003FFFFF;
    	while (r--) {
      	    asm volatile("nop");
    	}
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO1_to_input
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 1 to an input pin without using
//                  any internal pull-up or pull-down resistors. Note that a
//                  pull-up resistor MUST be used externally on the Traffic
//                  Light board, or any breadboard circuit connected to the pin.
//                  Be sure that the pin high level is 3.3V (definitely NOT 5V).
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO1_to_input()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 3 - 5. This is the field FSEL1, which maps to GPIO pin 1.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin.
    r &= ~(0x7 << 3);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 1:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 2-3 in this bit pattern, since this maps to GPIO pin 1.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 2);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_GPIO1
//
//  Arguments:      none
//
//  Returns:        1 if the pin level is high, and 0 if the pin level is low.
//
//  Description:    This function gets the current value of GPIO pin 1.
//
////////////////////////////////////////////////////////////////////////////////

unsigned int get_GPIO1()
{
    register unsigned int r;
	  
	  
    // Get the current contents of the GPIO Pin Level Register 0
    r = *GPLEV0;
	  
    // Isolate GPIO pin 1, and return its value (a 0 if low, or a 1 if high)
    return ((r >> 1) & 0x1);
}
