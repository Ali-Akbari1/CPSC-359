// This program sets up GPIO 16 as an output pin, which is assumed to be
// connected to the Green LED on the Traffic Light board when that board is
// connected to the Pi via Port B on the Pi Hat. If you are using a breadboard
// instead of the Traffic Light board, then be sure that your circuit uses a
// 330 ohm current-limiting resistor in series with the LED (consult the class
// notes for full details).
//
// Once the GPIO pin 16 is set up, the program then blinks the LED on and off in
// an infinite loop. The program also writes ON and OFF to the console terminal
// as the program runs.
//
// Note that the Red LED on the Traffic Light board uses GPIO 4 and the Yellow
// LED uses GPIO 12 when the board is connected to the Pi via Port B on the
// Pi Hat.



// Include files
#include "uart.h"
#include "gpio.h"

// Function prototypes
void init_GPIO16_to_output();
void set_GPIO16();
void clear_GPIO16();



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function initializes GPIO pin 16 to an output pin
//                  without a internal pull-up or pull-down resistor. It then
//                  turns the output pin on and off (1 and 0) in an infinite
//                  loop.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    register unsigned int r;


    // Set up the UART serial port
    uart_init();
    
    // Set up GPIO pin 16 for output
    init_GPIO16_to_output();
    
    // Print out a message to the console
    uart_puts("Blinking LED Program starting.\n");
    
    // Loop forever, blinking the LED on and off
    while (1) {
        // Turn on the LED
        set_GPIO16();
        
        // Print a message to the console
        uart_puts("ON\n");
        
        // Delay using a busy loop
        r = 0x003FFFFF;
    	while (r--) {
      		asm volatile("nop");
    	}
    	
    	// Turn the LED off
        clear_GPIO16();
        
        // Print a message to the console
        uart_puts("OFF\n");
        
        // Delay using a busy loop
        r = 0x003FFFFF;
    	while (r--) {
      		asm volatile("nop");
    	}
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO16_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 16 to an output pin without any
//                  internal pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO16_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 18 - 20. This is the field FSEL16, which maps to GPIO pin 16.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 18);

    // Set the field FSEL16 to 001, which sets pin 16 to an output pin. We do so
    // by ORing the bit pattern 001 into the field.
    r |= (0x1 << 18);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 16:
    
    // Get the current bit pattern of the GPPUPPDN1 register
    r = *GPPUPPDN1;

    // Zero out bits 0-1 in this bit pattern, since this maps to GPIO pin 16.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 0);

    // Write the modified bit pattern back to the GPPUPPDN1 register
    *GPPUPPDN1 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO16
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 16 to a 1 (high)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO16()
{
    register unsigned int r;
	  
    // Put a 1 into the SET16 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 16);
    *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO16
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 16 to a 0 (low)
//                  level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO16()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR16 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 16);
    *GPCLR0 = r;
}
